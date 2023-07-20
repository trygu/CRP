/****************************************************/
/* Controlled Rounding Problem for Linked Tables    */
/* in Statistical Disclosure Control (SDC)          */
/*                                                  */
/*                                                  */
/* WrapCRP.h                                        */
/* Version 3.0.0                                    */
/* 20 May 2014                  jjsalaza@ull.es     */
/*                                                  */
/*                                                  */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
//#include <direct.h>
#include "WrapCRP.h"
#include "crpglob.h"

char *fsolution;
char *fstatistic;

void CRPsetfunctions( void (*f)(double,double,int,int) , int (*g)(void) )
{
	CRPmessage   = f;
	CRPextratime = g;
}

void CRPSetFileNames(const char *Solver, const char *dir)
{
    fsolution = (char*) malloc((strlen(dir) + 15)*sizeof(char));
    fstatistic = (char*) malloc((strlen(dir) + 15)*sizeof(char));
    strcpy(fsolution,dir);
    strcpy(fstatistic,dir);
#ifdef CPLEXV
    if (strcmp(Solver,"CPLEX")==0) {
       strcat(fsolution,"crpCPLEX.sol");
       strcat(fstatistic,"crpCPLEX.stat");
    }
#endif
#ifdef SCIPV
    if (strcmp(Solver,"SCIP")==0) {
       strcat(fsolution,"crpSCIP.sol");
       strcat(fstatistic,"crpSCIP.stat");
    }
#endif
#ifdef XPRESSV
    if (strcmp(Solver,"XPRESS")==0) {
       strcat(fsolution,"crpXPRESS.sol");
       strcat(fstatistic,"crpXPRESS.stat");
    }
#endif
}

void CRPFreeFileNames()
{
    free(fsolution);
    free(fstatistic);

}


//-------------------------------------------------

void   CRPSetDoubleConstant(const int VarNumber, double VarValue)
{
    switch (VarNumber){
        case JJZERO:
            ZERO = VarValue;
            break;
        case JJINF:
            INF = VarValue;
            break;
        case JJMAXTIME:
            MAX_TIME = VarValue;
            break;
        case JJMINVIOLA:
            MIN_VIOLA = VarValue;
            break;
        case JJMAXSLACK:
            MAX_SLACK = VarValue;
            break;
        default:
            printf("\nUnknown constant %d\n", VarNumber);
            break;
    }

}

double CRPGetDoubleConstant(const int VarNumber)
{
    switch (VarNumber){
        case JJZERO:
            return ZERO;
        case JJINF:
            return INF;
        case JJMAXTIME:
            return MAX_TIME;
        case JJMINVIOLA:
            return MIN_VIOLA;
        case JJMAXSLACK:
            return MAX_SLACK;
        default:
            printf("\nUnknown constant %d\n",VarNumber);
            return -9;
    }
}


//-------------------------------------------------

int    CRPoptimize(const char *Solver, double *lb, double *ub)
{
    int retval = -1;
#ifdef CPLEXV
    if (strcmp(Solver,"CPLEX")==0)  retval = C_CRPoptimize(lb, ub);
#endif
#ifdef SCIPV
    if (strcmp(Solver,"SCIP")==0)   retval = S_CRPoptimize(lb, ub);
#endif
#ifdef XPRESSV
    if (strcmp(Solver,"XPRESS")==0) retval = X_CRPoptimize(lb, ub);
#endif
    return retval;
}

int    CRPloadprob(const char *Solver, int nsums_, double *rhs_, int ncells_, double *data_, int *weight_, char *status_, 
                    double *lpl_, double *upl_, double *spl_, double *lb_, double *ub_, char **names_, int *nlist_, int *listcell_,
                    signed char *listcoef_, int base)
{
    int retval = -1;
#ifdef CPLEXV
    if (strcmp(Solver,"CPLEX")==0) 
        retval = C_CRPloadprob(nsums_,rhs_,ncells_,data_,weight_,status_,lpl_,upl_,spl_,lb_,ub_,names_,nlist_,listcell_,listcoef_,base);
#endif
#ifdef SCIPV
    if (strcmp(Solver,"SCIP")==0)
        retval = S_CRPloadprob(nsums_,rhs_,ncells_,data_,weight_,status_,lpl_,upl_,spl_,lb_,ub_,names_,nlist_,listcell_,listcoef_,base);
#endif
#ifdef XPRESSV
    if (strcmp(Solver,"XPRESS")==0)
        retval = X_CRPloadprob(nsums_,rhs_,ncells_,data_,weight_,status_,lpl_,upl_,spl_,lb_,ub_,names_,nlist_,listcell_,listcoef_,base);
#endif
    return retval;
}

int    CRPfreeprob(const char *Solver)
{
    int retval = -1;
#ifdef CPLEXV
    if (strcmp(Solver,"CPLEX")==0)  retval = C_CRPfreeprob();
#endif
#ifdef SCIPV
    if (strcmp(Solver,"SCIP")==0)   retval = S_CRPfreeprob();
#endif
#ifdef XPRESSV
    if (strcmp(Solver,"XPRESS")==0) retval = X_CRPfreeprob();
#endif
    return retval;
}


int    CRPauditing(const char *Solver, int type)
{
    int retval = -1;
#ifdef CPLEXV
    if (strcmp(Solver,"CPLEX")==0)  retval = C_CRPauditing(type);
#endif
#ifdef SCIPV
    if (strcmp(Solver,"SCIP")==0)   retval = S_CRPauditing(type);
#endif
#ifdef XPRESSV
    if (strcmp(Solver,"XPRESS")==0) retval = X_CRPauditing(type);
#endif
    return retval;
}

int    CRPopenSOLVER(const char *Solver, const char *LicenseFile)
{
    int retval = -1;
#ifdef CPLEXV
    if (strcmp(Solver,"CPLEX")==0)  retval = C_CRPopenSOLVER(LicenseFile);
#endif
#ifdef SCIPV
    if (strcmp(Solver,"SCIP")==0)   retval = S_CRPopenSOLVER();
#endif
#ifdef XPRESSV
    if (strcmp(Solver,"XPRESS")==0) retval = X_CRPopenSOLVER();
#endif
    return retval;
}

int    CRPcloseSOLVER(const char *Solver)
{
    int retval = -1;
#ifdef CPLEXV
    if (strcmp(Solver,"CPLEX")==0)  retval = C_CRPcloseSOLVER();
#endif
#ifdef SCIPV
    if (strcmp(Solver,"SCIP")==0)   retval = S_CRPcloseSOLVER();
#endif
#ifdef XPRESSV
    if (strcmp(Solver,"XPRESS")==0) retval = X_CRPcloseSOLVER();
#endif
    return retval;
}





