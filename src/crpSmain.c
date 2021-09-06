/****************************************************/
/* Controlled Rounding Problem for Linked Tables    */
/* in Statistical Disclosure Control (SDC)          */
/*                                                  */
/*                                                  */
/* CRPMAIN.c                                        */
/* Version 3.0.0                                    */
/* 20 May 2014                  jjsalaza@ull.es     */
/*                                                  */
/*                                                  */
/****************************************************/
#ifdef SCIPV
//#define PERTURBATION
#define ROUNDING

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/cons_linear.h>
#include "crpmain.h"


SCIP *          CRPenvS;
SCIP_LPI *      CRPsubproblemS;     /* structure for the subproblem */



static int S_MIPmodel(int);
//static int S_unMIPsolver(void);
//static int S_imcompatibility(int,char*,double*,int*,int*,int*,int*,double*);

extern int S_loadsubproblem(double*,double*);
extern int S_unloadsubproblem(void);
extern int S_solvesubproblem(int,int,double,double*,double*,double*,double*);



SCIP_RETCODE SCIPincludeEventHdlrTotal(SCIP* scip);


/*  FUNCTIONS  */

int S_CRPopenSOLVER()
{
    SCIP_RETCODE statusS;

    statusS = SCIPcreate(&CRPenvS);
    if (statusS != SCIP_OKAY){
        fprintf (stderr, "Could not open SCIP environment.\n");
        SCIPprintError(statusS);
		return -1;
    }
    printf("SCIP version is %lf \n", SCIPversion());
    printf("     using %s\n\n", SCIPlpiGetSolverName());

    statusS = SCIPincludeDefaultPlugins(CRPenvS) ;
    SCIPsetIntParam(CRPenvS, "display/verblevel", 0);

	return 0;
}

int S_CRPcloseSOLVER()
{

    if ( CRPenvS != NULL ) {
        SCIP_RETCODE statusS = SCIPfree(&CRPenvS);
        if ( statusS != SCIP_OKAY ) {
            fprintf (stderr, "Could not close SCIP environment.\n");
            SCIPprintError(statusS);
        }
    }
     
	return 0;
}




int  S_CRPoptimize(lowerbound,upperbound)
double *lowerbound,*upperbound;
{
    int status,k;

    CRPfeasible = 0;
    CRPobjval   = 0;
    CRPextended = 0;
    CRPabort       = 0;
    CRPlowerbound  = INF;
    InitialTime    = (double)clock()/CLOCKS_PER_SEC;;
    MaximumTime    = MAX_TIME;
    basicOBJvalue  = 0;
    for(k=0;k<CRPncell;k++) basicOBJvalue += CRPcell[k].weight * (CRPcell[k].nominal - CRPcell[k].nominaldown); 

    status = S_MIPmodel(0);
    *upperbound = CRPobjval;
    *lowerbound = CRPlowerbound;

    if( status==9 )
        return -1;
    if( status==2 ){
        CRPextended = 1;
        status = S_MIPmodel(1);
    }

    if( status==1 ){
        printf("Warning: bad result from MIPsolver : status=%d\n",status);
	return status;
    }

    *upperbound = CRPobjval;
    *lowerbound = CRPlowerbound;
    return 0;
}


/******

  variables:  3*CRPncells+1
			  the first family corresponds to "rounding down (=0) or up (=1)"
			  the second family corresponds to "rounding up to the second iff =1"
			  the third family corresponds to "rounding down to the second iff =1"
              the last one is a dummy variable to ensure feasibility

*****/
clock_t tttt0;
/********************************   integer model : controlled rounding ***********************************/

