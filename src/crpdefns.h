/****************************************************/
/* Controlled Rounding Problem for Linked Tables    */
/* in Statistical Disclosure Control (SDC)          */
/*                                                  */
/*                                                  */
/* CRPDEFNS.h                                       */
/* Version 3.0.0                                    */
/* 20 May 2014                  jjsalaza@ull.es     */
/*                                                  */
/*                                                  */
/****************************************************/


#define JJZERO          101
#define JJINF           102
#define JJMINVIOLA      103
#define JJMAXSLACK      104
#define JJMAXTIME	105


typedef struct{
    double nominal;           /* nominal value                             */
    double nominaldown;       /* nominal value rounded down                */
    char*  name;              /* cell name                                 */
    int    index;             /* position in order                         */
    double lvalue;            /* lower possible value                      */
    double uvalue;            /* upper possible value                      */
    double weight;            /* importance of unmodifing the cell value   */
    double lpl,upl,spl;
    char   status;            /* ='s' for safe and ='u' for unsafe         */
    double solution;          /* jumps of "base" respecto to "nominaldown" */
} VARIABLE;


typedef struct CON{
    int      card;            /* number of non-zero elements               */
    VARIABLE **stack;         /* non-zero variables                        */
    double   *coef;           /* non-zero coefficients                     */
    double   rhs;             /* right-hand-side of the constraint         */
    char     sense;           /* 'G' , 'L'  or   'E'                       */
    long int hash;            /* hash number for the row identification    */
} CONSTRAINT;