int CRPprintsolution(char *nsolution, char *nstatistics, double *MaxJump, int *NumberJump)
{
	int k,l,mini,maxi,fixed,unfixed,numberjumps;
	int *count;
	double distance,maxdistance,d;
	FILE   *f;

	if( !CRPfeasible ){
	printf("        NO OUTPUT PATTERN EXISTS\n");
        *NumberJump = 0;
        *MaxJump    = 0;
	return 0;
	}

	l = 0;
	for(k=0;k<CRPncell;k++)
		if( CRPcell[k].solution>ZERO && CRPcell[k].solution<1-ZERO )
			l++;

	if( nsolution ){
		f = fopen( nsolution , "w" );
		if( f==NULL ){
			printf("file (%s) cannot be opened\n",nsolution);
			return 1;
		}
	}
        numberjumps = 0;
	distance = maxdistance = 0;
	mini = maxi = 0;
	for(k=0;k<CRPncell;k++){
//		if ( fabs( CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution-CRPcell[k].nominal )>ZERO )
//   also print the fixed cells
			if( nsolution ){
				fprintf(f,"%3d from %5.1f to %5.1f\n",k,CRPcell[k].nominal,
				CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution);
//				fprintf(f,"%5.0f",CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution);
//				if((k%13)==12) fprintf(f,"\n");
			}

        d = fabs( CRPcell[k].nominaldown+CRPbase*CRPcell[k].solution-CRPcell[k].nominal );
		distance += d;
        if( maxdistance < d )
            maxdistance = d;
        if( d>CRPbase-ZERO )
            numberjumps ++;
	}
        *NumberJump = numberjumps;
        *MaxJump    = maxdistance;
	if( nsolution ){
		fprintf(f,"SUMdistance = %.1f    MAXdistance = %.1f  NumberOfJumps=%d\n",distance,maxdistance,numberjumps);
		fclose(f);
	}


	if( CRPfeasible==2 ) 
		return 0;

	for(k=0;k<CRPncell;k++){
		l = (int)floor(CRPcell[k].solution+ZERO);
		if( mini > l ) mini = l;
        if( maxi < l ) maxi = l;
	}

	count = (int *)calloc( maxi-mini+1 , sizeof(int) );
	if( count==NULL ){
		printf("Not enough memory to print\n");
		return 1;
	}
	
	for(k=0;k<CRPncell;k++){
		l = (int)floor(CRPcell[k].solution+ZERO);
		count[ l-mini ]++;
	}

	fixed = unfixed = 0;
	for(k=0;k<CRPncell;k++)
		if ( fabs( CRPcell[k].nominal - CRPcell[k].nominaldown ) < ZERO ){
			if( CRPcell[k].solution>ZERO )
				unfixed++;
			else
				fixed++;
		}

	printf("SOLUTION in <%s>\n with distance=%f:  unmodified multiples=%d  modified multiples=%d\n",nsolution,distance,fixed,unfixed);

        for(k=mini;k<=0;k++){
		printf(" [-%d]=%d",-k,count[k-mini]);
	}
        for(k=1;k<=maxi;k++){
		printf(" [+%d]=%d",k,count[k-mini]);
	}
	printf("\n");
        free(count);
	
	return 0;
}






/*******************************************************************/


#define MARGINALS

static int  load_crp0old(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                         char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names);
static int  load_crp0(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                         char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names);
static int  load_crp2(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                      char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names);
static int  load_crp3(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                      char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names);
static int  load_crp4(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                      char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names);
static int  load_crp4simple(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                            char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names);

#ifndef MARGINALS
static double adding3d(int N1, int N2, int N3, double *data, int i, int j, int k);
static double adding3i(int N1, int N2, int N3, int *data, int i, int j, int k);

static int  adding2d(int N1, int N2 ,double *data, int i, int j);
static int  adding2i(int N1, int N2 ,int *data, int i, int j);
#endif

static void free_crp(int ncells, double *rhs, int *ncard, int *list, signed char *val, double *data, int *weight, char *status, double *lpl, double *upl, double *spl,
                     double *lb, double *ub, char **names);
static char *CellName(int i, int j, int k, int s);
static int cellindex( char *nombre , int ncells , char **Tnombre );

