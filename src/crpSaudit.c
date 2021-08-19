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
#ifdef SCIPV

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <scip/scip.h>
#include "crpdefns.h"

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

extern SCIP *         CRPenvS;
extern SCIP_LPI *     CRPsubproblemS;     /* structure for the subproblem */




int S_loadsubproblem(double*,double*);
int S_unloadsubproblem(void);
int S_solvesubproblem(int,int,double,double*,double*,double*,double*);


int S_loadsubproblem(lb,ub)
double *lb,*ub;
{
	int        k;
    int        newnz;
	int        card,position,index;
	double     coef;
    double     *obj,*dmatval, *rhsS, *lhsS;
	char       *qrtype,*cnames,**colnames;
	int        *mstart,*mclind;
    SCIP_RETCODE statusS;

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

    statusS = SCIPlpiCreate(&CRPsubproblemS, NULL, "CRPsubproblem", SCIP_OBJSEN_MINIMIZE);
    if(statusS != SCIP_OKAY){
        printf("ERROR (status=%d): problems creating a master problem with SCIP\n",statusS);
        return 9;
    }

    statusS = SCIPlpiAddCols(CRPsubproblemS, CRPncell, obj, lb, ub, colnames, 0, NULL, NULL, NULL );
    if(statusS != SCIP_OKAY){
        printf("ERROR (status=%d): problems loading SCIP-solver\n",statusS);
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

    rhsS = (double *)malloc( CRPnsum * sizeof(double) );
    if( rhsS==NULL ){
        printf("ERROR: not enough memory for RHS\n");
        return 1;
    }
    lhsS = (double *)malloc( CRPnsum * sizeof(double) );
    if( lhsS==NULL ){
        printf("ERROR: not enough memory for LHS\n");
        return 1;
    }
    for(k=0;k<CRPnsum;k++)
    {
        switch( qrtype[k] )
        {
        case 'L':
                rhsS[k] = CRPsum[k].rhs;
                lhsS[k] = -SCIPlpiInfinity(CRPsubproblemS);
                break;
        case 'E':
                rhsS[k] = CRPsum[k].rhs;
                lhsS[k] = rhsS[k];
                break;
        case 'G':
                lhsS[k] = CRPsum[k].rhs;
                rhsS[k] = SCIPlpiInfinity(CRPsubproblemS);
                break;
        default:
                SCIPerrorMessage("invalid row sense\n");
        }        
    }

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

    statusS = SCIPlpiAddRows(CRPsubproblemS, CRPnsum, lhsS, rhsS, NULL, newnz, mstart, mclind, dmatval);
    if(statusS != SCIP_OKAY){
        printf("ERROR (status=%d): problems adding rows with SCIP-solver\n",statusS);
        return 1;
    }

	free( dmatval );
	free( mclind );
	free( mstart );
    free( rhsS );
    free( lhsS );
	free( qrtype );


	/******************   Writing the ILP model in a file  *******/

    statusS = SCIPlpiChgObjsen(CRPsubproblemS, SCIP_OBJSEN_MAXIMIZE);
    //statusS = SCIPlpiWriteLP(CRPsubproblemS, "CRPsubproblemS.lp");

	return 0;
}


int S_unloadsubproblem()
{
    //SCIP_RETCODE statusS; // PWOF: statusS is not used

    //statusS = SCIPlpiFree(&CRPsubproblemS);
    SCIPlpiFree(&CRPsubproblemS);
    return 0;
}

int S_solvesubproblem(sense,var,bound,objval,dj,dual,yval)
int sense,var;
double bound;
double *objval,*dj,*dual,*yval;
{
	int    k,lpstatus,nels,mstart,*mrwind;
	double obj,val,*dmatval;
	//char   type;
        //SCIP_RETCODE statusS;

	if( sense==1 ){
		obj = 1;
                //statusS = SCIPlpiChgObj(CRPsubproblemS, 1, &var, &obj);
                SCIPlpiChgObj(CRPsubproblemS, 1, &var, &obj);
                //statusS = SCIPlpiGetBounds(CRPsubproblemS, var, var, 0, &val);
                SCIPlpiGetBounds(CRPsubproblemS, var, var, 0, &val);
		if( bound < val ){
                    //type = 'U'; // PWOF: never used
                    //statusS = SCIPlpiGetBounds(CRPsubproblemS, var, var, 0, &bound);
                    SCIPlpiGetBounds(CRPsubproblemS, var, var, 0, &bound);
		}
                //statusS = SCIPlpiSolvePrimal(CRPsubproblemS);
                SCIPlpiSolvePrimal(CRPsubproblemS);
	} else {
		obj = -1;
                //statusS = SCIPlpiChgObj(CRPsubproblemS, 1, &var, &obj);
                SCIPlpiChgObj(CRPsubproblemS, 1, &var, &obj);
                //statusS = SCIPlpiGetBounds(CRPsubproblemS, var, var, &val, 0);
                SCIPlpiGetBounds(CRPsubproblemS, var, var, &val, 0);
		if( bound > val ){
			//type = 'L'; // PWOF: never used
                        //statusS = SCIPlpiGetBounds(CRPsubproblemS, var, var, &bound, 0);
                        SCIPlpiGetBounds(CRPsubproblemS, var, var, &bound, 0);
		}
                //statusS = SCIPlpiSolvePrimal(CRPsubproblemS);
                SCIPlpiSolvePrimal(CRPsubproblemS);
	}

    if (SCIPlpiIsObjlimExc(CRPsubproblemS))
        //statusS = SCIPlpiSolveDual(CRPsubproblemS);
        SCIPlpiSolveDual(CRPsubproblemS);

    if (SCIPlpiIsOptimal (CRPsubproblemS))
    {
        //statusS = SCIPlpiGetSol(CRPsubproblemS, objval, 0, 0, 0, 0);
        SCIPlpiGetSol(CRPsubproblemS, objval, 0, 0, 0, 0);
        if( sense==-1 )
            (*objval) = -(*objval);

        if( yval )
            //statusS = SCIPlpiGetSol(CRPsubproblemS, 0, yval, 0, 0, 0);
            SCIPlpiGetSol(CRPsubproblemS, 0, yval, 0, 0, 0);

        if( dj==NULL || dual==NULL ){
			lpstatus = 0;
            goto OUT; //break;
		}

        //statusS = SCIPlpiGetSol(CRPsubproblemS, 0, 0, dual, 0, 0);
        SCIPlpiGetSol(CRPsubproblemS, 0, 0, dual, 0, 0);

        mrwind   = (int *)malloc( CRPnsum  * sizeof(int) );
        dmatval  = (double *)malloc( CRPnsum  * sizeof(double) );
        for(k=0;k<CRPncell;k++){
            //statusS = SCIPlpiGetCols(CRPsubproblemS, k, k, 0, 0, &nels, &mstart, mrwind, dmatval);
            SCIPlpiGetCols(CRPsubproblemS, k, k, 0, 0, &nels, &mstart, mrwind, dmatval);
            dj[k] = 0;
            while(nels--)
                dj[k] -= dual[mrwind[nels]]*dmatval[nels];
        }
        dj[var] += ( sense==1 ? 1 : -1 );

        free(mrwind);
        free(dmatval);

        lpstatus = 0;
    }else{
        printf(" lpstat=%d\n", SCIPlpiGetInternalStatus( CRPsubproblemS ) );
        lpstatus = 1;
    }
OUT:
    //statusS = SCIPlpiWriteLP(CRPsubproblemS, "CRPsubproblemS2.lp");

	obj = 0;
        //statusS = SCIPlpiChgObj(CRPsubproblemS, 1, &var, &obj);
        SCIPlpiChgObj(CRPsubproblemS, 1, &var, &obj);
	if( sense==1 ){
		if( bound < val ){
                    //type = 'U'; // PWOF: never used
                    //statusS = SCIPlpiChgBounds(CRPsubproblemS, 1, &var, 0, &val);
                    SCIPlpiChgBounds(CRPsubproblemS, 1, &var, 0, &val);
		}
	} else {
		if( bound > val ){
                    //type = 'L'; // PWOF: never used
                    //statusS = SCIPlpiChgBounds(CRPsubproblemS, 1, &var, &val, 0);
                    SCIPlpiChgBounds(CRPsubproblemS, 1, &var, &val, 0);
		}
	}
	return lpstatus;
}

#endif