static int S_MIPmodel(large)
int large;
{
    int        i,h,k,lb,ub,tb;
    int        ncol,newnz;
    int        card,position,index;
    double     coef,penalty;
    CONSTRAINT *sum;
    VARIABLE   *cell;
    double     *obj,*dlb,*dub,*matval,*xval, *rhsS, *lhsS;
    char       *sense,*ctype,*cnames,**colnames;
    int        *matcnt;
    SCIP_VAR   **matind;
    clock_t    t1,t2;
    SCIP_RETCODE statusS;
	SCIP_VAR   **SCIPcols;
	SCIP_CONS  **SCIProws;
//    SCIP_RESULT result;
//    FILE       *filelp;
	SCIP_SOL   *sol;
    SCIP_CONS  *row;
	SCIP_STATUS mipstatus;

	if( large )
	    printf("..... finding an output pattern of the extended CRP\n");
	else
	    printf("..... finding an output pattern of the zero-restricted CRP\n");

    tttt0 = t1 = clock();

	penalty = ( CRPncell > CRPbase*100 ? CRPbase*100 : CRPncell );
	if( large )
	    ncol = 3*CRPncell;
	else
		ncol = CRPncell;

	obj = (double *)malloc( ncol * sizeof(double) );
	if( obj==NULL ){
		printf("ERROR: not enough memory for OBJ\n");
		return 1;
	}
	for(k=0;k<CRPncell;k++){
		obj[k] = ( CRPcell[k].nominal - CRPcell[k].nominaldown < ZERO ? 
			penalty : CRPbase - 2 * (CRPcell[k].nominal - CRPcell[k].nominaldown) );
		obj[k] *= CRPcell[k].weight;
		if( large )
			obj[k+CRPncell] = obj[k+2*CRPncell] = penalty + obj[k];
	}

	dlb = (double *)malloc( ncol * sizeof(double) );
	if( dlb==NULL ){
		printf("ERROR: not enough memory for OBJ\n");
		return 1;
	}
	dub = (double *)malloc( ncol * sizeof(double) );
	if( dub==NULL ){
		printf("ERROR: not enough memory for OBJ\n");
		return 1;
	}
	ctype = (char *)malloc( ncol * sizeof(char) );
	if( ctype==NULL ){
		printf("ERROR: not enough memory for OBJ\n");
		return 1;
	}
	SCIPcols = (SCIP_VAR **)malloc( ncol * sizeof(SCIP_VAR *) );
	if( SCIPcols==NULL ){
		printf("ERROR: not enough memory for SCIPcols\n");
		return 1;
	}

	for(k=0;k<CRPncell;k++){
		lb = (int)(ceil ( (CRPcell[k].lvalue -ZERO)/CRPbase )+ZERO);
		ub = (int)(floor( (CRPcell[k].uvalue +ZERO)/CRPbase )+ZERO);
		tb = (int)(floor( (CRPcell[k].nominal+ZERO)/CRPbase )+ZERO);


#ifdef ROUNDING
		dlb[k] = 0;
		dub[k] = 1;

		if( CRPcell[k].lvalue > CRPcell[k].nominaldown+ZERO || CRPcell[k].nominal+CRPcell[k].upl > CRPcell[k].nominaldown+CRPbase+ZERO)
			dlb[k] = 1;

		if( CRPcell[k].uvalue < CRPcell[k].nominaldown+CRPbase-ZERO || CRPcell[k].nominal-CRPcell[k].lpl < CRPcell[k].nominaldown-ZERO)
			dub[k] = 0;

		if( dlb[k]>dub[k] ){
			printf("Warning: variable %d with LPL=%f,UPL=%f\n",k,CRPcell[k].lpl,CRPcell[k].upl);
			dlb[k] = 0;
			dub[k] = 1;
		}

		ctype[k] = 'B';
#endif

#ifdef PERTURBATION
		dlb[k] = 0;
		dub[k] = 1;

		coef = (CRPcell[k].lvalue - CRPcell[k].nominaldown )/CRPbase;
		if( coef > dlb[k] )
			dlb[k] = coef;
		coef = (CRPcell[k].nominal + CRPcell[k].upl - CRPbase - CRPcell[k].nominaldown )/CRPbase;
		if( coef > dlb[k] )
			dlb[k] = coef;
// printf(" xval[%d] >= %lf \n",k,coef);

		coef = (CRPcell[k].uvalue - CRPcell[k].nominaldown )/CRPbase;
		if( coef < dub[k] )
			dub[k] = coef;
		coef = (CRPcell[k].nominal - CRPcell[k].lpl + CRPbase - CRPcell[k].nominaldown )/CRPbase;
		if( coef < dub[k] )
			dub[k] = coef;
//printf(" xval[%d] <= %lf \n",k,coef);

		if( dlb[k]>dub[k] ){
			printf("Warning: variable %d with LPL=%f,UPL=%f\n",k,CRPcell[k].lpl,CRPcell[k].upl);
			dlb[k] = 0;
			dub[k] = 1;
		}

		ctype[k] = 'C';
#endif
		
		
		
		if( !large && CRPcell[k].nominal - CRPcell[k].nominaldown < ZERO )
			dub[k] = 0;

		if( large ){
			dlb[k+CRPncell] = 0;
			dub[k+CRPncell] = ( ub > tb ? ub-tb-1 : 0 );
			ctype[k+CRPncell] = 'I';

			dlb[k+2*CRPncell] = 0;
			dub[k+2*CRPncell] = ( lb > tb ? 0 : tb-lb );	
			ctype[k+2*CRPncell] = 'I';
		}
	}


	newnz = 0;
	for(k=0;k<CRPncell;k++)
		newnz += (int)strlen( CRPcell[k].name )+1;
	if( large )
		newnz *= 3;

	cnames = (char *)malloc ( newnz * sizeof( char ) );
	if( cnames==NULL ){
		printf("ERROR: not enough memory for NAMES\n");
		return 1;
	}
	colnames = (char **)malloc ( ncol * sizeof( char* ) );
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
	if( large ){
		for(k=0;k<CRPncell;k++){
  		    colnames[CRPncell+k] = cnames+newnz;
			strcpy( cnames+newnz , CRPcell[k].name );
			cnames[newnz] = 'y';
			newnz += (int)strlen( CRPcell[k].name )+1;
		}
		for(k=0;k<CRPncell;k++){
  		    colnames[2*CRPncell+k] = cnames+newnz;
			strcpy( cnames+newnz , CRPcell[k].name );
			cnames[newnz] = 'z';
			newnz += (int)strlen( CRPcell[k].name )+1;
		}
	}


    statusS = SCIPcreateProb(CRPenvS, "CRPmasterS", NULL, NULL, NULL, NULL, NULL, NULL, NULL) ;
    statusS = SCIPsetObjsense(CRPenvS, SCIP_OBJSENSE_MINIMIZE) ;
    if(statusS != SCIP_OKAY){
        printf("ERROR (status=%d): problems creating a master problem with SCIP\n",statusS);
        return 9;
    }

    for (k=0;k<ncol;k++)
    {
        SCIP_VAR *col;
        switch (ctype[k]) {
        case 'B':
            statusS = SCIPcreateVar(CRPenvS, &col, colnames[k], dlb[k], dub[k], obj[k], SCIP_VARTYPE_BINARY, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL);
            break;
        case 'I':
            statusS = SCIPcreateVar(CRPenvS, &col, colnames[k], dlb[k], dub[k], obj[k], SCIP_VARTYPE_INTEGER, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL);
            break;
        default:  //'C'
            statusS = SCIPcreateVar(CRPenvS, &col, colnames[k], dlb[k], dub[k], obj[k], SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL);
            break;
        }
        statusS = SCIPaddVar(CRPenvS, col);
		SCIPcols[k] = col;
    }
    if(statusS != SCIP_OKAY){
        printf("ERROR (status=%d): problems loading SCIP-solver\n",statusS);
        return 1;
    }


	/************  adding the basic constraints, i.e., the structure of the table ***************/

	newnz = 0;
	for(k=0;k<CRPnsum;k++)
		newnz += CRPsum[k].card;
	if( large )
		newnz *= 3;
    matind = (SCIP_VAR**)malloc( newnz * sizeof(SCIP_VAR*) );
    if( matind==NULL ){
		printf("ERROR: not enough memory for MCLIND\n");
		return 1;
	}
	matcnt = (int *)malloc( newnz * sizeof(int) );
	if( matcnt==NULL ){
		printf("ERROR: not enough memory for MCNTIND\n");
		return 1;
	}
	matval = (double *)malloc( newnz * sizeof(double) );
	if( matval==NULL ){
		printf("ERROR: not enough memory for DMATVAL\n");
		return 1;
	}

	sense = (char *)malloc( CRPnsum * sizeof(char) );
	if( sense==NULL ){
		printf("ERROR: not enough memory for SENSE\n");
		return 1;
	}
	for(k=0;k<CRPnsum;k++)
		sense[k] = CRPsum[k].sense;

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
	SCIProws = (SCIP_CONS **)malloc( CRPnsum * sizeof(SCIP_CONS *) );
	if( SCIProws==NULL ){
		printf("ERROR: not enough memory for SCIProws\n");
		return 1;
	}
    for(k=0;k<CRPnsum;k++){
        switch( sense[k] )
        {
            case 'L':
                    rhsS[k] = CRPsum[k].rhs;
                    sum = CRPsum+k;
                    h = sum->card;
                    while(h--){
                        cell = sum->stack[h];
                        rhsS[k] -= sum->coef[h] * cell->nominaldown;
                    }
                    rhsS[k] = rhsS[k] / CRPbase;
                    lhsS[k] = -SCIPinfinity(CRPenvS);
                    break;
            case 'E':
                    rhsS[k] = CRPsum[k].rhs;
                    sum = CRPsum+k;
                    h = sum->card;
                    while(h--){
                        cell = sum->stack[h];
                        rhsS[k] -= sum->coef[h] * cell->nominaldown;
                    }
                    rhsS[k] = rhsS[k] / CRPbase;
                    lhsS[k] = rhsS[k];
                    break;
            case 'G':
                    lhsS[k] = CRPsum[k].rhs;
                    sum = CRPsum+k;
                    h = sum->card;
                    while(h--){
                        cell = sum->stack[h];
                        rhsS[k] -= sum->coef[h] * cell->nominaldown;
                    }
                    lhsS[k] = lhsS[k] / CRPbase;
                    rhsS[k] = SCIPinfinity(CRPenvS);
                    break;
            default:
                    SCIPerrorMessage("invalid row sense\n");
        }        
	}

	position = 0;
	for(k=0;k<CRPnsum;k++){
		matcnt[k] = card = CRPsum[k].card;
        while(card--){
			index = CRPsum[k].stack[card]->index;
			coef = CRPsum[k].coef[card];
            matind[ position ] = SCIPcols[index];
            matval[position ] = coef;
			position++;
            if( large ){
                matind[ position ] = SCIPcols[CRPncell+index];
				matval[position ] = coef;
				position++;
                matind[ position ] = SCIPcols[2*CRPncell+index];
				matval[position ] = -coef;
                position++;
                matcnt[k] += 2;
            }
		}
        statusS = SCIPcreateConsLinear(CRPenvS, &row, "x", matcnt[k], matind, matval, lhsS[k], rhsS[k], TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE) ;
        statusS = SCIPaddCons(CRPenvS, row);
		SCIProws[k] = row;
        position = 0;
	}

    /*if( position!=newnz ){
		printf("ERROR: position=%d   newnz=%d\n",position,newnz);
		return 1;
    }*/

    if(statusS != SCIP_OKAY){
            printf("ERROR (status=%d): problems adding rows with SCIP-solver\n",statusS);
            return 1;
    }

    free( ctype );
	free( dub );
	free( dlb );
	free( obj );
	free( cnames );
	free( colnames );
	free( matval );
    free( matind );
	free( matcnt );	
    free( rhsS );
    free( lhsS );
    free( sense );


	/******************   Writing the ILP model in a file  *******
	filelp = fopen("CRPmodelS.lp", "w");
    if (SCIP_ERROR == SCIPwriteLp(
                    CRPenvS,               //< SCIP data structure /
                    filelp,               //< coutput file, or NULL if standard coutput should be used /
                    "CRPmaster",               //< problem name /
                    FALSE,        //< TRUE iff problem is the transformed problem /
                    SCIPgetObjsense(CRPenvS),           //< objective sense /
                    1.0,           //< scalar applied to objective function; external objective value is extobj = objsense * objscale * (intobj + objoffset) /
                    0.0,          //< objective offset from bound shifting and fixing /
                    SCIPgetVars(CRPenvS),               //< array with active variables ordered binary, integer, implicit, continuous /
                    SCIPgetNVars(CRPenvS),              //< number of mutable variables in the problem /
                    SCIPgetNBinVars(CRPenvS),           //< number of binary variables /
                    SCIPgetNIntVars(CRPenvS),           //< number of general integer variables /
                    SCIPgetNImplVars(CRPenvS),          //< number of implicit integer variables /
                    SCIPgetNContVars(CRPenvS),          //< number of continuous variables /
                    SCIPgetConss(CRPenvS), //constr,    //< array with constraints of the problem /
                    SCIPgetNConss(CRPenvS),             //< number of constraints in the problem /
                    &result              //< pointer to store the result of the file writing call /
                    )) 
			printf("ERROR: Write LP\n");
    fclose(filelp);

	******************   solving the ILP model  *******/


    /* Assure linear mappings between the presolved and original models */
    //status = CPXsetintparam (CRPenv, CPX_PARAM_PRELINEAR, 0);

    /* Let MIP callbacks work on the original model */
    //status = CPXsetintparam (CRPenv, CPX_PARAM_MIPCBREDLP, CPX_OFF);

    /* since the new constraints are not imply by the basic model */
    //status = CPXsetintparam (CRPenv, CPX_PARAM_REDUCE, 0);


    //statusS = SCIPsetRealParam(CRPenvS, "limits/time", MAX_TIME);

    SCIPincludeEventHdlrTotal(CRPenvS);
    //statusS = SCIPsetRealParam(CRPenvS,"numerics/infinity", INF);

    //CRPmessage(-SCIPinfinity(CRPenvS), SCIPinfinity(CRPenvS), 0, 0);
    
    statusS = SCIPsolve(CRPenvS);
    if( statusS != SCIP_OKAY ){
        printf("Warning: bad result from SCIPsolve : status=%d\n",statusS);
        return statusS;
    }

	
    t2 = clock();
	if( large )
		CRPtime2 = (float)(t2-t1)/CLOCKS_PER_SEC;
	else
		CRPtime1 = (float)(t2-t1)/CLOCKS_PER_SEC;

    CRPnodes  = (int)SCIPgetNNodes(CRPenvS);
    mipstatus = SCIPgetStatus(CRPenvS);

    //printf("mipstatusS=%d\n",mipstatusS);
    switch( mipstatus ){

        case SCIP_STATUS_OPTIMAL:
        case SCIP_STATUS_NODELIMIT :
        case SCIP_STATUS_TIMELIMIT:
        case SCIP_STATUS_MEMLIMIT:
        case SCIP_STATUS_USERINTERRUPT :
            break;
        default:
            return 2;
            //printf("mipstatusS=%d\n",mipstatusS);
    }

    sol = SCIPgetBestSol(CRPenvS); 
	if( sol != NULL ){

		xval = (double *)malloc( ncol * sizeof(double) );
		if( xval==NULL ){
			printf("ERROR: not enough memory for save an ILP solution\n");
			return 1;
		}
		for (k = 0; k < ncol; k++)
			xval[k] = SCIPgetSolVal(CRPenvS, sol, SCIPcols[k]);

		for(i=0;i<CRPncell;i++){
			CRPcell[i].solution = xval[i];

			if( large ){
				if( xval[i+CRPncell]>ZERO ){
					//printf("var %s up-up (val=%lf)\n",CRPcell[i].name,xval[i+CRPncell]);
					CRPcell[i].solution += (int)(xval[i+CRPncell]+ZERO);
				}
				if( xval[i+2*CRPncell]>ZERO ){
					//printf("var %s down-down (val=%lf)\n",CRPcell[i].name,xval[i+2*CRPncell]);
					CRPcell[i].solution -= (int)(xval[i+2*CRPncell]+ZERO);
				}
			}
		}
		free(xval);

		CRPobjval = SCIPgetSolOrigObj(CRPenvS, sol);
                CRPobjval += basicOBJvalue;
		//printf(" NEW output pattern with relative objective value %lf\n",CRPobjval);
		CRPfeasible++;
                
                if( mipstatus == SCIP_STATUS_OPTIMAL) CRPlowerbound = CRPobjval;
	}

    SCIPfreeTransform(CRPenvS);
    for (k = 0; k < CRPncell; ++k)
        SCIPreleaseVar(CRPenvS, SCIPcols+k);
    free( SCIPcols );
    for (k = 0; k < CRPnsum; ++k)
        SCIPreleaseCons(CRPenvS, SCIProws+k);
    free( SCIProws );

    return 0;
}