int do_round(char *Solver, char *InFileName, double Base, double *UpperBound, double *LowerBound, long Auditing, 
	     char *SolutionFile, char *StatisticsFile, char *LicenseFile, char *LogFile,
             long MaxTime, long ZeroRestricted, char *NamePathExe,
             double *MaxJump, long *NumberJump, double *UsedTime, long *ErrorCode)
{
    int    ncells,nsums,i,j=0;
    int    *ncard,*list,*weight;
    double *data,*lpl,*upl,*spl,*lb,*ub,*rhs;
    char   *status;
    char   **names;
    signed char *val;
    clock_t  t1,t2,t3,t4;
    int      dim,base,auditing;
    double   ttime1,ttime2,d=0.0;
    struct tm   *newtime;
    time_t      aclock;
    FILE        *file,*outputfile;
    outputfile=NULL; // PWOF: intialisation

    base     = (int)Base;
    auditing = Auditing;

    if( LogFile ){
        outputfile = freopen( LogFile, "w", stdout );
        if( outputfile == NULL ){
            *ErrorCode = NOOUTPUTFILE;
	    return 1;
	}
        outputfile = freopen( LogFile, "a", stderr );
        if( outputfile == NULL ){
            *ErrorCode = NOOUTPUTFILE;
            return 1;
        }
    }

    time( &aclock );                 /* Get time in seconds */
    newtime = localtime( &aclock );  /* Convert time to struct */
    printf( "Controlled Rounding Solver on %s", asctime( newtime ) );

    //CRPSetFileNames(Solver,"");
    fsolution = (char*) malloc((strlen(SolutionFile) + 15)*sizeof(char));
    fstatistic = (char*) malloc((strlen(StatisticsFile) + 15)*sizeof(char));
    strcpy(fsolution,SolutionFile);
    strcpy(fstatistic,StatisticsFile);

    //CRPSetDoubleConstant(JJZERO,1.0E-5);
    //CRPSetDoubleConstant(JJINF,1000000000);
    CRPSetDoubleConstant(JJMAXTIME,MaxTime*60.0);  // in seconds
    //CRPSetDoubleConstant(JJMINVIOLA,0.0001);
    //CRPSetDoubleConstant(JJMAXSLACK,0.01);

    if( CRPopenSOLVER(Solver, LicenseFile) ){
        printf("Problems with the SOLVER license\n");
	if( LogFile ) fclose( outputfile );
        *ErrorCode = NOLICENSE;
	return 1;
    }

        
    file = fopen(InFileName,"r");
    if( file==NULL ){
        printf("ERROR: it is not possible to open file %s\n",InFileName);
        return(1);
    }
    fscanf(file,"%d\n",&dim);
	
    switch( dim ){
        case 0 :
             if( load_crp0old(file,&ncells,&nsums,&rhs,&ncard,&list,&val,
                 &data,&weight,&status,&lpl,&upl,&spl,&lb,&ub,&names) ){
                printf(" ERROR: loading CRP instance\n");
		if( LogFile ) fclose( outputfile );
		*ErrorCode = NOJJFORMAT;
                return(1);
             }
             break;
        case -1 :
             if( load_crp0(file,&ncells,&nsums,&rhs,&ncard,&list,&val,
                 &data,&weight,&status,&lpl,&upl,&spl,&lb,&ub,&names) ){
                printf(" ERROR: loading CRP instance\n");
		if( LogFile ) fclose( outputfile );
		*ErrorCode = NOJJFORMAT;
                return(1);
             }
             break;
        case 2 :
             if( load_crp2(file,&ncells,&nsums,&rhs,&ncard,&list,&val,
                 &data,&weight,&status,&lpl,&upl,&spl,&lb,&ub,&names) ){
                printf(" ERROR: loading CRP instance\n");
		if( LogFile ) fclose( outputfile );
		*ErrorCode = NOJJFORMAT;
                return(1);
             }
             break;
        case 3 :
             if( load_crp3(file,&ncells,&nsums,&rhs,&ncard,&list,&val,
                 &data,&weight,&status,&lpl,&upl,&spl,&lb,&ub,&names) ){
                printf(" ERROR: loading CRP instance\n");
		if( LogFile ) fclose( outputfile );
		*ErrorCode = NOJJFORMAT;
                return(1);
             }
             break;
        case 4 :
             if( load_crp4(file,&ncells,&nsums,&rhs,&ncard,&list,&val,
                 &data,&weight,&status,&lpl,&upl,&spl,&lb,&ub,&names) ){
                printf(" ERROR: loading CRP instance\n");
		if( LogFile ) fclose( outputfile );
		*ErrorCode = NOJJFORMAT;
                return(1);
             }
             break;
        case -4 :
             if( load_crp4simple(file,&ncells,&nsums,&rhs,&ncard,&list,&val,
                 &data,&weight,&status,&lpl,&upl,&spl,&lb,&ub,&names) ){
                printf(" ERROR: loading CRP instance\n");
		if( LogFile ) fclose( outputfile );
		*ErrorCode = NOJJFORMAT;
                return(1);
             }
             break;
        default:
             printf("ERROR: unknown type of input file\n");
             exit(1);
    }
    fclose(file);


    i = CRPloadprob(Solver,nsums,rhs,ncells,data,weight,status,lpl,upl,spl,lb,ub,names,ncard,list,val,base);
	free_crp(ncells,rhs,ncard,list,val,data,weight,status,lpl,upl,spl,lb,ub,names);
    if(i){
        printf(" ERROR: loading the CRP instance\n");
	return(1);
    }

    *UpperBound = 10000000000000;
    *LowerBound = 0;

    t1 = clock();
    i = CRPoptimize(Solver,LowerBound,UpperBound);
    t2 = clock();
    ttime1 = (float)(t2-t1)/CLOCKS_PER_SEC;

    if(i<0) return 0;

    t3 = clock();
    i = CRPauditing(Solver,auditing);
    t4 = clock();
	ttime2 = (float)(t4-t3)/CLOCKS_PER_SEC;
    if(i) {
        printf(" ERROR: auditing\n");
        CRPfreeprob(Solver);
        CRPcloseSOLVER(Solver);
	if( LogFile ) fclose( outputfile );
        *ErrorCode = NOAUDITING;
        return 1;
    }

    i = CRPprintsolution(fsolution,fstatistic,&d,&j);
    *NumberJump = j;
    *MaxJump    = d;
    if(i) {
        printf(" ERROR: printing solution\n");
        CRPfreeprob(Solver);
        CRPcloseSOLVER(Solver);
	if( LogFile ) fclose( outputfile );
        *ErrorCode = NOPRINTING;
	return 1;
    }

    i = CRPfreeprob(Solver);
    if(i){
        printf(" ERROR: unloading the CRP instance\n");
        CRPcloseSOLVER(Solver);
	if( LogFile ) fclose( outputfile );
        *ErrorCode = NOFREEPROBLEM;
        return 1;
    }

    printf(" opt-Time = %.2f ; audit-time = %.2f ; UB = %.1f; LB = %.1f\n",ttime1,ttime2,*UpperBound,*LowerBound);

    if( CRPcloseSOLVER(Solver) )   {
        printf("Problems with the SOLVER license\n"); 
	if( LogFile ) fclose( outputfile );
        *ErrorCode = NOCLOSELICENSE;
        return 1;
    }   
    CRPFreeFileNames();

    if( LogFile ) fclose( outputfile );

    return(0);
}

/*******************************************************************/

static int  load_crp0old(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                         char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names)
{
    int  j,k,l,dim,w,d1,d2,maxsize;
    double d,lbd,ubd,up,lp,sp;
    int  ncells,nsums,*ncard,*list,*weight;
    double *data,*lpl,*upl,*spl,*lb,*ub,*rhs;
    char *status;
    signed char *val;
    char **names;
    char s;
//	char username[50];


    fscanf(file,"%d\n",&ncells);
    if( ncells<2 ){
        printf("ERROR: bad input file\n");
        exit(1);
    }
    dim = ncells;
    data  = (double *)malloc( dim*sizeof(double) );
    weight= (int *)malloc( dim*sizeof(int) );
    status= (char *)malloc( dim*sizeof(char) );
    lpl   = (double *)malloc( dim*sizeof(double) );
    upl   = (double *)malloc( dim*sizeof(double) );
    spl   = (double *)malloc( dim*sizeof(double) );
    lb    = (double *)malloc( dim*sizeof(double) );
    ub    = (double *)malloc( dim*sizeof(double) );
    names = (char **)malloc( dim*sizeof(char *) );
    if( !data || !weight || !status || !lpl || !upl || !spl || !lb || !ub || !names ){
        printf("ERROR: Not enough memory for READING\n");
        return(1);
    }
    

    for(l=0;l<ncells;l++){
//        if( fscanf(file,"%d %s %lf %d %c %lf %lf %lf %lf %lf\n",&k,username,&d,&w,&s,&lbd,&ubd,&lp,&up,&sp)<0 ) break;
        if( fscanf(file,"%d %lf %d %c %lf %lf %lf %lf %lf\n",&k,&d,&w,&s,&lbd,&ubd,&lp,&up,&sp)<0 ) break;
        if( l != k ){
            printf("ERROR: l=%d k=%d\n",l,k);
            return(1);
        }
        data[l] = d;
        weight[l] = w;
        status[l] = s;
//        lpl[l] = d-lp;
//        upl[l] = up-d;
        lpl[l] = lp;
        upl[l] = up;
	spl[l] = sp;
        lb[l]  = lbd;
        ub[l]  = ubd;
        names[l] = CellName(k,-1,-1,-1);
//      names[l] = (char *)malloc( strlen(username)+1 );
//		strcpy( names[l] , username );
        
        // to deal with negative cell values as well:
        // Note that lp >=0
        // if( lp>d ){ OLD USED CONDITION DID NOT WORK WITH NEGATIVE CELL VALUES
        // want this situation: lbd < d-lp < d < d+up < ubd
        if ((d-lp)<lbd){
            printf("ERROR: reading lp=%lf and nominal=%lf in cell=%d\n",lp,d,l);
            return(1);
        }
    }
    printf(" %d linked cells\n",l);
        

    /* passing through general data structure (for linked tables) */


    fscanf(file,"%d\n",&nsums);
    maxsize = ( ncells<1000 && nsums<100 ? ncells*nsums : 30*nsums );

    list  = (int *)malloc( maxsize*sizeof(int) );
    val   = (signed char *)malloc( maxsize*sizeof(signed char) );
    ncard = (int *)malloc( nsums*sizeof(int) );
    rhs   = (double *)malloc( nsums*sizeof(double) );

    if( !ncard || !rhs || !list || !val ){
        printf("ERROR: Not enough memory for GENERAL DATA STRUCTURE\n");
        return(1);
    }
    j=0;
    for(l=0;l<nsums;l++){
        fscanf(file,"%lf %d :",&d,&k);
        rhs[l] = d;
        ncard[l] = k;
        while(k--){
            fscanf(file," %d (%d)",&d1,&d2);
            list[j] = d1;
            val[j] = d2;
            j++;
        }
        fscanf(file,"\n");
    }

    if( j>maxsize ){
#ifdef STAMP
        printf("ERROR: maxsize=%d  j=%d\n",maxsize,j);
#endif
        return(1);
    }
    
    *_ncells = ncells;
    *_nsums  = nsums;
    *_rhs    = rhs;
    *_ncard  = ncard;
    *_list   = list;
    *_val    = val;
    *_data   = data;
    *_weight = weight;
    *_lpl    = lpl;
    *_upl    = upl;
    *_spl    = spl;
    *_lb     = lb;
    *_ub     = ub;
    *_status = status;
    *_names  = names;
    return(0);
}


