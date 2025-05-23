// XXRounder.cpp : Implementation of CXXRounder
#include "stdafx.h"
#include "XXRoundCom.h"
#include "XXRounder.h"
#include "LogFile.h"
#include "crpmain.h"
#include "GlobErrors.h"
#include "xprs.h"
#include "crpload.h"

#define MAXUB   1.0E+40
/////////////////////////////////////////////////////////////////////////////
// CXXRounder

/////////////////////////////////////////////////////////////////////////////
// CXpresRounder

void CXXRounder::FireUpdateLowerBound(double LowerBound)
{
    Fire_UpdateProgressLowerBound(LowerBound);
}
void CXXRounder::FireUpdateUpperBound(double UpperBound)
{
    Fire_UpdateProgressUpperBound(UpperBound);
}
void CXXRounder::FireUpdateNumberCloseSubProb(long CloseSubProb)
{
    Fire_UpdateNumberCloseSubProb(CloseSubProb);
}
void CXXRounder::FireUpdateNumberOpenSubProb(long OpenSubProb)
{
    Fire_UpdateNumberOpenSubProb(OpenSubProb);
}

void CXXRounder::FireUpdateNumberFeasible(long Feasible)
{
    Fire_UpdateNumberFeasible(Feasible);
}
void CXXRounder::FireUpdateProgress(long Progress)
{
    Fire_UpdateProgress(Progress);
}


//Start JJ message function
#define XPRESS
#include "rounder.h"

double InitialTime,TotalTime,MaxTime;
char namepathexe[MAXPATH];
long StoppingRule;
CXXRounder *mypointer;

int TAUmessage(double lb,double ub,double rapid,int nodedone,int nodeleft)
{
    char   buf1[MAXPATH],buf2[MAXPATH],buf3[MAXPATH],buf4[MAXPATH],buf5[MAXPATH],buf6[MAXPATH];
    double TotalTime = (double)clock()/CLOCKS_PER_SEC - InitialTime;

    mypointer->FireUpdateLowerBound(lb);
    mypointer->FireUpdateUpperBound(ub);
    mypointer->FireUpdateNumberCloseSubProb(nodedone);
    mypointer->FireUpdateNumberOpenSubProb(nodeleft);
//  mypointer->Fire_UpdateNumberFeasible(NFeas);
//  mypointer->FireUpdateProgress(Prog);

    if (StoppingRule == 1) return 1; //Only the first Rapid

    if ( (StoppingRule == 2) && ( (ub - MAXUB) < -10) ) return 1; // stop after the first feasible solution
                                                           // -10 voor precisie problemen

    if( TotalTime > MaxTime ){
        sprintf(buf1,"%.2lf",lb);
        sprintf(buf2,"%.2lf",ub);
        sprintf(buf3,"%d",nodedone);
        sprintf(buf4,"%d",nodeleft);
        sprintf(buf5,"%.2lf",TotalTime/60.0);
        sprintf(buf6,"%.2lf",rapid);
#if defined(_WIN32) || defined(_WIN64)
        MaxTime += 60.0 * _spawnl(_P_WAIT, namepathexe, namepathexe, buf1, buf2, buf3, buf4, buf5, buf6, NULL );
#else
        // Compose command string for system()
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "%s %s %s %s %s %s %s %s", namepathexe, buf1, buf2, buf3, buf4, buf5, buf6);
        MaxTime += 60.0 * system(cmd);
#endif
        if( TotalTime > MaxTime )
            return 1;                // abortion
    }
    return 0;                        // continue
}
// end of JJ TauMessage function



