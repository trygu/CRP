/****************************************************/
/* Controlled Rounding Problem for Linked Tables    */
/* in Statistical Disclosure Control (SDC)          */
/*                                                  */
/*                                                  */
/* CRPMAIN.h                                        */
/* Version 3.0.0                                    */
/* 20 May 2014                  jjsalaza@ull.es     */
/*                                                  */
/*                                                  */
/****************************************************/
#include "crpglob.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
    #define EXPORTFUNC __declspec(dllexport)
#else
    #define EXPORTFUNC __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif 

#ifdef CPLEXV
EXPORTFUNC int    C_CRPoptimize(double*,double*);
EXPORTFUNC int    C_CRPloadprob(int,double*,int,double*,int*,char*,double*,double*,double*,double*,double*,char**,int*,int*,signed char*,int);
EXPORTFUNC int    C_CRPfreeprob(void);
EXPORTFUNC int    C_CRPprintsolution(char*,char*);
EXPORTFUNC int    C_CRPauditing(int);
EXPORTFUNC int    C_CRPopenSOLVER(const char*);
EXPORTFUNC int    C_CRPcloseSOLVER(void);
#endif
#ifdef SCIPV
EXPORTFUNC int    S_CRPoptimize(double*,double*);
EXPORTFUNC int    S_CRPloadprob(int,double*,int,double*,int*,char*,double*,double*,double*,double*,double*,char**,int*,int*,signed char*,int);
EXPORTFUNC int    S_CRPfreeprob(void);
EXPORTFUNC int    S_CRPprintsolution(char*,char*);
EXPORTFUNC int    S_CRPauditing(int);
EXPORTFUNC int    S_CRPopenSOLVER(void);
EXPORTFUNC int    S_CRPcloseSOLVER(void);
#endif
#ifdef XPRESSV
EXPORTFUNC int    X_CRPoptimize(double*,double*);
EXPORTFUNC int    X_CRPloadprob(int,double*,int,double*,int*,char*,double*,double*,double*,double*,double*,char**,int*,int*,signed char*,int);
EXPORTFUNC int    X_CRPfreeprob(void);
EXPORTFUNC int    X_CRPprintsolution(char*,char*);
EXPORTFUNC int    X_CRPauditing(int);
EXPORTFUNC int    X_CRPopenSOLVER(const char*);
EXPORTFUNC int    X_CRPcloseSOLVER(void);
#endif

EXPORTFUNC void (*CRPmessage)(double,double,int,int);

EXPORTFUNC int  (*CRPextratime)(void);

//EXPORTFUNC double ZERO;           /* zero-epsilon                             */
//EXPORTFUNC double INF;            /* infinity                                 */
//EXPORTFUNC double MAX_TIME;       /* maximum total CPU time                   */
//EXPORTFUNC double MIN_VIOLA;      /* minimum violation for valid cuts         */
//EXPORTFUNC double MAX_SLACK;      /* maximum slack for cuts in the LP         */

#ifdef __cplusplus
}
#endif