/*******************************************************************/


#define   Cell3(i,j,k)  ((i)*N2*N3+(j)*N3+(k))
static int  load_crp3(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                      char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names)
{
    int  i,j,k,l,t,N1,N2,N3,dim,w;
    double d,lbd,ubd,lpl_,upl_,spl_;
    int  ncells,nsums,*ncard,*list,*weight;
    double *data,*lpl,*upl,*spl,*lb,*ub,*rhs;
    char *status;
    signed char *val;
    char **names;
	char s_;


    fscanf(file,"%d %d %d\n",&N1,&N2,&N3);
    if( N1<2 || N2<2 || N3<2 ){
        printf("ERROR: bad input file\n");
        exit(1);
    }
    N1++;
    N2++;
    N3++;
    dim = N1*N2*N3;
    data  = (double *)malloc( dim*sizeof(double) );
    weight= (int *)malloc( dim*sizeof(int) );
    status= (char *)malloc( dim*sizeof(char) );
    lpl   = (double *)malloc( dim*sizeof(double) );
    upl   = (double *)malloc( dim*sizeof(double) );
    spl   = (double *)malloc( dim*sizeof(double) );
    lb    = (double *)malloc( dim*sizeof(double) );
    ub    = (double *)malloc( dim*sizeof(double) );
    names = (char **)malloc( dim*sizeof(char *) );
    if( !data || !weight || !status || !lpl || !upl || !spl || !lb || !ub || !names ){
        printf("ERROR: Not enough memory for READING\n");
        return(1);
    }
    

    t = 0;
    while(1){
        if( fscanf(file,"%d %d %d %lf %d %c %lf %lf %lf %lf %lf\n",&i,&j,&k,&d,&w,&s_,&lbd,&ubd,&lpl_,&upl_,&spl_)<0 ) break;
        l = Cell3(i,j,k);
        data[l] = d;
        weight[l] = w;
        status[l] = s_;
        lpl[l] = lpl_;
        upl[l] = upl_;
		spl[l] = spl_;
        lb[l]  = lbd;
        ub[l]  = ubd;
        names[l] = CellName(i,j,k,-1);
        t++;
        if( lbd>d || ubd<d){
            printf("ERROR: reading lb=%lf,ub=%lf and nominal=%lf in cell=%d\n",lbd,ubd,d,l);
            return(1);
        }
    }
    printf(" %d 3-cells\n",t);
        
#ifndef MARGINALS
    /* computing marginals of the linear system */
    printf(" Asuming: WITHOUT MARGINALS\n");

        for(i=1;i<N1;i++)
            for(j=1;j<N2;j++){
                l = Cell3(i,j,0);
                data[l]   = adding3d(N1,N2,N3,data,i,j,-1);
                weight[l] = adding3i(N1,N2,N3,weight,i,j,-1);
                status[l] = 's';
                upl[l] = lpl[l] = 0;
            }
                
        for(i=1;i<N1;i++)
            for(k=1;k<N3;k++){
                l = Cell3(i,0,k);
                data[l]   = adding3d(N1,N2,N3,data,i,-1,k);
                weight[l] = adding3i(N1,N2,N3,weight,i,-1,k);
                status[l] = 's';
                upl[l] = lpl[l] = 0;
            }
        for(j=1;j<N2;j++)
            for(k=1;k<N3;k++){
                l = Cell3(0,j,k);
                data[l]   = adding3d(N1,N2,N3,data,-1,j,k);
                weight[l] = adding3i(N1,N2,N3,weight,-1,j,k);
                status[l] = 's';
                upl[l] = lpl[l] = 0;
            }
        for(i=1;i<N1;i++){
            l = Cell3(i,0,0);
            data[l]   = adding3d(N1,N2,N3,data,i,-1,0);
            weight[l] = adding3i(N1,N2,N3,weight,i,-1,0);
            status[l] = 's';
            upl[l] = lpl[l] = 0;
        }
        for(j=1;j<N2;j++){
            l = Cell3(0,j,0);
            data[l] = adding3d(N1,N2,N3,data,-1,j,0);
            weight[l] = adding3i(N1,N2,N3,weight,-1,j,0);
            status[l] = 's';
            upl[l] = lpl[l] = 0;
        }
        for(k=1;k<N3;k++){
            l = Cell3(0,0,k);
            data[l] = adding3d(N1,N2,N3,data,-1,0,k);
            weight[l] = adding3i(N1,N2,N3,weight,-1,0,k);
            status[l] = 's';
            upl[l] = lpl[l] = 0;
        }
        l = Cell3(0,0,0);
        data[l] = adding3d(N1,N2,N3,data,0,0,-1);
        weight[l] = adding3i(N1,N2,N3,weight,0,0,-1);
        status[l] = 's';
        upl[l] = lpl[l] = 0;
#endif

    /* passing through general data structure (for linked tables) */

    ncells= N1 * N2 * N3;
    list  = (int *)malloc( 3*ncells*sizeof(int) );
    val   = (signed char *)malloc( 3*ncells*sizeof(signed char) );

    nsums = N1*N2 + N1*N3 + N2*N3;
    ncard = (int *)malloc( nsums*sizeof(int) );
    rhs   = (double *)malloc( nsums*sizeof(double) );

    if( !ncard || !rhs || !list || !val ){
        printf("ERROR: Not enough memory for GENERAL DATA STRUCTURE\n");
        return(1);
    }
    t = l = 0;
    for(i=0;i<N1;i++)
        for(j=0;j<N2;j++){
            rhs[t] = 0;
            for(k=0;k<N3;k++){
                val[l] = ( k ? 1 : -1 );
                list[l++] = Cell3(i,j,k);
            }
            ncard[t++] = N3;
        }
    for(i=0;i<N1;i++)
        for(k=0;k<N3;k++){
            rhs[t] = 0;
            for(j=0;j<N2;j++){
                val[l] = ( j ? -1 : 1 );
                list[l++] = Cell3(i,j,k);
            }
            ncard[t++] = N2;
        }
    for(j=0;j<N2;j++)
        for(k=0;k<N3;k++){
            rhs[t] = 0;
            for(i=0;i<N1;i++){
                val[l] = ( i ? 1 : -1 );
                list[l++] = Cell3(i,j,k);
            }
            ncard[t++] = N1;
        }
    
    if( l != 3*ncells ){
        printf("ERROR: more memory is needed\n");
        exit(1);
    }




    *_ncells = ncells;
    *_nsums  = nsums;
    *_rhs    = rhs;
    *_ncard  = ncard;
    *_list   = list;
    *_val    = val;
    *_data   = data;
    *_weight = weight;
    *_lpl    = lpl;
    *_upl    = upl;
    *_spl    = spl;
    *_lb     = lb;
    *_ub     = ub;
    *_status = status;
    *_names  = names;
    return(0);
}


