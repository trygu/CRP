/****************************************************/
/* Controlled Rounding Problem for Linked Tables    */
/* in Statistical Disclosure Control (SDC)          */
/*                                                  */
/*                                                  */
/* CRPGLOB.h                                        */
/* Version 3.0.0                                    */
/* 20 May 2014                  jjsalaza@ull.es     */
/*                                                  */
/*                                                  */
/****************************************************/

#include "crpdefns.h"


/* Global variables for CRP */

int            CRPextended;   /* =0 if we are in the zero-restricted version
                                 =1 if we allow rounding to the second.... */
double         CRPbase;       /* base number for rounding             */
double         CRPobjval;     /* cost of the best output pattern      */
int            CRPfeasible;   /* =1 means that a solution was found   */
double         CRPlowerbound; /* LB for the cost of the best output pattern */
int            CRPclosednodes;      /* number of branch-and-bound nodes     */
int            CRPopenednodes;      /* number of branch-and-bound nodes     */
int            CRPlargerjump; 
int            CRPnumberlarger;
int            CRPabort;
double         basicOBJvalue,InitialTime,TotalTime,MaximumTime;

int            CRPncell;      /* number of cells after reduction      */
VARIABLE       *CRPcell;      /* potential variables                  */

int            CRPnsum;       /* number of sums after reduction       */
CONSTRAINT     *CRPsum;       /* potential constraints                */


double ZERO;           /* zero-epsilon                             */
double INF;            /* infinity                                 */
double MAX_TIME;       /* maximum total CPU time                   */
double MIN_VIOLA;      /* minimum violation for valid cuts         */
double MAX_SLACK;      /* maximum slack for cuts in the LP         */


#ifndef __WRAPCRP_H
// for statistic outputs, not to be used in WrapCRP.c

static float          CRPtime1=-1.0;
static float          CRPtime2=-1.0;
static float          CRPtime3=-1.0;
static int            CRPnodes;
static int            CRPaudit = -1;
#endif