STDMETHODIMP CXXRounder::DoRoundXpress(BSTR InFileName, long Base,
									   double *UpperBound, double *LowerBound,
									   long audit, long StopRule, BSTR OutSolutionFileName,
									   BSTR OutStatisticsFileName,
									   BSTR ilmfile, BSTR LogFile,
									   long maxtime, long ZeroRestricted,
									   BSTR NamePathExe, double *MaxJump,
									   long *NumberJump, double *UsedTime,
									   long *SolutionType,
									   long *ErrorCode, VARIANT_BOOL *pVal)
{

    int    ncells,nsums,i,j;
    double d,base;
    int    *ncard,*list,*weight;
    double *data,*lpl,*upl,*spl,*lb,*ub,*rhs;
    char   *status;
    char   **names;
    signed char *val;
    clock_t  t1,t2,t3,t4;
    int      dim,auditing;
    double   upperbound,lowerbound,ttime1,ttime2;
    FILE     *file,*flog;
    CString  inFileName;
    CString  outSolFileName, outStatFileName;
    CString  ILMFile = ilmfile;
    CString  csLogFile,sNamePathExe;
    LPCTSTR  sLogFile = CString(LogFile);
    csLogFile = LogFile;

    outSolFileName = OutSolutionFileName;
    outStatFileName = OutStatisticsFileName;
	inFileName = InFileName;
    sNamePathExe = NamePathExe;
    CLogFile objlogfile;
    objlogfile.openfiletowrite(sLogFile);
    objlogfile.closefile();
	char infilefile[MAXPATH];
    char outsolfile[MAXPATH];
    char outstatfile[MAXPATH];
//    char namepathexe[MAXPATH];
    char namepathlic[MAXPATH];
	strcpy(infilefile, (const char *) inFileName);
    strcpy(outsolfile, (const char *) outSolFileName);
    strcpy(outstatfile, (const char *)outStatFileName);
    strcpy(namepathexe, (const char *)sNamePathExe);
    strcpy(namepathlic, (const char *)ILMFile);
    int largemodel = ZeroRestricted;

	int  beta  = (int)Base;
	int  delta = (int)audit;
    char inputfilename[MAXPATH];
    char namelogfile[MAXPATH];
	int  number,error;

    strcpy(inputfilename, (const char *) InFileName);
    strcpy(namelogfile, (const char *)sLogFile);
	MaxTime = 60.0 * maxtime;
	mypointer = this;
	StoppingRule = StopRule;

    InitialTime = (double)clock()/CLOCKS_PER_SEC;
    i = doXPRround(infilefile, beta, UpperBound, LowerBound, delta,
			outsolfile, outstatfile, namepathlic, namelogfile,
			largemodel,MaxJump, &number, &error);
	TotalTime = (double)clock()/CLOCKS_PER_SEC - InitialTime;

	*NumberJump  = (long)number;
	*ErrorCode   = (long)error;
	*SolutionType = (long)i;
	*UsedTime    = (long)TotalTime/60.0;
//	return i;
	if (i == 0 ) {
        *pVal = VARIANT_FALSE;
        *ErrorCode = error;
        return S_OK;
    }

    *pVal = VARIANT_TRUE;
    return S_OK;



 /*   if( CRPopenSOLVER(namepathlic,sLogFile) )    {
            objlogfile.openfiletoappend(sLogFile);
            fprintf(objlogfile.flog,"Problems with the XPRESS license\n");
            objlogfile.closefile();
            *ErrorCode = NOLICENSE;
            *pVal = VARIANT_FALSE;
            return S_OK;
    }
*/
/***
    int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
    tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
    tmpFlag |= _CRTDBG_CHECK_CRT_DF;
    tmpFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
//  tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag( tmpFlag );
***/

    //if( argc < 4 ){
    //     printf("Usage:  %s   file   base   auditing(0,1,2)\n",argv[0]);
    //     return(1);
    //}

    //base = atoi( argv[2] );
    //auditing = atoi( argv[3] );
    //file = fopen(argv[1],"r");
    //printf("file=%s  base=%d  auditing=%d\n",argv[0],base,auditing);
    inFileName = InFileName;
    file = fopen(inFileName, "r");
    if( file==NULL ){
        objlogfile.openfiletoappend(sLogFile);
        fprintf(objlogfile.flog,"ERROR: it is not possible to open file %s\n",inFileName);
        objlogfile.closefile();
        *ErrorCode = NOINPUTFILE;
        *pVal = VARIANT_FALSE;
        CRPcloseSOLVER();
        return S_OK;
    }
    fscanf(file,"%d\n",&dim);

    objlogfile.openfiletoappend(sLogFile);
	flog = objlogfile.flog;
    switch( dim ){
        case -1 :
             i= CRPread_file(flog,file,&ncells,&nsums,&rhs,&ncard,&list,&val,
                 &data,&weight,&status,&lpl,&upl,&spl,&lb,&ub,&names) ;
             break;
        case 0 :
             i= CRPread0file(flog,file,&ncells,&nsums,&rhs,&ncard,&list,&val,
                 &data,&weight,&status,&lpl,&upl,&spl,&lb,&ub,&names) ;
             break;
        case 2 :
             i= CRPread2file(flog,file,&ncells,&nsums,&rhs,&ncard,&list,&val,
                 &data,&weight,&status,&lpl,&upl,&spl,&lb,&ub,&names) ;
             break;
        case 3 :
             i= CRPread3file(flog,file,&ncells,&nsums,&rhs,&ncard,&list,&val,
                 &data,&weight,&status,&lpl,&upl,&spl,&lb,&ub,&names) ;
             break;
        case 4 :
             i= CRPread4file(flog,file,&ncells,&nsums,&rhs,&ncard,&list,&val,
                 &data,&weight,&status,&lpl,&upl,&spl,&lb,&ub,&names) ;
             break;
        default:
			 i= 1;
    }

    if(i){
         fprintf(objlogfile.flog,"ERROR: data in %s is in incorrect format\n",inFileName);
         objlogfile.closefile();
         *ErrorCode = NOJJFORMAT;
         *pVal = VARIANT_FALSE;
         CRPcloseSOLVER();
         return S_OK;
    }
    objlogfile.closefile();


    fclose(file);
    base = Base;
    auditing = audit;

    upperbound = 1E+20;
    lowerbound = 0;

//  upperbound = UpperBound;
//  lowerbound = LowerBound;

    i = CRPloadprob(nsums,rhs,ncells,data,weight,status,lpl,upl,spl,lb,ub,names,ncard,list,val,base, sLogFile);//, sLogFile);
    //free_crp(ncells,rhs,ncard,list,val,data,weight,status,lpl,upl,spl,lb,ub,names);
    CRPunreadfile(ncells,rhs,ncard,list,val,data,weight,status,lpl,upl,spl,lb,ub,names);

    if(i){
        //printf(" ERROR: loading the CRP instance\n");
        //return(1);

        objlogfile.openfiletoappend(sLogFile);
        fprintf(objlogfile.flog,"ERROR: loading the CRP instance\n");
        objlogfile.closefile();
        *ErrorCode = NOLOADPROBLEM;
        *pVal = VARIANT_FALSE;
        CRPcloseSOLVER();
        return S_OK;
    }


    t1 = clock();
    i = CRPoptimize(&lowerbound,&upperbound, sLogFile, this, maxtime, largemodel, namepathexe);
    t2 = clock();
    ttime1 = (float)(t2-t1)/CLOCKS_PER_SEC;
    *UsedTime   = ttime1/60.0;
    *UpperBound = upperbound;
    *LowerBound = lowerbound;

    if(i==10) {
        *pVal = VARIANT_FALSE;
        *ErrorCode = NOSOLVEDPROBLEM;
        CRPfreeprob();
        CRPcloseSOLVER();
        return S_OK;
    }
    if(i==2 || i==4) {
        *pVal = VARIANT_FALSE;
        *ErrorCode = INFEASIBLEPROBLEM;
        CRPfreeprob();
        CRPcloseSOLVER();
        return S_OK;
    }

    //
    //     We have a solution!!!!!!!!!!!!!!
    //
   // i = CRPprintsolution("C:/jj/ONS_controlled_rounding/ANCO_errores/solution.txt","C:/jj/ONS_controlled_rounding/ANCO_errores/statistics.txt", sLogFile);
    i = CRPprintsolution(outsolfile,outstatfile,sLogFile,&d,&j);
    *NumberJump = j;
    *MaxJump    = d;
    if(i) {
        *pVal = VARIANT_FALSE;
        *ErrorCode = NOPRINTING;
        CRPfreeprob();
        CRPcloseSOLVER();
        return S_OK;
    }


    t3 = clock();
    i = CRPauditing(auditing, largemodel, base , sLogFile, this);
    t4 = clock();
    ttime2 = (float)(t4-t3)/CLOCKS_PER_SEC;
    if(i) {
        *pVal = VARIANT_FALSE;
        *ErrorCode = NOAUDITING;
        CRPfreeprob();
        CRPcloseSOLVER();
        return S_OK;
    }


    i = CRPfreeprob();
    if(i){
        // printf(" ERROR: unloading the CRP instance\n");
        //return(1);
        objlogfile.openfiletoappend(sLogFile);
        fprintf(objlogfile.flog,"ERROR: unloading the CRP instance\n");
        objlogfile.closefile();
        *pVal = VARIANT_FALSE;
        *ErrorCode = NOFREEPROBLEM;
        CRPcloseSOLVER();
        return S_OK;
    }
    //printf(" opt-Time = %.2f ; audit-time = %.2f ; UB = %.1f; LB = %.1f\n",ttime1,ttime2,upperbound,lowerbound);

    if( CRPcloseSOLVER())   { //sLogFile) ){
    //  printf("Problems with the SOLVER license\n");
         objlogfile.openfiletoappend(sLogFile);
         fprintf(objlogfile.flog,"Problems with the SOLVER license\n");
         objlogfile.closefile();

    //  return 1;
         *pVal = VARIANT_FALSE;
         *ErrorCode = NOCLOSELICENSE;
         return S_OK;
    }

 //   _ASSERTE( !_CrtDumpMemoryLeaks( ) );
 //   _ASSERTE( _CrtCheckMemory( ) );
    *pVal = VARIANT_TRUE;

    return S_OK;
}