#ifndef MARGINALS

static double adding3i(N1,N2,N3,data,i,j,k)
int N1,N2,N3,i,j,k;
int *data;
{
    double value = 0;
    if(i<0)
        for(i=1;i<N1;i++) value += data[ Cell3(i,j,k) ];
    else if(j<0)
        for(j=1;j<N2;j++) value += data[ Cell3(i,j,k) ];
    else if(k<0)
        for(k=1;k<N3;k++) value += data[ Cell3(i,j,k) ];
    return( value );
}

static double adding3d(N1,N2,N3,data,i,j,k)
int N1,N2,N3,i,j,k;
double *data;
{
    double value = 0;
    if(i<0)
        for(i=1;i<N1;i++) value += data[ Cell3(i,j,k) ];
    else if(j<0)
        for(j=1;j<N2;j++) value += data[ Cell3(i,j,k) ];
    else if(k<0)
        for(k=1;k<N3;k++) value += data[ Cell3(i,j,k) ];
    return( value );
}
#endif


/*******************************************************************/


#define   Cell2(i,j)  ((i)*N2+(j))
static int  load_crp2(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                      char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names)
{
    int  i,j,l,t,N1,N2,dim,w;
    double  d,lbd,ubd,lpl_,upl_,spl_;
    int  ncells,nsums,*ncard,*list,*weight;
    double *data,*lpl,*upl,*spl,*lb,*ub,*rhs;
    char *status;
    signed char *val;
    char **names;
	char s_;

    fscanf(file,"%d %d\n",&N1,&N2);
    if( N1<2 || N2<2 ){
        printf("ERROR: bad input file\n");
        exit(1);
    }
    N1++;
    N2++;
    dim = N1*N2;
    data  = (double *)malloc( dim*sizeof(double) );
    weight= (int *)malloc( dim*sizeof(int) );
    status= (char *)malloc( dim*sizeof(char) );
    lpl   = (double *)malloc( dim*sizeof(double) );
    upl   = (double *)malloc( dim*sizeof(double) );
    spl   = (double *)malloc( dim*sizeof(double) );
    lb    = (double *)malloc( dim*sizeof(double) );
    ub    = (double *)malloc( dim*sizeof(double) );
    names = (char **)malloc( dim*sizeof(char *) );
    if( !data || !weight || !status || !lpl || !upl || !spl || !lb || !ub || !names ){
        printf("ERROR: Not enough memory for READING\n");
        return(1);
    }

    t = 0;
    while(1){
        if( fscanf(file,"%d %d %lf %d %c %lf %lf %lf %lf %lf\n",&i,&j,&d,&w,&s_,&lbd,&ubd,&lpl_,&upl_,&spl_)<0 ) break;
        l = Cell2(i,j);
        data[l] = d;
        weight[l] = w;
        status[l] = s_;
        lpl[l] = lpl_;
        upl[l] = upl_;
		spl[l] = spl_;
        lb[l]  = lbd;
        ub[l]  = ubd;
        names[l] = CellName(i,j,-1,-1);
        t++;
        if( lbd>d || ubd<d){
            printf("ERROR: reading lb=%lf,ub=%lf and nominal=%lf in cell=%d\n",lbd,ubd,d,l);
            return(1);
        }
    }
    printf(" %d 2-cells\n",t);
        
#ifndef MARGINALS
    /* computing marginals of the linear system */
    printf(" Asuming: WITHOUT MARGINALS\n");

        for(i=1;i<N1;i++){
            l = Cell2(i,0);
            data[l]   = adding2d(N1,N2,data,i,-1);
            weight[l] = adding2i(N1,N2,weight,i,-1);
            status[l] = 's';
            upl[l] = lpl[l] = 0;
        }
        for(j=1;j<N2;j++){
            l = Cell2(0,j);
            data[l] = adding2d(N1,N2,data,-1,j);
            weight[l] = adding2i(N1,N2,weight,-1,j);
            status[l] = 's';
            upl[l] = lpl[l] = 0;
        }
        l = Cell2(0,0);
        data[l] = adding2d(N1,N2,data,0,-1);
        weight[l] = adding2i(N1,N2,weight,0,-1);
        status[l] = 's';
        upl[l] = lpl[l] = 0;
#endif
    
    /* passing through general data structure (for linked tables) */

    ncells= N1 * N2;
    list  = (int *)malloc( 2*ncells*sizeof(int) );
    val   = (signed char *)malloc( 2*ncells*sizeof(signed char) );

    nsums = N1+N2;
    ncard = (int *)malloc( nsums*sizeof(int) );
    rhs   = (double *)malloc( nsums*sizeof(double) );

    if( !ncard || !rhs || !list || !val ){
        printf("ERROR: Not enough memory for GENERAL DATA STRUCTURE\n");
        return(1);
    }
    t = l = 0;
    for(i=0;i<N1;i++){
        rhs[t] = 0;
        for(j=0;j<N2;j++){
            val[l] = ( j ? 1 : -1 );
            list[l++] = Cell2(i,j);
        }
        ncard[t++] = N2;
    }
    for(j=0;j<N2;j++){
        rhs[t] = 0;
        for(i=0;i<N1;i++){
            val[l] = ( i ? -1 : 1 );
            list[l++] = Cell2(i,j);
        }
        ncard[t++] = N1;
    }
    
    if( l != 2*ncells ){
        printf("ERROR: more memory is needed\n");
        exit(1);
    }


    *_ncells = ncells;
    *_nsums  = nsums;
    *_rhs    = rhs;
    *_ncard  = ncard;
    *_list   = list;
    *_val    = val;
    *_data   = data;
    *_weight = weight;
    *_lpl    = lpl;
    *_upl    = upl;
	*_spl    = spl;
    *_lb     = lb;
    *_ub     = ub;
    *_status = status;
    *_names  = names;
    return(0);
}


#ifndef MARGINALS