/*******************************************************************/



int S_CRPloadprob(nsums_,rhs_,ncells_,data_,weight_,status_,lpl_,upl_,spl_,lb_,ub_,names_,nlist_,listcell_,listcoef_,base)
int  ncells_,nsums_,base;
int  *nlist_,*listcell_,*weight_;
double *data_,*lpl_,*upl_,*spl_,*lb_,*ub_,*rhs_;
signed char *listcoef_;
char *status_;
char **names_;
{
     int        h,k,l;
     VARIABLE   *col;
 	 CONSTRAINT *row;
     int        sort_prot(const void*,const void*);

     CRPncell = ncells_;
     CRPnsum  = nsums_;
	 CRPbase  = base;

     printf(" %d cells, %d equations, base=%d\n",ncells_,nsums_,base);

    /* allocating memory */

     CRPcell = (VARIABLE *)malloc( CRPncell * sizeof(VARIABLE) );
     if( CRPcell==NULL ){
         return(1);
     }
     CRPsum    = (CONSTRAINT *)malloc( CRPnsum * sizeof(CONSTRAINT) );
     if( CRPsum==NULL ){
         return(1);
     }


     for(k=0;k<CRPncell;k++){
		 if( upl_[k]+lpl_[k] > CRPbase+ZERO )
			 printf("WARNING: variable %d with too long protection levels\n",k);
         col = CRPcell + k;
         col->nominal     = data_[k];
		 col->nominaldown = floor(col->nominal / CRPbase)*CRPbase;
		 col->weight      = weight_[k];
         col->uvalue      = ub_[k];      // ub_[k] - data_[k];
         col->lvalue      = lb_[k];      // data_[k] - lb_[k];
		 col->upl         = upl_[k];
		 col->lpl         = lpl_[k];
		 col->spl         = spl_[k];
		 col->status      = status_[k];
         col->solution    = 0;
		 col->index       = k;
		 l = (int)strlen( names_[k] )+1;
		 if(l==1){
			 printf("Warning: variable %d with no name\n",k);
		 }
         col->name      = (char *)malloc( l*sizeof(char) );
         if( col->name==NULL ){
             printf("ERROR: not memory for names\n");
             return 1;
		 }
         strcpy(col->name,names_[k]);
     }

     for(l=k=0;k<CRPnsum;k++){
         row = CRPsum + k;
         row->card      = h = nlist_[k];
		 row->stack     = (VARIABLE **)malloc( h*sizeof(VARIABLE*) );
		 row->coef      = (double *)malloc( h*sizeof(double) );
		 if( row->stack==NULL || row->coef==NULL )
			 return 1;
		 while(h--){
			 row->stack[h] = CRPcell + listcell_[l];
			 row->coef[h]  = listcoef_[l];
			 l++;
		 }
         row->rhs       = rhs_[k];
         row->sense     = 'E';
		 row->hash      = 0;
     }
     return(0);
}


