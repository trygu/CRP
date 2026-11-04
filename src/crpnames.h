#ifndef TAUARGUS_CRP_NAMES_H
#define TAUARGUS_CRP_NAMES_H
/* CSP has identically named globals. Keep separate storage when both
 * engines are statically linked into one extension. Shared ABI is unchanged. */
#ifdef TAUARGUS_STATIC
#define ZERO TAUARGUS_CRP_ZERO
#define INF TAUARGUS_CRP_INF
#define MAX_TIME TAUARGUS_CRP_MAX_TIME
#define MIN_VIOLA TAUARGUS_CRP_MIN_VIOLA
#define MAX_SLACK TAUARGUS_CRP_MAX_SLACK
#endif
#endif