bool  CXXRounder::load_crp0old(FILE*file,int*_ncells,int*_nsums,double**_rhs,int**_ncard,int**_list,
                                            signed char** _val,double** _data,int**_weight,char**_status,double**_lpl,
                                            double**_upl,double**_spl,
                                            double**_lb,double**_ub,char***_names)
{
    int  j,k,l,dim,w,d1,d2,maxsize;
    double d,lbd,ubd,up,lp,sp;
    int  ncells,nsums,*ncard,*list,*weight;
    double *data,*lpl,*upl,*spl,*lb,*ub,*rhs;
    char *status;
    signed char *val;
    char **names;
    char s;
    //char username[LNAME];
     char username[256];
     CString tempstr;


    fscanf(file,"%d\n",&ncells);
    if( ncells<2 ){
        /*printf("ERROR: bad input file\n");
        exit(1);*/
         return false;
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
      /*  printf("ERROR: Not enough memory for READING\n");
        return(1);
          */
         return false;
    }


    for(l=0;l<ncells;l++){
        if( fscanf(file,"%d %lf %d %c %lf %lf %lf %lf %lf\n",&k,&d,&w,&s,&lbd,&ubd,&lp,&up,&sp)<0 ) break;
        if( l != k ){
            /*printf("ERROR: l=%d k=%d\n",l,k);
            return(1);*/
              return false;
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
//        names[l] = CellName(k,-1,-1,-1);
        username[0]='X';
        _itoa(k,username+1,10);
        names[l] = (char *)malloc( strlen(username)+1 );
        strcpy( names[l] , username );
        if( lp>d ){
            /*printf("ERROR: reading lp=%lf and nominal=%lf in cell=%d\n",lp,d,l);
            return(1);*/
              return false;
        }
    }
    //printf(" %d linked cells\n",l);


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
/*#ifdef STAMP
        printf("ERROR: maxsize=%d  j=%d\n",maxsize,j);
#endif*/
       // return(1);
         return false;
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
 //   return(0);
     return true;

}

void CXXRounder::free_crp(int ncells,double* rhs,int* ncard,int* list,
                                                signed char*val,double*data,int*weight,
                                                char*status,double*lpl,double*upl,double*spl,
                                                double*lb,double*ub,char**names)
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
