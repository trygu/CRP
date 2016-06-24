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
#ifdef XPRESSV

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "xprs.h"
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

extern XPRSprob       CRPmaster;         /* XPRESS structure for the master problem */
extern XPRSprob       CRPsubproblem;     /* XPRESS structure for the subproblem */

extern void XPRS_CC Message(XPRSprob, void*, const char *, int, int);



int X_loadsubproblem(double*,double*);
int X_unloadsubproblem(void);
int X_solvesubproblem(int,int,double,double*,double*,double*);


int X_loadsubproblem(lb,ub)
double *lb,*ub;
{
	int        k;
	int        status,newnz;
	int        card,position,index;
	double     coef;
	double     *rhs,*obj,*dmatval;
	char       *qrtype,*cnames;
	int        *mstart,*mnel,*mclind;

	status = XPRScreateprob(&CRPsubproblem);
	if(status){
		printf("ERROR (status=%d): problems creating a subproblem with XPRESS\n",status);
		return 9;
	}
	// XPRSsetlogfile( CRPmaster , NULL );
//    status = XPRSsetcbmessage( CRPmaster , Message , NULL );



	obj = (double *)malloc( CRPncell * sizeof(double) );
	if( obj==NULL ){
		printf("ERROR: not enough memory for OBJ\n");
		return 1;
	}
	mstart = (int *)malloc( CRPncell * sizeof(int) );
	if( mstart==NULL ){
		printf("ERROR: not enough memory for MSTART\n");
		return 1;
	}
	mnel = (int *)malloc( CRPncell * sizeof(int) );
	if( mnel==NULL ){
		printf("ERROR: not enough memory for MSTART\n");
		return 1;
	}
	for(k=0;k<CRPncell;k++){
		obj[k]    = 0;
		mstart[k] = 0;
		mnel[k]   = 0;
	}

	newnz = 0;
	for(k=0;k<CRPnsum;k++)
		newnz += CRPsum[k].card;
	status = XPRSsetintcontrol( CRPmaster , XPRS_EXTRAELEMS , newnz );
	status = XPRSsetintcontrol( CRPmaster , XPRS_EXTRAROWS , CRPnsum );
//	status = XPRSsetintcontrol( CRPmaster , XPRS_MAXTIME , -MAX_TIME );
//	status = XPRSsetintcontrol( CRPmaster , XPRS_SOLUTIONFILE , 0 );   jj: nov-2013
//	status = XPRSsetintcontrol( CRPmaster , XPRS_CUTDEPTH , 50);
//	status = XPRSsetintcontrol( CRPmaster , XPRS_BACKTRACK , 2);
//	status = XPRSsetintcontrol( CRPmaster , XPRS_CUTSTRATEGY , 3);
//	status = XPRSsetintcontrol( CRPmaster , XPRS_HEURDEPTH , 10);
//	status = XPRSsetintcontrol( CRPmaster , XPRS_HEURSTRATEGY , 1);
//	status = XPRSsetintcontrol( CRPmaster , XPRS_GOMCUTS , 1);
//	status = XPRSsetintcontrol( CRPmaster , XPRS_COVERCUTS , 100);
//	status = XPRSsetintcontrol( CRPmaster , XPRS_TREECOVERCUTS , 10);
//	status = XPRSsetintcontrol( CRPmaster , XPRS_VARSELECTION , 4);
//	status = XPRSsetintcontrol( CRPmaster , XPRS_MIPPRESOLVE , 7);

    status = XPRSloadlp ( CRPsubproblem, "CRPsubproblem", CRPncell, 0, NULL, NULL,
		               NULL, obj, mstart, mnel, NULL, NULL, lb, ub);
	
	if(status){
		printf("ERROR (status=%d): problems loading XPRESS-solver\n",status);
		return 1;
	}

	free(mstart);
	free(mnel);
	free(obj);

	/************  adding names to the variables inside the internal XPRESS model ***************/

	newnz = 0;
	for(k=0;k<CRPncell;k++)
		newnz += (int)strlen( CRPcell[k].name )+1;

	cnames = (char *)malloc ( newnz * sizeof( char ) );
	if( cnames==NULL ){
		printf("ERROR: not enough memory for NAMES\n");
		return 1;
	}
	newnz = 0;
	for(k=0;k<CRPncell;k++){
		strcpy( cnames+newnz , CRPcell[k].name );
		newnz += (int)strlen( CRPcell[k].name )+1;
	}

	status = XPRSaddnames( CRPsubproblem, 2, cnames, 0, CRPncell-1 );

	free(cnames);




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

	status = XPRSaddrows( CRPsubproblem, CRPnsum, newnz, qrtype, rhs, NULL, mstart, mclind, dmatval );
	if(status){
		printf("ERROR (status=%d): problems adding rows with XPRESS-solver\n",status);
		return 1;
	}

	free( dmatval );
	free( mclind );
	free( mstart );
	free( rhs );
	free( qrtype );


	/******************   Writing the ILP model in a file  *******/

//	status = XPRSwriteprob( CRPsubproblem , "CRPsubproblem","l");
	return 0;
}