static int adding2d(N1,N2,data,i,j)
int N1,N2,i,j;
double *data;
{
    double value = 0;
    if(i<0)
        for(i=1;i<N1;i++) value += data[ Cell2(i,j) ];
    else if(j<0)
        for(j=1;j<N2;j++) value += data[ Cell2(i,j) ];
    return( value );
}
static int adding2i(N1,N2,data,i,j)
int N1,N2,i,j;
int *data;
{
    int value = 0;
    if(i<0)
        for(i=1;i<N1;i++) value += data[ Cell2(i,j) ];
    else if(j<0)
        for(j=1;j<N2;j++) value += data[ Cell2(i,j) ];
    return( value );
}
#endif

/*******************************************************************/

static void free_crp(int ncells, double *rhs, int *ncard, int *list, signed char *val, double *data, int *weight, char *status, double *lpl, double *upl, double *spl,
                     double *lb, double *ub, char **names)
{
    int i;

    free(rhs);
    free(ncard);
    free(list);
    free(val);
    free(data);
    free(weight);
    free(status);
    free(lpl);
    free(upl);
    free(spl);
    free(lb);
    free(ub);
    for(i=0;i<ncells;i++)
        if( names[i]!=NULL ) free( names[i] );
    free(names);
}

static char *CellName(int i, int j, int k, int s)
{
    char *ptr;

    ptr = (char *)malloc( 50 * sizeof(char) );
    if( ptr==NULL ){
        printf("ERROR: not enough memory for names\n");
        exit(1);
    }
    if( j<0 )
        sprintf(ptr,"x%d",i);
    else if( k<0 )
        sprintf(ptr,"x%d_%d",i,j);
    else if( s<0 )
        sprintf(ptr,"x%d_%d_%d",i,j,k);
    else 
        sprintf(ptr,"x%d_%d_%d_%d",i,j,k,s);
    
    return ptr;
}


/*********** 4-dim *********/

#define   Cell4(i,j,k,s)  ((i)*N2*N3*N4+(j)*N3*N4+(k)*N4+(s))
static int  load_crp4(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                      char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names)
{
    int  i,j,k,s,l,t,N1,N2,N3,N4,dim,w;
    double d,lbd,ubd,lpl_,upl_,spl_;
    int  ncells,nsums,*ncard,*list,*weight;
    double *data,*lpl,*upl,*spl,*lb,*ub,*rhs;
    char *status;
    signed char *val;
    char **names;
	char s_;


    fscanf(file,"%d %d %d %d\n",&N1,&N2,&N3,&N4);
    if( N1<2 || N2<2 || N3<2 || N4<2 ){
        printf("ERROR: bad input file\n");
        exit(1);
    }
    N1++;
    N2++;
    N3++;
    N4++;
    dim = N1*N2*N3*N4;
    data  = (double *)malloc( dim*sizeof(double) );
    weight= (int *)malloc( dim*sizeof(int) );
    status= (char *)malloc( dim*sizeof(char) );
    lpl   = (double *)malloc( dim*sizeof(double) );
    upl   = (double *)malloc( dim*sizeof(double) );
    spl   = (double *)malloc( dim*sizeof(double) );
    lb    = (double *)malloc( dim*sizeof(double) );
    ub    = (double *)malloc( dim*sizeof(double) );
    names = (char **)malloc( dim*sizeof(char *) );
    if( !data || !weight || !status || !lpl || !upl || !spl || !lb || !ub || !names ){
        printf("ERROR: Not enough memory for READING\n");
        return(1);
    }

    t = 0;
    while(1){
        if( fscanf(file,"%d %d %d %d %lf %d %c %lf %lf %lf %lf %lf\n",
			&i,&j,&k,&s,&d,&w,&s_,&lbd,&ubd,&lpl_,&upl_,&spl_)<0 ) break;
        l = Cell4(i,j,k,s);
        data[l] = d;
        weight[l] = w;
        status[l] = s_;
        lpl[l] = lpl_;    
        upl[l] = upl_;      
		spl[l] = spl_;
        lb[l]  = lbd;
        ub[l]  = ubd;
        names[l] = CellName(i,j,k,s);
        t++;
        if( lbd>d || ubd<d){
            printf("ERROR: reading lb=%lf,ub=%lf and nominal=%lf in cell=%d\n",lbd,ubd,d,l);
            return(1);
        }
    }
    printf(" %d 4-cells\n",t);
        
#ifndef MARGINALS
    /* computing marginals of the linear system */
    printf(" Asuming: WITHOUT MARGINALS\n");

        for(i=1;i<N1;i++)
            for(j=1;j<N2;j++){
                l = Cell3(i,j,0);
                data[l]   = adding3d(N1,N2,N3,data,i,j,-1);
                weight[l] = adding3i(N1,N2,N3,weight,i,j,-1);
                status[l] = 's';
                upl[l] = lpl[l] = 0;
            }
                
        for(i=1;i<N1;i++)
            for(k=1;k<N3;k++){
                l = Cell3(i,0,k);
                data[l]   = adding3d(N1,N2,N3,data,i,-1,k);
                weight[l] = adding3i(N1,N2,N3,weight,i,-1,k);
                status[l] = 's';
                upl[l] = lpl[l] = 0;
            }
        for(j=1;j<N2;j++)
            for(k=1;k<N3;k++){
                l = Cell3(0,j,k);
                data[l]   = adding3d(N1,N2,N3,data,-1,j,k);
                weight[l] = adding3i(N1,N2,N3,weight,-1,j,k);
                status[l] = 's';
                upl[l] = lpl[l] = 0;
            }
        for(i=1;i<N1;i++){
            l = Cell3(i,0,0);
            data[l]   = adding3d(N1,N2,N3,data,i,-1,0);
            weight[l] = adding3i(N1,N2,N3,weight,i,-1,0);
            status[l] = 's';
            upl[l] = lpl[l] = 0;
        }
        for(j=1;j<N2;j++){
            l = Cell3(0,j,0);
            data[l] = adding3d(N1,N2,N3,data,-1,j,0);
            weight[l] = adding3i(N1,N2,N3,weight,-1,j,0);
            status[l] = 's';
            upl[l] = lpl[l] = 0;
        }
        for(k=1;k<N3;k++){
            l = Cell3(0,0,k);
            data[l] = adding3d(N1,N2,N3,data,-1,0,k);
            weight[l] = adding3i(N1,N2,N3,weight,-1,0,k);
            status[l] = 's';
            upl[l] = lpl[l] = 0;
        }
        l = Cell3(0,0,0);
        data[l] = adding3d(N1,N2,N3,data,0,0,-1);
        weight[l] = adding3i(N1,N2,N3,weight,0,0,-1);
        status[l] = 's';
        upl[l] = lpl[l] = 0;
#endif

    
    /* passing through general data structure (for linked tables) */

    ncells= N1 * N2 * N3 * N4;
    list  = (int *)malloc( 4*ncells*sizeof(int) );
    val   = (signed char *)malloc( 4*ncells*sizeof(signed char) );

    nsums = N2*N3*N4 + N1*N3*N4 + N1*N2*N4 + N1*N2*N3;
    ncard = (int *)malloc( nsums*sizeof(int) );
    rhs   = (double *)malloc( nsums*sizeof(double) );

    if( !ncard || !rhs || !list || !val ){
        printf("ERROR: Not enough memory for GENERAL DATA STRUCTURE\n");
        return(1);
    }
    t = l = 0;
    for(i=0;i<N1;i++)
        for(j=0;j<N2;j++)
            for(k=0;k<N3;k++){
                rhs[t] = 0;
                for(s=0;s<N4;s++){
                    val[l] = ( s ? 1 : -1 );
                    list[l++] = Cell4(i,j,k,s);
                }
                ncard[t++] = N4;
            }
    for(i=0;i<N1;i++)
        for(j=0;j<N2;j++)
            for(s=0;s<N4;s++){
                rhs[t] = 0;
                for(k=0;k<N3;k++){
                    val[l] = ( k ? 1 : -1 );
                    list[l++] = Cell4(i,j,k,s);
                }
                ncard[t++] = N3;
            }
    for(i=0;i<N1;i++)
        for(s=0;s<N4;s++)
            for(k=0;k<N3;k++){
                rhs[t] = 0;
                for(j=0;j<N2;j++){
                    val[l] = ( j ? 1 : -1 );
                    list[l++] = Cell4(i,j,k,s);
                }
                ncard[t++] = N2;
            }
    for(s=0;s<N4;s++)
        for(j=0;j<N2;j++)
            for(k=0;k<N3;k++){
                rhs[t] = 0;
                for(i=0;i<N1;i++){
                    val[l] = ( i ? 1 : -1 );
                    list[l++] = Cell4(i,j,k,s);
                }
                ncard[t++] = N1;
            }
    
    if( l != 4*ncells ){
        printf("ERROR: more memory is needed\n");
        exit(1);
    }




    *_ncells = ncells;
    *_nsums  = nsums;
    *_rhs    = rhs;
    *_ncard  = ncard;
    *_list   = list;
    *_val    = val;
    *_data   = data;
    *_weight = weight;
    *_lpl    = lpl;
    *_upl    = upl;
	*_spl    = spl;
    *_lb     = lb;
    *_ub     = ub;
    *_status = status;
    *_names  = names;
    return(0);
}



