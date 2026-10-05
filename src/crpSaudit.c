/*************************************************************************/
/* Controlled Rounding Problem for Linked Tables                       */
/* in Statistical Disclosure Control (SDC)                             */
/*                                                                     */
/*                                                                     */
/* CRPAUDIT.c  (HiGHS backend)                                         */
/* Ported from the SCIP 3.x LPI version to the HiGHS C API.            */
/*                                                                     */
/* The audit subproblem is a pure LP that is loaded once and then       */
/* re-solved many times with a single column's objective cost toggled  */
/* between +1 / -1 (and zero between solves).  The LP structure        */
/* (columns, rows, bounds) never changes, so the HiGHS C API is a      */
/* drop-in: build once with Highs_passLp, then Highs_changeColCost +   */
/* Highs_run + Highs_getObjectiveValue per audit cell.                 */
/*                                                                     */
/* Note: the original LPI GetBounds/ChgBounds dance in                 */
/* S_solvesubproblem was dead code (the bound was never actually        */
/* changed: it was reassigned to itself, so the restore was a no-op).  */
/* The dj/dual/yval outputs are always passed as NULL by the only       */
/* caller (S_CRPauditing), so only the objective value is read back.   */
/*                                                                     */
/*************************************************************************/
#ifdef SCIPV

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <highs/interfaces/highs_c_api.h>
#include "crpdefns.h"

/* The HiGHS C API has no public infinity constant; 1e20 is the value
   HiGHS itself treats as "infinite" (same convention as csp/Jjsolver.c). */
#define CRP_HIGHS_INF  1.0e20

extern double         CRPbase;       /* base number for rounding             */
extern int            CRPaudit;      /* =0 no auditing;
                                  =1 up to protection levels;
                                  =2 up to optimal extremes            */

extern double         CRPobjval;     /* cost of the best output pattern      */
extern int            CRPfeasible;   /* =1 means that a solution was found   */

extern int            CRPncell;      /* number of cells after reduction      */
extern VARIABLE       *CRPcell;      /* potential variables                  */

extern int            CRPnsum;       /* number of sums after reduction       */
extern CONSTRAINT     *CRPsum;       /* potential constraints                */

extern void*          CRPenvS;       /* HiGHS instance (main MIP)            */
extern void*          CRPsubproblemS;/* HiGHS instance (audit subproblem LP) */



int S_loadsubproblem(double*,double*);
int S_unloadsubproblem(void);
int S_solvesubproblem(int,int,double,double*,double*,double*,double*);


