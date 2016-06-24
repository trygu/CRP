#ifndef ROUNDERCTRL_H
#define	ROUNDERCTRL_H

#include "cplex.h"
#include "xprs.h"
#include "IProgressListener.h"
#include "ICallback.h"
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include "objscip/objscip.h"
#include <iostream>
#include <time.h>

#include "WrapCRP.h"

class RounderCtrl
{
private:
    IProgressListener* m_ProgressListener;
    void FireUpdateLowerBound(double LowerBound);
    void FireUpdateUpperBound(double UpperBound);
    void FireUpdateNumberClosedSubProb(long ClosedSubProb);
    void FireUpdateNumberOpenSubProb(long OpenSubProb);
    void FireUpdateNumberFeasible(long Feasible);
    void FireUpdateProgress(long Progress);
    
public:
    RounderCtrl()
    {
        m_ProgressListener = NULL;
        // Load from file the strings assigned to error-codes
	LoadErrorStrings(ErrorStrings);
    
	// Default settings for JJ-variables, in case of Cplex
	JJzero        = 1E-7;
	JJzero1       = 1E-7;
	JJzero2       = 1E-10;
	JJInf         = 2.14E9;
	JJmaxcolslp   = 50000;
	JJmaxrowslp   = 15000;
	JJmaxcutspool = 500000;
	JJmaxcutsiter = 50;
	JJminviola    = 0.0001;
	JJmaxslack    = 0.0001;

        JJFeasibilityTol = 1E-6;
        JJOptimalityTol = 1E-9;
    }
    
    ~RounderCtrl()
    {
    }
    
    void SetProgressListener(IProgressListener *ProgressListener);
    void SetCallback(ICallback *jCallback);

    void SetJJconstantsInt(const int ConstName, const long ConstValue);
    void SetJJconstantsDbl(const int ConstName, const double ConstValue);
    long GetJJconstantsInt(const int ConstName);
    double GetJJconstantsDbl(const int ConstName);
    std::string GetErrorString(int ErrorNumber);
    
    int TAUmessage(double lb,double ub,double rapid,int nodedone,int nodeleft);
    int DoRound(std::string InFileName, long Base, double *UpperBound, double *LowerBound, long audit, long StopRule, std::string OutSolutionFileName,
     	        std::string OutStatisticsFileName, std::string ilmfile, std::string LogFile, long maxtime, long ZeroRestricted, std::string NamePathExe, double *MaxJump,
                long *NumberJump, double *UsedTime, long *SolutionType, long *ErrorCode, bool *pVal);
};

#endif	/* HITASCTRL_H */