//////////////  to load linked tables

static int  load_crp0(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                         char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names)
{
    int  l,k,i,counter;
    double d;
    int  ncells,nsums,nzero,*ncard,*list,*weight;
    double *data,*lpl,*upl,*spl,*lb,*ub,*rhs;
    char *status,*ptr;
    signed char *val;
    char **names;
	long position;
	char cadena[100];


    position = ftell( file );
	for(ncells=0 ; strcmp(cadena,"END") ; fscanf(file,"%s",cadena) )
		ncells++;
	ncells = ncells/2;
	fseek( file , position , SEEK_SET );


    data  = (double *)malloc( ncells*sizeof(double) );
    weight= (int *)malloc( ncells*sizeof(int) );
    status= (char *)malloc( ncells*sizeof(char) );
    lpl   = (double *)malloc( ncells*sizeof(double) );
    upl   = (double *)malloc( ncells*sizeof(double) );
    spl   = (double *)malloc( ncells*sizeof(double) );
    lb    = (double *)malloc( ncells*sizeof(double) );
    ub    = (double *)malloc( ncells*sizeof(double) );
    names = (char **)malloc( ncells*sizeof(char *) );
    if( !data || !weight || !status || !lpl || !upl || !lb || !ub || !names ){
        printf("ERROR: Not enough memory for READING\n");
        return(1);
    }

    for(l=0;l<ncells;l++){
        fscanf(file,"%s",cadena);
		fscanf(file,"%lf",&d);
        data[l] = d;
        weight[l] = 1;
        status[l] = 's';
        lpl[l] = 0;
        upl[l] = 0;
		spl[l] = 0;
        lb[l]  = 0;  // ( d>0.001 ? 1 : 0);
        ub[l]  = ( d>0.001 ? 10000 : 0);
        names[l] = ptr = (char *)malloc( (strlen(cadena)+1) * sizeof(char) );
		if( ptr==NULL ){
			printf("ERROR: not enough memory for names\n");
			exit(1);
		}
		strcpy(ptr,cadena);
    }
    fscanf(file,"%s",cadena);      //END
        

    
    /* passing through general data structure (for linked tables) */


    position = ftell( file );
	nsums=nzero=0;
	do{
		fscanf(file,"%s",cadena);
		if( cadena[0]=='=' )
			nsums++;
		nzero++;
	} while( strcmp(cadena,"END") );

	nzero = nzero/2;
	fseek( file , position , SEEK_SET );

    list  = (int *)malloc( nzero*sizeof(int) );
    val   = (signed char *)malloc( nzero*sizeof(signed char) );
    ncard = (int *)malloc( nsums*sizeof(int) );
    rhs   = (double *)malloc( nsums*sizeof(double) );

    if( !ncard || !rhs || !list || !val ){
        printf("ERROR: Not enough memory for GENERAL DATA STRUCTURE\n");
        return(1);
    }


	counter = 0;
	for(l=0;l<nsums;l++){
		position = ftell( file );
		k=0;
		do{
			fscanf(file,"%s",cadena);
			k++;
		}while( cadena[0]!='&' );
		k = k/2;
		ncard[l] = k;
		fseek( file , position , SEEK_SET );

		d = 0;
		do{
			fscanf(file,"%s",cadena);
			list[ counter ] = i = cellindex( cadena , ncells , names );
			val[ counter ] = -1;
			counter++;
			fscanf(file,"%s",cadena);
			d += data[i];
		} while( cadena[0]!='=' );

		do{
			fscanf(file,"%s",cadena);
			list[ counter ] = i = cellindex( cadena , ncells , names );
			val[ counter ] = +1;
			counter++;
			fscanf(file,"%s",cadena);
			d -= data[i];
		} while( cadena[0]!='&' );
		if( fabs(d)>0.001 ){
			printf("ERROR: equation %d is not satisfied\n",l);
			return 1;
		}
		rhs[l] = 0;
	}
	if( counter!=nzero ){
		printf("ERROR:\n");
		return 1;
	}

    *_ncells = ncells;
    *_nsums  = nsums;
    *_rhs    = rhs;
    *_ncard  = ncard;
    *_list   = list;
    *_val    = val;
    *_data   = data;
    *_weight = weight;
    *_lpl    = lpl;
    *_upl    = upl;
	*_spl    = spl;
    *_lb     = lb;
    *_ub     = ub;
    *_status = status;
    *_names  = names;
    return(0);
}

static int cellindex( char *nombre , int ncells , char **Tnombre )
{
	while(ncells--)
		if( !strcmp(nombre,Tnombre[ncells]) )
			return ncells;
	printf("ERROR: undefined variable %s in a the equation system\n",nombre);
	return -1;
}


/*************************************************************************/

