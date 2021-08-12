/****************************************************/
/* Controlled Rounding Problem for Linked Tables    */
/* in Statistical Disclosure Control (SDC)          */
/*                                                  */
/*                                                  */
/* CRPAUDIT.c                                       */
/* Version 3.0.0                                    */
/* 20 May 2014                  jjsalaza@ull.es     */
/*                                                  */
/*                                                  */
/****************************************************/
#ifdef CPLEXV

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <ilcplex/cplex.h>
#include "crpdefns.h"

/* Global variables for CRP */

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

extern CPXENVptr      CRPenv; 
extern CPXLPptr       CRPmaster;         /* structure for the master problem */
extern CPXLPptr       CRPsubproblem;     /* structure for the subproblem */



int C_loadsubproblem(double*,double*);
int C_unloadsubproblem(void);
int C_solvesubproblem(int,int,double,double*,double*,double*,double*);

////////////////////////////////////////////////////////////

int C_loadsubproblem(lb,ub)
double *lb,*ub;
{
	int        k;
	int        status,newnz;
	int        card,position,index;
	double     coef;
	double     *rhs,*obj,*dmatval;
	char       *qrtype,*cnames,**colnames;
	int        *mstart,*mclind;

	/************  adding names to the variables inside the internal XPRESS model ***************/

    obj = (double *)malloc( CRPncell * sizeof(double) );
	if( obj==NULL ){
		printf("ERROR: not enough memory for OBJ\n");
		return 1;
	}
	for(k=0;k<CRPncell;k++){
		obj[k]    = 0;
	}

	newnz = 0;
	for(k=0;k<CRPncell;k++)
		newnz += (int)strlen( CRPcell[k].name )+1;

	cnames = (char *)malloc ( newnz * sizeof( char ) );
	if( cnames==NULL ){
		printf("ERROR: not enough memory for NAMES\n");
		return 1;
	}
	colnames = (char **)malloc ( CRPncell * sizeof( char* ) );
	if( colnames==NULL ){
		printf("ERROR: not enough memory for NAMES\n");
		return 1;
	}

	newnz = 0;
	for(k=0;k<CRPncell;k++){
		colnames[k] = cnames+newnz;
		strcpy( cnames+newnz , CRPcell[k].name );
		newnz += (int)strlen( CRPcell[k].name )+1;
	}

    CRPsubproblem = CPXcreateprob (CRPenv, &status, "CRPsubproblem");
	if(CRPsubproblem==NULL){
		printf("ERROR (status=%d): problems creating a master problem with SOLVER\n",status);
		return 9;
	}

	status = CPXnewcols (CRPenv, CRPsubproblem, CRPncell, obj, lb, ub, NULL, colnames );
	if(status){
		printf("ERROR (status=%d): problems loading CPLEX-solver\n",status);
		return 1;
	}

	free(colnames);
	free(cnames);
	free(obj);
	

	/************  adding the basic constraints, i.e., the structure of the table ***************/


	qrtype = (char *)malloc( CRPnsum * sizeof(char) );
	if( qrtype==NULL ){
		printf("ERROR: not enough memory for SENSE\n");
		return 1;
	}
	for(k=0;k<CRPnsum;k++)
		qrtype[k] = CRPsum[k].sense;

	rhs = (double *)malloc( CRPnsum * sizeof(double) );
	if( rhs==NULL ){
		printf("ERROR: not enough memory for RHS\n");
		return 1;
	}
	for(k=0;k<CRPnsum;k++)
		rhs[k] = CRPsum[k].rhs;

	mstart = (int *)malloc( (CRPnsum+1) * sizeof(int) );
	if( mstart==NULL ){
		printf("ERROR: not enough memory for MSTART\n");
		return 1;
	}

	newnz = 0;
	for(k=0;k<CRPnsum;k++)
		newnz += CRPsum[k].card;

	mclind = (int *)malloc( newnz * sizeof(int) );
	if( mclind==NULL ){
		printf("ERROR: not enough memory for MCLIND\n");
		return 1;
	}
	dmatval = (double *)malloc( newnz * sizeof(double) );
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
			coef = CRPsum[k].coef[card];
			mclind[ position ] = index;
			dmatval[position ] = coef;
			position++;
		}
	}
	mstart[CRPnsum] = position;

	if( position!=newnz ){
		printf("ERROR: position=%d   newnz=%d\n",position,newnz);
		return 1;
	}

	status = CPXaddrows( CRPenv, CRPsubproblem, 0, CRPnsum, newnz, rhs, qrtype, mstart, mclind, dmatval , NULL, NULL );
	if(status){
		printf("ERROR (status=%d): problems adding rows with CPLEX-solver\n",status);
		return 1;
	}

	free( dmatval );
	free( mclind );
	free( mstart );
	free( rhs );
	free( qrtype );


	/******************   Writing the ILP model in a file  *******/

    CPXchgobjsen (CRPenv, CRPsubproblem, CPX_MAX);  /* Problem is maximization */