int S_CRPfreeprob()
{
     int k;

     for(k=0;k<CRPncell;k++)
         free( CRPcell[k].name );
     free((void *)CRPcell);

     for(k=0;k<CRPnsum;k++){
         free( CRPsum[k].stack );
		 free( CRPsum[k].coef );
	 }
     free((void *)CRPsum);
     return 0;
}
 

int S_CRPprintsolution(nsolution,nstatistics)
char *nsolution,*nstatistics;
{
	int k0,k1,k2,k,l,mini,maxi;
	int *count;
	double distance;
	FILE *f,*g;
        g=NULL; // PWOF intialisation

	if( nstatistics ){
		l = 0;
		for(k=0;k<CRPncell;k++)
			if( CRPcell[k].uvalue - CRPcell[k].lvalue < ZERO )
				l++;
		g = fopen( nstatistics , "a" );
		if( g==NULL ){
			printf("file (%s) cannot be opened\n",nsolution);
			return 1;
		}
		fprintf(g,"S %4d %4d %4d  %5.1f %7.1f %5.1f %5.1f %4d  %5d ",CRPncell,l,CRPnsum,CRPbase,CRPtime1,CRPtime2,CRPtime3,CRPnodes,CRPaudit);
	}


	if( !CRPfeasible ){
		printf("        NO OUTPUT PATTERN EXISTS\n");
		if( nstatistics ){
			fprintf(g,"  %3d %3d %3d\n",0,0,0);
			fclose(g);
		}
		return 0;
	}

	l = 0;
	for(k=0;k<CRPncell;k++)
		if( CRPcell[k].solution>ZERO && CRPcell[k].solution<1-ZERO )
			l++;
	fprintf(g,"  %4d ",l);

	if( nsolution ){
		f = fopen( nsolution , "w" );
		if( f==NULL ){
			printf("file (%s) cannot be openned\n",nsolution);
			return 1;
		}
	}
	distance = 0;
	mini = maxi = 0;
	for(k=0;k<CRPncell;k++){
//		if ( fabs( CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution-CRPcell[k].nominal )>ZERO )
			if( nsolution ){
				fprintf(f,"%s from %5.1f to %5.1f\n",CRPcell[k].name,CRPcell[k].nominal,
				CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution);
//				fprintf(f,"%5.0f",CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution);
//				if((k%13)==12) fprintf(f,"\n");
			}

		distance += fabs( CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution-CRPcell[k].nominal );
	}
	if( nsolution ){
		fprintf(f,"distance = %7.1f\n",distance);
		fclose(f);
	}

	if( CRPfeasible==2 ) 
		return 0;

	for(k=0;k<CRPncell;k++){
		l = (int)(CRPcell[k].solution+ZERO);
		if( mini > l ) mini = l;
        if( maxi < l ) maxi = l;
	}

    count = (int*)calloc( maxi-mini+1 , sizeof(int) );
	if( count==NULL ){
		printf("Not enough memory to print\n");
		return 1;
	}
	
	for(k=0;k<CRPncell;k++){
		l = (int)(CRPcell[k].solution+ZERO);
		count[ l-mini ]++;
	}

	printf("SOLUTION in (%s) with distance=%f\n",nsolution,distance);
	if( nstatistics ){
		fprintf(g," %5.1f",distance);
//		for(k=mini;k<=0;k++)
//			fprintf(g," -%d:%d",-k,count[k-mini]);
//		for(k=1;k<=maxi;k++)
//			fprintf(g," +%d:%d",k,count[k-mini]);

		k0 = k1 = k2 = 0;
		for(k=0;k<CRPncell;k++)
			if( CRPcell[k].solution<ZERO )
				k0 ++;
			else if( CRPcell[k].solution<1+ZERO )
				k1 ++;
			else
				k2 ++;
		fprintf(g,"  %3d %3d %3d\n",k0,k1,k2);
		fclose(g);
	}

    for(k=mini;k<=0;k++)
		printf(" [-%d]=%d",-k,count[k-mini]);
    for(k=1;k<=maxi;k++)
		printf(" [+%d]=%d",k,count[k-mini]);
    free(count);
	

	return 0;
}