int X_unloadsubproblem()
{
	int status;
	status = XPRSdestroyprob( CRPsubproblem );
	return 0;
}


int X_solvesubproblem(sense,var,bound,objval,dj,dual)
int sense,var;
double bound;
double *objval,*dj,*dual;
{
	int    k,status,lpstatus,nels;
	double obj,val;
	int    *mrwind;
	double *dmatval;
	int    mstart[3];
	char   type;

//	status = XPRSsetdblcontrol( CRPsubproblem , XPRS_MIPABSCUTOFF , bound );
	if( sense==1 ){
		obj = 1;
		status = XPRSchgobj( CRPsubproblem, 1, &var, &obj);
	    status = XPRSgetub( CRPsubproblem, &val, var, var );
		if( bound < val ){
			type = 'U';
			status = XPRSchgbounds( CRPsubproblem, 1, &var, &type, &bound);
		}
		status = XPRSmaxim( CRPsubproblem , "d" );
	} else {
		obj = -1;
		status = XPRSchgobj( CRPsubproblem, 1, &var, &obj);
	    status = XPRSgetlb( CRPsubproblem, &val, var, var );
		if( bound > val ){
			type = 'L';
			status = XPRSchgbounds( CRPsubproblem, 1, &var, &type, &bound);
		}
		status = XPRSmaxim( CRPsubproblem , "d" );
	}
	status = XPRSgetintattrib( CRPsubproblem , XPRS_LPSTATUS , &lpstatus );

	switch( lpstatus ){
	case XPRS_LP_CUTOFF :
		printf("ERROR: subproblem cuttoff! (status=%d)\n",lpstatus);
		lpstatus = 1;
		break;
	case XPRS_LP_CUTOFF_IN_DUAL :
		printf("ERROR: subproblem cuttoff in dual! (status=%d)\n",lpstatus);
		lpstatus = 1;
		break;
	case XPRS_LP_INFEAS :
		printf("ERROR: subproblem infeas! (status=%d)\n",lpstatus);
    	status = XPRSwriteprob( CRPsubproblem , "CRPsubproblem","l");
		lpstatus = 1;
		break;
	case XPRS_LP_UNBOUNDED :
		printf("ERROR: subproblem unbounded! (status=%d)\n",lpstatus);
		lpstatus = 1;
		break;
	case XPRS_LP_UNFINISHED :
		printf("ERROR: subproblem not finished! (status=%d)\n",lpstatus);
		lpstatus = 1;
		break;
	case XPRS_LP_OPTIMAL :

		status = XPRSgetdblattrib( CRPsubproblem , XPRS_LPOBJVAL, objval );
		if( sense==-1 )
			(*objval) = -(*objval);

		if( dj==NULL || dual==NULL )
			break;

		status = XPRSgetsol( CRPsubproblem , NULL, NULL, dual, NULL );

		mrwind   = (int *)malloc( CRPnsum  * sizeof(int) );
		dmatval  = (double *)malloc( CRPnsum  * sizeof(double) );
		for(k=0;k<CRPncell;k++){
			XPRSgetcols( CRPsubproblem, mstart, mrwind, dmatval, CRPnsum, &nels, k, k);
			dj[k] = 0;
			while(nels--)
				dj[k] -= dual[mrwind[nels]]*dmatval[nels];
		}
		dj[var] += ( sense==1 ? 1 : -1 );

		free(mrwind);
		free(dmatval);

		lpstatus = 0;
		break;
	}

//	status = XPRSwriteprob( CRPsubproblem , "CRPsubproblem","l");

	obj = 0;
	status = XPRSchgobj( CRPsubproblem, 1, &var, &obj);
	if( sense==1 ){
		if( bound < val ){
			type = 'U';
			status = XPRSchgbounds( CRPsubproblem, 1, &var, &type, &val);
		}
	} else {
		if( bound > val ){
			type = 'L';
			status = XPRSchgbounds( CRPsubproblem, 1, &var, &type, &val);
		}
	}
	return lpstatus;
}

#endif
