/****************************************************/
/* Controlled Rounding Problem for Linked Tables    */
/* in Statistical Disclosure Control (SDC)          */
/*                                                  */
/*                                                  */
/* CRPMAIN.c  (HiGHS backend)                       */
/* Ported from the modern-SCIP version to the       */
/* HiGHS C API.  The controlled-rounding MIP is     */
/* assembled once in CSR form and passed with a     */
/* single Highs_passMip call; the event handler     */
/* (which only reported progress / extended the     */
/* time limit) is dropped because the HiGHS C API   */
/* has no equivalent callback hook.                 */
/*                                                  */
/****************************************************/
#ifdef SCIPV
//#define PERTURBATION
#define ROUNDING

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

#include <highs/interfaces/highs_c_api.h>
#include "crpmain.h"

/* crpglob.h only declares these when __WRAPCRP_H is set; declare them here
   with C linkage (they are defined in WrapCRP.c, compiled as C). */
#ifdef __cplusplus
extern "C" {
extern double ZERO;
extern double INF;
extern double MAX_TIME;
}
#endif

/* The HiGHS C API has no public infinity constant; 1e20 is the value
   HiGHS itself treats as "infinite" (same convention as csp/Jjsolver.c). */
#define CRP_HIGHS_INF  1.0e20


void*           CRPenvS;          /* HiGHS instance (main MIP)            */
void*           CRPsubproblemS;   /* HiGHS instance (audit subproblem LP) */

/* Highs_clear() resets all options to defaults (re-enabling the console log),
   so the quiet options must be (re)applied after every Highs_clear. */
static void S_CRP_set_silent(void* H)
{
    if( H == NULL ) return;
    Highs_setBoolOptionValue(H, "output_flag", 0);
    Highs_setBoolOptionValue(H, "log_to_console", 0);
}



static int S_MIPmodel(int);
//static int S_unMIPsolver(void);
//static int S_imcompatibility(int,char*,double*,int*,int*,int*,int*,double*);

extern int S_loadsubproblem(double*,double*);
extern int S_unloadsubproblem(void);
extern int S_solvesubproblem(int,int,double,double*,double*,double*,double*);



/*  FUNCTIONS  */

int S_CRPopenSOLVER()
{
    CRPenvS = Highs_create();
    if( CRPenvS == NULL ){
        fprintf (stderr, "Could not open HiGHS instance.\n");
        return -1;
    }
    /* NOTE: Highs_clear() resets options to defaults, so these are (re)applied
       after each Highs_clear in S_MIPmodel via S_CRP_set_silent(). */
    printf("     using HiGHS\n\n");

    return 0;
}

int S_CRPcloseSOLVER()
{
    if( CRPenvS != NULL ){
        Highs_destroy(CRPenvS);
        CRPenvS = NULL;
    }
    return 0;
}