int S_CRPauditing(type)
int type;
{
	int k;
	double *lb,*ub;
	double val,objval1,objval2,bound;
    clock_t  t1,t2;

	if( type==0 ) return 0;

	if( !CRPfeasible ){
		printf("No solution exists\n");
		return 1;
	}

	if( CRPextended ){
		printf("The current solution is a NON zero-restricted version\n");
		return 1;
	}

    t1 = clock();
	lb = (double *)malloc( CRPncell * sizeof(double) );
	if( lb==NULL ){
		printf("ERROR: not enough memory for auditing LB\n");
		return 1;
	}
	ub = (double *)malloc( CRPncell * sizeof(double) );
	if( ub==NULL ){
		printf("ERROR: not enough memory for auditing UB\n");
		return 1;
	}

	for(k=0;k<CRPncell;k++){
		val   = CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution;
		lb[k] = ( CRPcell[k].lvalue < val-CRPbase ? val-CRPbase : CRPcell[k].lvalue );
		ub[k] = ( CRPcell[k].uvalue > val+CRPbase ? val+CRPbase : CRPcell[k].uvalue );
        //printf("\nuvalue %lf v+base %lf lb %lf ub %lf ", CRPcell[k].uvalue, val+CRPbase, lb[k], ub[k]);
	}
    S_loadsubproblem(lb,ub);

	CRPaudit = 0;
	if( type==1 )
		for(k=0;k<CRPncell;k++)
		if( CRPcell[k].status=='u' ){
			bound = CRPcell[k].nominal+CRPcell[k].upl;
            S_solvesubproblem( 1,k,bound,&objval1,NULL,NULL,NULL);
			if( objval1<bound-ZERO ){
				printf("Invalid UPL in cell k=%d (upl=%lf,obj=%lf)\n",k,bound,objval1);
				CRPaudit++;
			}
			bound = CRPcell[k].nominal-CRPcell[k].lpl;
            S_solvesubproblem(-1,k,bound,&objval2,NULL,NULL,NULL);
			if( objval2>bound+ZERO ){
				printf("Invalid LPL in cell k=%d (lpl=%lf,obj=%lf)\n",k,bound,objval2);
				CRPaudit++;
			}
//			printf("cell=%s from %.1lf to %.1lf [%.1lf,%.1lf]\n",
//				CRPcell[k].name,CRPcell[k].nominal,CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution,objval2,objval1);
		}
		
	
	if( type==2 )
		for(k=0;k<CRPncell;k++)
		if( CRPcell[k].uvalue-CRPcell[k].lvalue > ZERO ){
			bound = CRPcell[k].uvalue;
            S_solvesubproblem( 1,k,bound,&objval1,NULL,NULL,NULL);
			bound = CRPcell[k].lvalue;
            S_solvesubproblem(-1,k,bound,&objval2,NULL,NULL,NULL);
//			printf("cell=%s from %.1lf to %.1lf [%.1lf,%.1lf]\n",
//				CRPcell[k].name,CRPcell[k].nominal,CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution,objval2,objval1);
			if( objval1 - objval2 < CRPbase-ZERO ){
				printf("Warning: cell=%s from %.1lf to %.1lf so the range is [%.1lf,%.1lf]\n",
					CRPcell[k].name,CRPcell[k].nominal,CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution,objval2,objval1);
				CRPaudit++;
			}
		}
		

    S_unloadsubproblem();
    t2 = clock();
	CRPtime3 = (float)(t2-t1)/CLOCKS_PER_SEC;
	
	printf("End of the auditing phase. ");
	if( CRPaudit )  printf(" %d warnings\n",CRPaudit);
	else            printf(" all cells are safe\n");
	free(lb);
	free(ub);
	return 0;
}

