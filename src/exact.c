/**********************************************************************************\
* Name:        exact.c                                         Tenerife 20/05/2014 *
* Purpose:     Test the CRP library                          .                     *
*              The program demonstrates the use of the global log callback.        *
\**********************************************************************************/

#include <stdio.h>
#include <stdlib.h>

extern int  do_round(char *Solver, char *InFileName, double base, double *UpperBound, double *LowerBound, int auditing, 
			char *SolutionFile, char* StatisticsFile, char *LicenseFile, char *LogFile,
			int MaxTime, int ZeroRestricted, char *NamePathExe, 
			double *MaxJump, int *NumberJump, double *UsedTime,
            int *ErrorCode);

void TAUmessage(lb,ub,nodecount,nodeleft)
double lb,ub;
int    nodecount,nodeleft;
{
//    printf("TAU message: lb=%lf , ub=%lf , nodecount=%d , nodeleft=%d\n",lb,ub,nodecount,nodeleft);
}

int TAUextratime()
{ 
//	printf("TAU message: asking for extra time? assuming 0.\n");
	return 0;
}


int  main(argc,argv)
int  argc;
char *argv[];
{
	double Base,UpperBound,LowerBound,MaxJump,UsedTime;
	int    MaxTime,NumberJump,ErrorCode,ZeroRestricted,audit;
	char   InFileName[100],SolutionFile[100],StatisticsFile[100],LicenseFile[100],LogFile[100];
    char   Solver[10];

    if( argc < 6 ){
         printf("Usage:  %s   file   base   auditing(0,1,2)   solver(CPLEX,SCIP,XPRESS)   outputfile\n",argv[0]);
         return(1);
    }

#if !defined CPLEXV && !defined XPRESSV && !defined SCIPV 
	printf(" Error : you need to link this code with an optimization solver\n");
	return 1;
#elif defined CPLEXV && !defined XPRESSV && !defined SCIPV
	printf(" Using CPLEX as optimization solver\n");
	strcpy( Solver , "CPLEX" );
#elif !defined CPLEXV && defined XPRESSV && !defined SCIPV
	printf(" Using XPRESS as optimization solver\n");
	strcpy( Solver , "XPRESS" );
#elif !defined CPLEXV && !defined XPRESSV && defined SCIPV
	printf(" Using SCIP as optimization solver\n");
	strcpy( Solver , "SCIP" );
#else
	strcpy( Solver , argv[4] );                 // option: CPLEX   or  SCIP   or  XPRESS
#endif


	strcpy( InFileName , argv[1] );             // Name of the file with the input data in JJ-format
    Base           = atof( argv[2] );			// Base number for rounding
    UpperBound     = 1E+20;                     // Overstimation of the optimal distance
    LowerBound     = 0;                         // Understimation of the optimal distance
    audit = atoi( argv[3] );                    // option: 0 means no rounding post-optimality phase is required
	strcpy( SolutionFile , argv[5] );           // file to save the protected table
	strcpy( StatisticsFile , "statistics.txt" );// file name to save statistics on the run
	strcpy( LicenseFile , "" );                 // file name containing the LP-solver license file
	strcpy( LogFile , "logfile.txt" );          // file name to display out messages during execution
    MaxTime        = 60;                        // Maximum time for the whole execution
    ZeroRestricted = 0;                         // option: 0 means zero-restricted solution is required
	MaxJump        = 0.0;                       // maximum number of perturbation of a cell over the nearest multiple of the base
	NumberJump     = 0;							// largest perturbation to a cell value
	UsedTime       = 0.0;						// overall computational time
	ErrorCode      = 0;							// exit status for t-ARGUS

    printf("File=%s Rounder=%s  base=%.1f  audit=%d\n",InFileName,Solver,Base,audit);

    CRPsetfunctions( &TAUmessage , &TAUextratime );
	do_round(Solver, InFileName, Base, &UpperBound, &LowerBound, audit, 
			SolutionFile, StatisticsFile, LicenseFile, NULL, // LogFile,
			MaxTime, ZeroRestricted, NULL,
            &MaxJump, &NumberJump, &UsedTime, &ErrorCode);

	return 0;
}