int  S_CRPoptimize(double *lowerbound,double *upperbound)
{
    int status,k;

    CRPfeasible = 0;
    CRPobjval   = 0;
    CRPextended = 0;
    CRPabort       = 0;
    CRPlowerbound  = CRP_HIGHS_INF;
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

static int S_MIPmodel(int large)
{
    int        i,k,h,lb,ub,tb;
    int        ncol,newnz;
    int        card,position,index;
    double     coef,penalty;
    double     *obj,*dlb,*dub,*matval,*col_value,*rhsS,*lhsS;
    char       *ctype;
    HighsInt   *row_start,*row_index,*integrality;
    clock_t    t1,t2;
    HighsInt   statusS,ms;

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

        coef = (CRPcell[k].uvalue - CRPcell[k].nominaldown )/CRPbase;
        if( coef < dub[k] )
            dub[k] = coef;
        coef = (CRPcell[k].nominal - CRPcell[k].lpl + CRPbase - CRPcell[k].nominaldown )/CRPbase;
        if( coef < dub[k] )
            dub[k] = coef;

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


    /* Integality: 'B' (binary) and 'I' (integer) are integer in HiGHS;
       'C' is continuous.  A binary var is an integer with bounds [0,1]. */
    integrality = (HighsInt *)malloc( ncol * sizeof(HighsInt) );
    if( integrality==NULL ){
        printf("ERROR: not enough memory for integrality\n");
        return 1;
    }
    for(k=0;k<ncol;k++)
        integrality[k] = (ctype[k]=='C') ? 0 : 1;


    /************  adding the basic constraints, i.e., the structure of the table ***************/

    newnz = 0;
    for(k=0;k<CRPnsum;k++)
        newnz += CRPsum[k].card * (large ? 3 : 1);

    row_start = (HighsInt *)malloc( (CRPnsum+1) * sizeof(HighsInt) );
    if( row_start==NULL ){
        printf("ERROR: not enough memory for MSTART\n");
        return 1;
    }
    row_index = (HighsInt *)malloc( (newnz>0?newnz:1) * sizeof(HighsInt) );
    if( row_index==NULL ){
        printf("ERROR: not enough memory for MCLIND\n");
        return 1;
    }
    matval = (double *)malloc( (newnz>0?newnz:1) * sizeof(double) );
    if( matval==NULL ){
        printf("ERROR: not enough memory for DMATVAL\n");
        return 1;
    }
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

    for(k=0;k<CRPnsum;k++){
        switch( CRPsum[k].sense )
        {
            case 'L':
                    rhsS[k] = CRPsum[k].rhs;
                    h = CRPsum[k].card;
                    while(h--){
                        rhsS[k] -= CRPsum[k].coef[h] * CRPsum[k].stack[h]->nominaldown;
                    }
                    rhsS[k] = rhsS[k] / CRPbase;
                    lhsS[k] = -CRP_HIGHS_INF;
                    break;
            case 'E':
                    rhsS[k] = CRPsum[k].rhs;
                    h = CRPsum[k].card;
                    while(h--){
                        rhsS[k] -= CRPsum[k].coef[h] * CRPsum[k].stack[h]->nominaldown;
                    }
                    rhsS[k] = rhsS[k] / CRPbase;
                    lhsS[k] = rhsS[k];
                    break;
            case 'G':
                    lhsS[k] = CRPsum[k].rhs;
                    h = CRPsum[k].card;
                    while(h--){
                        lhsS[k] -= CRPsum[k].coef[h] * CRPsum[k].stack[h]->nominaldown;
                    }
                    lhsS[k] = lhsS[k] / CRPbase;
                    rhsS[k] = CRP_HIGHS_INF;
                    break;
            default:
                    printf("ERROR: invalid row sense\n");
        }
    }

    /* Constraint matrix in row-major (CSR) form. */
    position = 0;
    row_start[0] = 0;
    for(k=0;k<CRPnsum;k++){
        card = CRPsum[k].card;
        while(card--){
            index = CRPsum[k].stack[card]->index;
            coef  = CRPsum[k].coef[card];
            row_index[position] = index;
            matval[position]    = coef;
            position++;
            if( large ){
                row_index[position] = CRPncell+index;
                matval[position]    = coef;
                position++;
                row_index[position] = 2*CRPncell+index;
                matval[position]    = -coef;
                position++;
            }
        }
        row_start[k+1] = position;
    }

    if( position != newnz ){
        printf("ERROR: position=%d   newnz=%d\n",position,newnz);
        return 1;
    }


    /******************  loading the MIP into HiGHS  *******/

    Highs_clear(CRPenvS);
    S_CRP_set_silent(CRPenvS);
    statusS = Highs_passMip(CRPenvS,
        (HighsInt)ncol, (HighsInt)CRPnsum, (HighsInt)newnz,
        kHighsMatrixFormatRowwise, kHighsObjSenseMinimize, 0.0,
        obj, dlb, dub, lhsS, rhsS,
        row_start, row_index, matval, integrality);
    if( statusS != kHighsStatusOk ){
        printf("ERROR (status=%d): problems loading HiGHS-solver\n",(int)statusS);
        return 9;
    }

    free( ctype );
    free( dub );
    free( dlb );
    free( obj );
    free( matval );
    free( row_index );
    free( row_start );
    free( integrality );
    free( rhsS );
    free( lhsS );


    /******************  solving the MIP  *******/

    statusS = Highs_run(CRPenvS);
    if( statusS != kHighsStatusOk ){
        printf("Warning: bad result from HiGHS_run : status=%d\n",(int)statusS);
        return 1;
    }

    t2 = clock();
    if( large )
        CRPtime2 = (float)(t2-t1)/CLOCKS_PER_SEC;
    else
        CRPtime1 = (float)(t2-t1)/CLOCKS_PER_SEC;

    /* Node count is best-effort: HiGHS may not expose "mip_nodes" via
       the C info API on every build; fall back to 0 if unavailable. */
    {
        int64_t nodes = 0;
        if( Highs_getInt64InfoValue(CRPenvS, "mip_nodes", &nodes) == kHighsStatusOk )
            CRPnodes = (int)nodes;
        else
            CRPnodes = 0;
    }
    ms = Highs_getModelStatus(CRPenvS);

    switch( ms ){
        case kHighsModelStatusOptimal:
        case kHighsModelStatusTimeLimit:
        case kHighsModelStatusIterationLimit:
        case kHighsModelStatusObjectiveBound:
        case kHighsModelStatusObjectiveTarget:
            break;
        default:
            return 2;   /* e.g. infeasible -> try the extended model */
    }

    {
        int i;
        col_value = (double *)malloc( ncol * sizeof(double) );
        if( col_value==NULL ){
            printf("ERROR: not enough memory to save an MIP solution\n");
            return 1;
        }
        Highs_getSolution(CRPenvS, col_value, NULL, NULL, NULL);

        for(i=0;i<CRPncell;i++){
            CRPcell[i].solution = col_value[i];

            if( large ){
                if( col_value[i+CRPncell]>ZERO ){
                    CRPcell[i].solution += (int)(col_value[i+CRPncell]+ZERO);
                }
                if( col_value[i+2*CRPncell]>ZERO ){
                    CRPcell[i].solution -= (int)(col_value[i+2*CRPncell]+ZERO);
                }
            }
        }
        free(col_value);

        CRPobjval = Highs_getObjectiveValue(CRPenvS) + basicOBJvalue;
        CRPfeasible++;

        if( ms == kHighsModelStatusOptimal ) CRPlowerbound = CRPobjval;
    }

    return 0;
}



int S_CRPloadprob(int nsums_,double *rhs_,int ncells_,double *data_,int *weight_,
                  char *status_,double *lpl_,double *upl_,double *spl_,
                  double *lb_,double *ub_,char **names_,int *nlist_,
                  int *listcell_,signed char *listcoef_,int base)
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
         col->uvalue      = ub_[k];
         col->lvalue      = lb_[k];
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


int S_CRPprintsolution(char *nsolution,char *nstatistics)
{
    int k0,k1,k2,k,l,mini,maxi;
    int *count;
    double distance;
    FILE *f,*g;
    g=NULL;

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
        if( nsolution ){
            fprintf(f,"%s from %5.1f to %5.1f\n",CRPcell[k].name,CRPcell[k].nominal,
            CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution);
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


int S_CRPauditing(int type)
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
        }



    if( type==2 )
        for(k=0;k<CRPncell;k++)
        if( CRPcell[k].uvalue-CRPcell[k].lvalue > ZERO ){
            bound = CRPcell[k].uvalue;
            S_solvesubproblem( 1,k,bound,&objval1,NULL,NULL,NULL);
            bound = CRPcell[k].lvalue;
            S_solvesubproblem(-1,k,bound,&objval2,NULL,NULL,NULL);
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



#endif