/******************************************************  EVENT *****************************************/

//#define EVENTHDLR_NAME         "evendHandler"
//#define EVENTHDLR_DESC         "event handler"
#define EVENTHDLR_NAME         "Total"
#define EVENTHDLR_DESC         "Total event handler"

/** initialization method of event handler (called after problem was transformed) */
static
SCIP_DECL_EVENTINIT(eventInitTotal)
{  /*lint --e{715}*/
   assert(scip != NULL);
   assert(eventhdlr != NULL);
   assert(strcmp(SCIPeventhdlrGetName(eventhdlr), EVENTHDLR_NAME) == 0);

   /* notify SCIP that your event handler wants to react on the event type best LB */
   SCIP_CALL( SCIPcatchEvent( scip, SCIP_EVENTTYPE_VAREVENT | SCIP_EVENTTYPE_NODEEVENT | SCIP_EVENTTYPE_LPEVENT | SCIP_EVENTTYPE_SOLEVENT | SCIP_EVENTTYPE_ROWEVENT, eventhdlr, NULL, NULL) );
   //SCIP_CALL( SCIPcatchEvent( scip, SCIP_EVENTTYPE_VAREVENT | SCIP_EVENTTYPE_NODEEVENT | SCIP_EVENTTYPE_LPEVENT | SCIP_EVENTTYPE_SOLEVENT, eventhdlr, NULL, NULL) );

   return SCIP_OKAY;
}

