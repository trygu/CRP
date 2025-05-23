extern double ZERO;           /* zero-epsilon                             */
extern double INF;            /* infinity                                 */
extern double MAX_TIME;       /* maximum total CPU time                   */
extern double MIN_VIOLA;      /* minimum violation for valid cuts         */
extern double MAX_SLACK;      /* maximum slack for cuts in the LP         */
extern void (*CRPmessage)(double,double,int,int);
extern int  (*CRPextratime)(void);