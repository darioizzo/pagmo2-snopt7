#include <stdlib.h>
#include <time.h>

#include "snopt7_c.h"


inline double closed_interval_rand(double x0, double x1)
{
    return x0 + (x1 - x0) * rand() / ((double)RAND_MAX);
}

__PAGMO_VISIBLE void snInit(snProblem_76 *prob, char *name, char *prtfile, int summOn){};

__PAGMO_VISIBLE int setIntParameter(snProblem_76 *prob, char stropt[], int opt)
{
    char *invalid;
    invalid = "invalid_integer_option";
    if (strcmp(stropt, invalid) == 0) {
        return 1;
    } else {
        return 0;
    }
};

__PAGMO_VISIBLE int setRealParameter(snProblem_76 *prob, char stropt[], double opt)
{
    char *invalid;
    invalid = "invalid_numeric_option";
    if (strcmp(stropt, invalid) == 0) {
        return 1;
    } else {
        return 0;
    }
};

__PAGMO_VISIBLE void deleteSNOPT(snProblem_76 *prob){};

// The following routine fakes the snOptA interface and generates 100 random vectors. It will not touch the input
// decision vector. We use this implementation to test since the true library is commercial
__PAGMO_VISIBLE int solveA(snProblem_76 *prob, int start, int nF, int n, double ObjAdd, int ObjRow, snFunA usrfun, int neA, int *iAfun,
           int *jAvar, double *A, int neG, int *iGfun, int *jGvar, double *xlow, double *xupp, double *Flow,
           double *Fupp, double *x, int *xstate, double *xmul, double *F, int *Fstate, double *Fmul, int *nS, int *nInf,
           double *sInf)
{
    int retval = 1;
    double *x_new
        = malloc(sizeof(double)
                 * n); // C99 does have variable-length arrays (VLAs), but Microsoft's compiler doesn't support them.
    int Status = 0;
    int needF = 1;
    int needG = 1;
    char cu[1];
    int lencu = 0;
    double *G = malloc(sizeof(double) * nF * n);
    srand((unsigned int)(time(NULL)));

    int i, j;
    for (i = 0; i < 100; ++i) {
        // Random vector
        for (j = 0; j < n; ++j) {
            x_new[j] = closed_interval_rand(xlow[j], xupp[j]);
        }
        // Call usrfun (will call both fitness and gradient)
        usrfun(&Status, &n, x_new, &needF, &nF, F, &needG, &neG, G, cu, &lencu, prob->iu, &(prob->leniu), prob->ru,
               &(prob->lenru));
        if (Status < 0) {
            retval = 71;
            break;
        }
    }
    free(x_new);
    free(G);
    return retval;
};
