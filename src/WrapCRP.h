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

#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
    #define EXPORTFUNC __declspec(dllexport)
#else
    #define EXPORTFUNC __attribute__ ((visibility("default")))
#endif

//#ifdef DLL
//__declspec(dllimport) double ZERO;           /* zero-epsilon                             */
//__declspec(dllimport) double INF;            /* infinity                                 */
//__declspec(dllimport) double MAX_TIME;       /* maximum total CPU time                   */
//__declspec(dllimport) double MIN_VIOLA;      /* minimum violation for valid cuts         */
//__declspec(dllimport) double MAX_SLACK;      /* maximum slack for cuts in the LP         */
//__declspec(dllimport) void (*CRPmessage)(double,double,int,int);
//__declspec(dllimport) int  (*CRPextratime)(void);
//#else
double ZERO;           /* zero-epsilon                             */
double INF;            /* infinity                                 */
double MAX_TIME;       /* maximum total CPU time                   */
double MIN_VIOLA;      /* minimum violation for valid cuts         */
double MAX_SLACK;      /* maximum slack for cuts in the LP         */
void (*CRPmessage)(double,double,int,int);
int  (*CRPextratime)(void);
//#endif

enum ErrorTypes {
        NOLICENSE = 7000,
        NOINPUTFILE,
        NOJJFORMAT,
        NOLOADPROBLEM,
        NOSOLVEDPROBLEM,
        INFEASIBLEPROBLEM,
        NOAUDITING,
        NOPRINTING,
        NOFREEPROBLEM,
        NOCLOSELICENSE,
	NOOUTPUTFILE
};


#ifdef __cplusplus
extern "C" {
#endif

void   CRPSetFileNames(const char*, const char*);
void   CRPFreeFileNames(void);
int    CRPopenSOLVER(const char*, const char*);
int    CRPcloseSOLVER(const char*);
int    CRPoptimize(const char* ,double*,double*);
int    CRPloadprob(const char* ,int,double*,int,double*,int*,char*,double*,double*,double*,double*,double*,char**,int*,int*,signed char*,int);
int    CRPfreeprob(const char* );
int    CRPprintsolution(char*,char*,double*,int*);
int    CRPauditing(const char* ,int);

#ifdef DYNAMIC
EXPORTFUNC void   CRPsetfunctions( void (*f)(double,double,int,int) , int (*g)(void) );
EXPORTFUNC void   CRPSetDoubleConstant(const int, double);
EXPORTFUNC double CRPGetDoubleConstant(const int);
EXPORTFUNC int do_round(char *Solver, char *InFileName, double Base, double *UpperBound, double *LowerBound, long Auditing, 
	     char *SolutionFile, char *StatisticsFile, char *LicenseFile, char *LogFile,
             long MaxTime, long ZeroRestricted, char *NamePathExe,
             double *MaxJump, long *NumberJump, double *UsedTime, long *ErrorCode);
#else
void   CRPsetfunctions( void (*f)(double,double,int,int) , int (*g)(void) );
void   CRPSetDoubleConstant(const int, double);
double CRPGetDoubleConstant(const int);
int do_round(char *Solver, char *InFileName, double Base, double *UpperBound, double *LowerBound, long Auditing, 
	     char *SolutionFile, char *StatisticsFile, char *LicenseFile, char *LogFile,
             long MaxTime, long ZeroRestricted, char *NamePathExe,
             double *MaxJump, long *NumberJump, double *UsedTime, long *ErrorCode);

#endif

#ifdef CPLEXV
int    C_CRPoptimize(double*,double*);
int    C_CRPloadprob(int,double*,int,double*,int*,char*,double*,double*,double*,double*,double*,char**,int*,int*,signed char*,int);
int    C_CRPfreeprob(void);
int    C_CRPprintsolution(char*,char*);
int    C_CRPauditing(int);
int    C_CRPopenSOLVER(const char*);
int    C_CRPcloseSOLVER(void);
#endif
#ifdef SCIPV
int    S_CRPoptimize(double*,double*);
int    S_CRPloadprob(int,double*,int,double*,int*,char*,double*,double*,double*,double*,double*,char**,int*,int*,signed char*,int);
int    S_CRPfreeprob(void);
int    S_CRPprintsolution(char*,char*);
int    S_CRPauditing(int);
int    S_CRPopenSOLVER(void);
int    S_CRPcloseSOLVER(void);
#endif
#ifdef XPRESSV
int    X_CRPoptimize(double*,double*);
int    X_CRPloadprob(int,double*,int,double*,int*,char*,double*,double*,double*,double*,double*,char**,int*,int*,signed char*,int);
int    X_CRPfreeprob(void);
int    X_CRPprintsolution(char*,char*);
int    X_CRPauditing(int);
int    X_CRPopenSOLVER(void);
int    X_CRPcloseSOLVER(void);
#endif

#ifdef __cplusplus
}
#endif