static int  load_crp4simple(FILE *file ,int *_ncells,int *_nsums,double **_rhs,int **_ncard,int **_list,signed char **_val,double **_data,int **_weight,
                            char **_status,double **_lpl,double **_upl,double **_spl,double **_lb,double **_ub,char ***_names)
{
    int  i,j,k,s,l,t,N1,N2,N3,N4,dim,w;
    double d,lbd,ubd;
    int  ncells,nsums,*ncard,*list,*weight;
    double *data,*lpl,*upl,*spl,*lb,*ub,*rhs;
    char *status;
    signed char *val;
    char **names;


    fscanf(file,"%d %d %d %d\n",&N1,&N2,&N3,&N4);
    if( N1<2 || N2<2 || N3<2 || N4<2 ){
        printf("ERROR: bad input file\n");
        exit(1);
    }
    N1++;
    N2++;
    N3++;
    N4++;
    dim = N1*N2*N3*N4;
    data  = (double *)malloc( dim*sizeof(double) );
    weight= (int *)malloc( dim*sizeof(int) );
    status= (char *)malloc( dim*sizeof(char) );
    lpl   = (double *)malloc( dim*sizeof(double) );
    upl   = (double *)malloc( dim*sizeof(double) );
    spl   = (double *)malloc( dim*sizeof(double) );
    lb    = (double *)malloc( dim*sizeof(double) );
    ub    = (double *)malloc( dim*sizeof(double) );
    names = (char **)malloc( dim*sizeof(char *) );
    if( !data || !weight || !status || !lpl || !upl || !spl || !lb || !ub || !names ){
        printf("ERROR: Not enough memory for READING\n");
        return(1);
    }

    t = 0;
    while(1){
        if( fscanf(file,"%d %d %d %d %lf %d %lf %lf\n",
			&i,&j,&k,&s,&d,&w,&lbd,&ubd)<0 ) break;
        l = Cell4(i,j,k,s);
        data[l] = d;
        weight[l] = w;
        status[l] = 's';
        lpl[l] = 0;    
        upl[l] = 0;      
		spl[l] = 0;
        lb[l]  = lbd;
        ub[l]  = ubd;
        names[l] = CellName(i,j,k,s);
        t++;
        if( lbd>d || ubd<d){
            printf("ERROR: reading lb=%lf,ub=%lf and nominal=%lf in cell=%d\n",lbd,ubd,d,l);
            return(1);
        }
    }
    printf(" %d 4-cells\n",t);
        
#ifndef MARGINALS
    /* computing marginals of the linear system */
    printf(" Asuming: WITHOUT MARGINALS\n");

        for(i=1;i<N1;i++)
            for(j=1;j<N2;j++){
                l = Cell3(i,j,0);
                data[l]   = adding3d(N1,N2,N3,data,i,j,-1);
                weight[l] = adding3i(N1,N2,N3,weight,i,j,-1);
                status[l] = 's';
                upl[l] = lpl[l] = 0;
            }
                
        for(i=1;i<N1;i++)
            for(k=1;k<N3;k++){
                l = Cell3(i,0,k);
                data[l]   = adding3d(N1,N2,N3,data,i,-1,k);
                weight[l] = adding3i(N1,N2,N3,weight,i,-1,k);
                status[l] = 's';
                upl[l] = lpl[l] = 0;
            }
        for(j=1;j<N2;j++)
            for(k=1;k<N3;k++){
                l = Cell3(0,j,k);
                data[l]   = adding3d(N1,N2,N3,data,-1,j,k);
                weight[l] = adding3i(N1,N2,N3,weight,-1,j,k);
                status[l] = 's';
                upl[l] = lpl[l] = 0;
            }
        for(i=1;i<N1;i++){
            l = Cell3(i,0,0);
            data[l]   = adding3d(N1,N2,N3,data,i,-1,0);
            weight[l] = adding3i(N1,N2,N3,weight,i,-1,0);
            status[l] = 's';
            upl[l] = lpl[l] = 0;
        }
        for(j=1;j<N2;j++){
            l = Cell3(0,j,0);
            data[l] = adding3d(N1,N2,N3,data,-1,j,0);
            weight[l] = adding3i(N1,N2,N3,weight,-1,j,0);
            status[l] = 's';
            upl[l] = lpl[l] = 0;
        }
        for(k=1;k<N3;k++){
            l = Cell3(0,0,k);
            data[l] = adding3d(N1,N2,N3,data,-1,0,k);
            weight[l] = adding3i(N1,N2,N3,weight,-1,0,k);
            status[l] = 's';
            upl[l] = lpl[l] = 0;
        }
        l = Cell3(0,0,0);
        data[l] = adding3d(N1,N2,N3,data,0,0,-1);
        weight[l] = adding3i(N1,N2,N3,weight,0,0,-1);
        status[l] = 's';
        upl[l] = lpl[l] = 0;
#endif

    
    /* passing through general data structure (for linked tables) */

    ncells= N1 * N2 * N3 * N4;
    list  = (int *)malloc( 4*ncells*sizeof(int) );
    val   = (signed char *)malloc( 4*ncells*sizeof(signed char) );

    nsums = N2*N3*N4 + N1*N3*N4 + N1*N2*N4 + N1*N2*N3;
    ncard = (int *)malloc( nsums*sizeof(int) );
    rhs   = (double *)malloc( nsums*sizeof(double) );

    if( !ncard || !rhs || !list || !val ){
        printf("ERROR: Not enough memory for GENERAL DATA STRUCTURE\n");
        return(1);
    }
    t = l = 0;
    for(i=0;i<N1;i++)
        for(j=0;j<N2;j++)
            for(k=0;k<N3;k++){
                rhs[t] = 0;
                for(s=0;s<N4;s++){
                    val[l] = ( s ? 1 : -1 );
                    list[l++] = Cell4(i,j,k,s);
                }
                ncard[t++] = N4;
            }
    for(i=0;i<N1;i++)
        for(j=0;j<N2;j++)
            for(s=0;s<N4;s++){
                rhs[t] = 0;
                for(k=0;k<N3;k++){
                    val[l] = ( k ? 1 : -1 );
                    list[l++] = Cell4(i,j,k,s);
                }
                ncard[t++] = N3;
            }
    for(i=0;i<N1;i++)
        for(s=0;s<N4;s++)
            for(k=0;k<N3;k++){
                rhs[t] = 0;
                for(j=0;j<N2;j++){
                    val[l] = ( j ? 1 : -1 );
                    list[l++] = Cell4(i,j,k,s);
                }
                ncard[t++] = N2;
            }
    for(s=0;s<N4;s++)
        for(j=0;j<N2;j++)
            for(k=0;k<N3;k++){
                rhs[t] = 0;
                for(i=0;i<N1;i++){
                    val[l] = ( i ? 1 : -1 );
                    list[l++] = Cell4(i,j,k,s);
                }
                ncard[t++] = N1;
            }
    
    if( l != 4*ncells ){
        printf("ERROR: more memory is needed\n");
        exit(1);
    }




    *_ncells = ncells;
    *_nsums  = nsums;
    *_rhs    = rhs;
    *_ncard  = ncard;
    *_list   = list;
    *_val    = val;
    *_data   = data;
    *_weight = weight;
    *_lpl    = lpl;
    *_upl    = upl;
	*_spl    = spl;
    *_lb     = lb;
    *_ub     = ub;
    *_status = status;
    *_names  = names;
    return(0);
}