/** deinitialization method of event handler (called before transformed problem is freed) */
static
SCIP_DECL_EVENTEXIT(eventExitTotal)
{  /*lint --e{715}*/
   assert(scip != NULL);
   assert(eventhdlr != NULL);
   assert(strcmp(SCIPeventhdlrGetName(eventhdlr), EVENTHDLR_NAME) == 0);

   /* notify SCIP that your event handler wants to drop the event type best LB */
   SCIP_CALL( SCIPdropEvent( scip, SCIP_EVENTTYPE_VAREVENT | SCIP_EVENTTYPE_NODEEVENT | SCIP_EVENTTYPE_LPEVENT | SCIP_EVENTTYPE_SOLEVENT | SCIP_EVENTTYPE_ROWEVENT, eventhdlr, NULL, -1) );
   //SCIP_CALL( SCIPdropEvent( scip, SCIP_EVENTTYPE_VAREVENT | SCIP_EVENTTYPE_NODEEVENT | SCIP_EVENTTYPE_LPEVENT | SCIP_EVENTTYPE_SOLEVENT, eventhdlr, NULL, -1) );

   return SCIP_OKAY;
}

/** execution method of event handler */
static
SCIP_DECL_EVENTEXEC(eventExecTotal)
{  /*lint --e{715}*/

    double bestUB,bestLB;
    //int    nodecount,nodeleft,statusS;
    int    nodecount,nodeleft;
    double res;

   if (CRPabort) return SCIP_OKAY;
    
   assert(eventhdlr != NULL);
   assert(strcmp(SCIPeventhdlrGetName(eventhdlr), EVENTHDLR_NAME) == 0);
   assert(event != NULL);
   assert(scip != NULL);
   
   /*assert((SCIPeventGetType(event) == SCIP_EVENTTYPE_VAREVENT) || (SCIPeventGetType(event) == SCIP_EVENTTYPE_NODEEVENT) ||
           (SCIPeventGetType(event) == SCIP_EVENTTYPE_LPEVENT) || (SCIPeventGetType(event) == SCIP_EVENTTYPE_SOLEVENT) ||
           (SCIPeventGetType(event) == SCIP_EVENTTYPE_ROWEVENT));
   */
   //assert(SCIPeventGetType(event) == SCIP_EVENTTYPE_VAREVENT | SCIP_EVENTTYPE_NODEEVENT | SCIP_EVENTTYPE_LPEVENT | SCIP_EVENTTYPE_SOLEVENT | SCIP_EVENTTYPE_ROWEVENT);
   //assert(SCIPeventGetType(event) == SCIP_EVENTTYPE_VAREVENT | SCIP_EVENTTYPE_NODEEVENT | SCIP_EVENTTYPE_LPEVENT | SCIP_EVENTTYPE_SOLEVENT);
//   assert(SCIPeventGetType(event) == SCIP_EVENTTYPE_VAREVENT );
//   assert(SCIPeventGetType(event) == SCIP_EVENTTYPE_NODEEVENT );
//   assert(SCIPeventGetType(event) == SCIP_EVENTTYPE_LPEVENT );
//   assert(SCIPeventGetType(event) == SCIP_EVENTTYPE_SOLEVENT );
//   assert(SCIPeventGetType(event) == SCIP_EVENTTYPE_ROWEVENT );

   //printf("SCIPeventGetType(event) = %12d (%#010x)\n", SCIPeventGetType(event), SCIPeventGetType(event));
   //fflush(stdout);
   //if ((SCIPgetStage(scip) == SCIP_STAGE_PRESOLVING) || (SCIPgetStage(scip) == SCIP_STAGE_SOLVING)) {   
   if (SCIPgetStage(scip) == SCIP_STAGE_SOLVING) {   
        SCIPdebugMessage("exec method of event handler\n");
        CRPlowerbound = basicOBJvalue + SCIPgetLowerbound(scip);
        bestLB = CRPlowerbound;
        CRPobjval = basicOBJvalue + SCIPgetUpperbound(scip);
        bestUB = CRPobjval;

        nodecount = (int)SCIPgetNTotalNodes(scip);
        nodeleft  = SCIPgetNNodesLeft(scip);

        CRPmessage(bestLB, bestUB, nodecount, nodeleft);
        if ((SCIPeventGetType(event) == SCIP_EVENTTYPE_BESTSOLFOUND) || (SCIPeventGetType(event) == SCIP_EVENTTYPE_LPSOLVED)){
            TotalTime = (double)clock()/CLOCKS_PER_SEC - InitialTime;
            // printf("UB=%g  LB=%.1f  iterations=%d  time=%.1f s\n", bestUB,bestLB,nodecount,TotalTime);
            if( TotalTime > MaximumTime ){
                //printf(" TIME LIMIT of %.1lf sec achieved\n",TotalTime);
                //printf("MaximumTime is %.1lf sec\n",MaximumTime);
                //printf("Best LB = %.2lf\n",bestLB);
                //printf("Best UB = %.2lf\n",bestUB);
                //printf("Nodes   explored = %d\n",nodecount);
                //printf("Nodes unexplored = %d\n",nodeleft);
                res = CRPextratime();
                if( res>0 )
                {
                    MaximumTime = (double)clock()/CLOCKS_PER_SEC - InitialTime + (res*60.0);
                    //printf("From JJ: New MaximumTime = %lf\n",MaximumTime);               
                }
                else
                {
                    CRPabort = 1;
                }
                //statusS = SCIPsetRealParam(CRPenvS, "limits/time", MaximumTime);
                SCIPsetRealParam(CRPenvS, "limits/time", MaximumTime);
            }
        }
   }
   return SCIP_OKAY;
}

/** includes event handler for best LB */
SCIP_RETCODE SCIPincludeEventHdlrTotal(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_EVENTHDLRDATA* eventhdlrdata;
   SCIP_EVENTHDLR* eventhdlr;
   eventhdlrdata = NULL;

   eventhdlr = NULL;
   /* create event handler for events on watched variables */
   SCIP_CALL( SCIPincludeEventhdlrBasic(scip, &eventhdlr, EVENTHDLR_NAME, EVENTHDLR_DESC, eventExecTotal, eventhdlrdata) );
   assert(eventhdlr != NULL);
   SCIP_CALL( SCIPsetEventhdlrInit(scip, eventhdlr, eventInitTotal) );
   SCIP_CALL( SCIPsetEventhdlrExit(scip, eventhdlr, eventExitTotal) );

   return SCIP_OKAY;
}



#endif

