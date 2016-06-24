#include "RounderCtrl.h"
#include "ICallback.h"

///////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////
void RounderCtrl::FireUpdateLowerBound(double LowerBound)
{
    if (m_ProgressListener != NULL) {
        m_ProgressListener->UpdateLowerBound(LowerBound);
    }
}

void RounderCtrl::FireUpdateUpperBound(double UpperBound)
{
    if (m_ProgressListener != NULL) {
        m_ProgressListener->UpdateUpperBound(UpperBound);
    }    
}

void RounderCtrl::FireUpdateNumberClosedSubProb(long ClosedSubProb)
{
    if (m_ProgressListener != NULL) {
        m_ProgressListener->UpdateNumberClosedSubProb(ClosedSubProb);
    }        
}

void RounderCtrl::FireUpdateNumberOpenSubProb(long OpenSubProb)
{
    if (m_ProgressListener != NULL) {
        m_ProgressListener->UpdateNumberOpenSubProb(OpenSubProb);
    }        
}

void RounderCtrl::FireUpdateNumberFeasible(long Feasible)
{
    if (m_ProgressListener != NULL) {
        m_ProgressListener->UpdateNumberFeasible(Feasible);
    }            
}

void RounderCtrl::FireUpdateProgress(long Progress)
{
    if (m_ProgressListener != NULL) {
        m_ProgressListener->UpdateProgress(Progress);
    }            
}