//	status = CPXwriteprob( CRPenv, CRPsubproblem , "CRPsubproblem.lp", NULL);

	return 0;
}


int C_unloadsubproblem()
{
	int status;

    status = CPXfreeprob (CRPenv, &CRPsubproblem);
	return 0;
}


int C_solvesubproblem(sense,var,bound,objval,dj,dual,yval)
int sense,var;
double bound;
double *objval,*dj,*dual,*yval;
{
	int    k,status,lpstatus,surplus,nels,mstart,*mrwind;
	double obj,val,*dmatval;
	char   type;

	if( sense==1 ){
		obj = 1;
		status = CPXchgobj( CRPenv, CRPsubproblem, 1, &var, &obj);
	    status = CPXgetub( CRPenv, CRPsubproblem, &val, var, var );
		if( bound < val ){
			type = 'U';
			status = CPXchgbds( CRPenv, CRPsubproblem, 1, &var, &type, &bound);
		}
		status = CPXprimopt( CRPenv, CRPsubproblem );
	} else {
		obj = -1;
		status = CPXchgobj( CRPenv, CRPsubproblem, 1, &var, &obj);
	    status = CPXgetlb( CRPenv, CRPsubproblem, &val, var, var );
		if( bound > val ){
			type = 'L';
			status = CPXchgbds( CRPenv, CRPsubproblem, 1, &var, &type, &bound);
		}
		status = CPXprimopt( CRPenv, CRPsubproblem );
	}
	lpstatus = CPXgetstat( CRPenv, CRPsubproblem );
//	if( lpstatus==CPX_STAT_ABORT_OBJ_LIM ){   //cplex 8.0
	if( lpstatus==CPX_OBJ_LIM ){   //cplex 7.0
		status = CPXdualopt( CRPenv, CRPsubproblem );
		lpstatus = CPXgetstat( CRPenv, CRPsubproblem );
	}

	switch( lpstatus ){



//	case CPX_STAT_OPTIMAL :                 // cplex 8.0
//	case CPX_STAT_OPTIMAL_INFEAS :
//	case CPX_STAT_NUM_BEST :
	case CPX_OPTIMAL :                       // cplex 7.0
	case CPX_OPTIMAL_INFEAS :
	case CPX_NUM_BEST_FEAS :

		status = CPXgetobjval( CRPenv, CRPsubproblem , objval );
		if( sense==-1 )
			(*objval) = -(*objval);

		if( yval )
			status = CPXgetx( CRPenv, CRPsubproblem, yval, 0, CRPncell-1 );
		
		if( dj==NULL || dual==NULL )
			break;

		status = CPXgetpi( CRPenv, CRPsubproblem, dual, 0, CRPnsum-1 );
			
		mrwind   = (int *)malloc( CRPnsum  * sizeof(int) );
		dmatval  = (double *)malloc( CRPnsum  * sizeof(double) );
		for(k=0;k<CRPncell;k++){
			CPXgetcols( CRPenv, CRPsubproblem, &nels, &mstart, mrwind, dmatval, CRPnsum, &surplus, k, k);
			dj[k] = 0;
			while(nels--)
				dj[k] -= dual[mrwind[nels]]*dmatval[nels];
		}
		dj[var] += ( sense==1 ? 1 : -1 );

		free(mrwind);
		free(dmatval);

		lpstatus = 0;
		break;

	default:
		printf(" lpstat=%d\n",lpstatus);
		lpstatus = 1;
	}

//	status = CPXwriteprob( CRPenv, CRPsubproblem , "CRPsubproblem2.lp", NULL);

	obj = 0;
	status = CPXchgobj( CRPenv, CRPsubproblem, 1, &var, &obj);
	if( sense==1 ){
		if( bound < val ){
			type = 'U';
			status = CPXchgbds( CRPenv, CRPsubproblem, 1, &var, &type, &val);
		}
	} else {
		if( bound > val ){
			type = 'L';
			status = CPXchgbds( CRPenv, CRPsubproblem, 1, &var, &type, &val);
		}
	}
	return lpstatus;
}

#endif
