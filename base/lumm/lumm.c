#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int n;
double *A;
double *B;
double *C;

double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec-st.tv_sec)+(et.tv_usec-st.tv_usec)/1000000.0;
}

int lumatmul()
{
    int i, j, l;
    // computataion should be done if (l <= i && l <= j)
    for (j = 0; j < n; j++) {
        for (l = 0; l <= j; l++) {
            double blj = B[l+j*n];
            for (i = l; i < n; i++) {
                double ail = A[i+l*n];
                C[i+j*n] += ail*blj;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int i, j;
    
    if (argc < 2) {
        printf("Specify N.\n");
        exit(1);
    }
    
    n = atoi(argv[1]);
    
    /* allocate matrix region */
    A = (double *)malloc(sizeof(double)*n*n);
    B = (double *)malloc(sizeof(double)*n*n);
    C = (double *)malloc(sizeof(double)*n*n);
    
    /* setup matrix (column major) */
    /* A is n*n lower-triangle matrix */
    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++) {
            double v = 0.0;
            if (i > j) { v = 2.0; }
            else if (i == j) { v = 1.0; }

            A[i+j*n] = v;
        }
    }
    /* B is n*n upper-triangle matrix */
    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++) {
            double v = 0.0;
            if (i <= j) { v = 5.0; }
            
            B[i+j*n] = v;
        }
    }
    /* C is n*n matrix */
    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++) {
            C[i+j*n] = 0.0;
        }
    }
    
    /* Repeat same computation for 5 times */
    for (i = 0; i < 5; i++) {
        struct timeval st;
        struct timeval et;
        double sec;
        
        gettimeofday(&st, NULL); /* get start time */
        lumatmul();
        gettimeofday(&et, NULL); /* get start time */
        
        sec = time_diff_sec(st, et);
        printf("Matmul took %lf sec --> %.3lf GFlops\n",
               sec, 2.0/3.0*(double)n*(double)n*(double)n/(double)sec/1e+9);
    }
    
    free(A);
    free(B);
    free(C);
    return 0;
}