int S_loadsubproblem(double *lb,double *ub)
{
	int        k, card, position, index, newnz;
	double     coef;
	char       *qrtype;
	double     *zobj, *rowlo, *rowhi, *dmatval;
	HighsInt   *mstart, *mclind;
	HighsInt   statusS;

	if( CRPsubproblemS == NULL )
		CRPsubproblemS = Highs_create();
	Highs_clear(CRPsubproblemS);
	/* Highs_clear() resets options to defaults; re-apply quiet logging. */
	Highs_setBoolOptionValue(CRPsubproblemS, "output_flag", 0);
	Highs_setBoolOptionValue(CRPsubproblemS, "log_to_console", 0);

	/* Row senses: 'L' -> x <= rhs, 'E' -> x == rhs, 'G' -> x >= rhs. */
	qrtype = (char *)malloc( CRPnsum * sizeof(char) );
	if( qrtype==NULL ){
		printf("ERROR: not enough memory for SENSE\n");
		return 1;
	}
	for(k=0;k<CRPnsum;k++)
		qrtype[k] = CRPsum[k].sense;

	rowlo = (double *)malloc( CRPnsum * sizeof(double) );
	rowhi = (double *)malloc( CRPnsum * sizeof(double) );
	if( rowlo==NULL || rowhi==NULL ){
		printf("ERROR: not enough memory for RHS\n");
		return 1;
	}
	for(k=0;k<CRPnsum;k++)
	{
		switch( qrtype[k] )
		{
		case 'L':
			rowhi[k] = CRPsum[k].rhs;
			rowlo[k] = -CRP_HIGHS_INF;
			break;
		case 'E':
			rowhi[k] = CRPsum[k].rhs;
			rowlo[k] = rowhi[k];
			break;
		case 'G':
			rowlo[k] = CRPsum[k].rhs;
			rowhi[k] = CRP_HIGHS_INF;
			break;
		default:
			printf("ERROR: invalid row sense\n");
			return 1;
		}
	}

	/* Constraint matrix in column-major (CSC) form. */
	newnz = 0;
	for(k=0;k<CRPnsum;k++)
		newnz += CRPsum[k].card;

	mstart = (HighsInt *)malloc( (CRPnsum+1) * sizeof(HighsInt) );
	if( mstart==NULL ){
		printf("ERROR: not enough memory for MSTART\n");
		return 1;
	}
	mclind = (HighsInt *)malloc( (newnz>0?newnz:1) * sizeof(HighsInt) );
	if( mclind==NULL ){
		printf("ERROR: not enough memory for MCLIND\n");
		return 1;
	}
	dmatval = (double *)malloc( (newnz>0?newnz:1) * sizeof(double) );
	if( dmatval==NULL ){
		printf("ERROR: not enough memory for DMATVAL\n");
		return 1;
	}
	position = 0;
	for(k=0;k<CRPnsum;k++){
		mstart[k] = position;
		card = CRPsum[k].card;
		while(card--){
			index = CRPsum[k].stack[card]->index;
			coef  = CRPsum[k].coef[card];
			mclind[position ] = index;
			dmatval[position ] = coef;
			position++;
		}
	}
	mstart[CRPnsum] = position;

	if( position!=newnz ){
		printf("ERROR: position=%d   newnz=%d\n",position,newnz);
		return 1;
	}

	/* Objective: zero for every column (toggled per cell in
	   S_solvesubproblem).  Column bounds come from the caller
	   (the audit window for each cell).  Maximise, matching the
	   original SCIP_OBJSEN_MAXIMIZE. */
	zobj = (double *)malloc( CRPncell * sizeof(double) );
	if( zobj==NULL ){
		printf("ERROR: not enough memory for OBJ\n");
		return 1;
	}
	for(k=0;k<CRPncell;k++)
		zobj[k] = 0;

	statusS = Highs_passLp(CRPsubproblemS,
		(HighsInt)CRPncell, (HighsInt)CRPnsum, (HighsInt)newnz,
		kHighsMatrixFormatColwise, kHighsObjSenseMaximize, 0.0,
		zobj, lb, ub, rowlo, rowhi,
		mstart, mclind, dmatval);
	free(zobj);
	free( dmatval );
	free( mclind );
	free( mstart );
	free( rowhi );
	free( rowlo );
	free( qrtype );

	if( statusS != kHighsStatusOk ){
		printf("ERROR (status=%d): problems loading HiGHS-solver\n",
		       (int)statusS);
		return 1;
	}

	return 0;
}


int S_unloadsubproblem()
{
	if( CRPsubproblemS != NULL ){
		Highs_clear(CRPsubproblemS);
		CRPsubproblemS = NULL;
	}
	return 0;
}


int S_solvesubproblem(int sense,int var,double bound,double *objval,
                      double *dj,double *dual,double *yval)
{
	HighsInt   statusS, ms;
	double     obj, objval_h;
	int        lpstatus;

	/* Set the objective to push the chosen variable in one direction.
	   Original sense=1  ->  cost +1 (maximise the upper side).
	   Original sense=-1 ->  cost -1 (push the lower side). */
	obj = ( sense==1 ? 1.0 : -1.0 );
	statusS = Highs_changeColCost(CRPsubproblemS, (HighsInt)var, obj);
	if( statusS != kHighsStatusOk ){
		lpstatus = 1;
		goto OUT;
	}

	statusS = Highs_run(CRPsubproblemS);
	if( statusS != kHighsStatusOk ){
		lpstatus = 1;
		goto OUT;
	}

	ms = Highs_getModelStatus(CRPsubproblemS);
	if( ms == kHighsModelStatusOptimal ){
		objval_h = Highs_getObjectiveValue(CRPsubproblemS);
		/* HiGHS reports the objective in the sense it was passed
		   (MAXIMIZE).  For sense==-1 we flipped the sign of the
		   cost, so the reported value is the negative of the true
		   minimum; undo the flip. */
		if( sense==-1 )
			objval_h = -objval_h;
		if( objval )
			*objval = objval_h;
		lpstatus = 0;
	}
	else {
		printf(" lpstat=%d\n", (int)ms);
		lpstatus = 1;
	}

OUT:
	/* Reset the objective coefficient back to zero for the next cell. */
	(void)Highs_changeColCost(CRPsubproblemS, (HighsInt)var, 0.0);

	return lpstatus;
}

#endif
