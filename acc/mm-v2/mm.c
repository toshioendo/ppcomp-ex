#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int m;
int n;
int k;
double *A;
double *B;
double *C;

double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec-st.tv_sec)+(et.tv_usec-st.tv_usec)/1000000.0;
}

int matmul()
{
    int i, j, l;
    int lda = m;
    int ldb = k;
    int ldc = m;
#pragma acc data copyin(A[0:m*k],B[0:k*n]),copy(C[0:m*n])
#pragma acc kernels
#pragma acc loop independent gang,vector
    for (j = 0; j < n; j++) {
#pragma acc loop independent gang,vector
        for (i = 0; i < m; i++) {
            double cij = C[i+j*ldc];
#pragma acc loop seq
            for (l = 0; l < k; l++) {
                double blj = B[l+j*ldb];
                double ail = A[i+l*lda];
                cij += ail*blj;
            }
            C[i+j*ldc] = cij;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int i, j;

    if (argc < 4) {
        printf("Specify M, N, K.\n");
        exit(1);
    }

    m = atoi(argv[1]);
    n = atoi(argv[2]);
    k = atoi(argv[3]);

    /* allocate matrix region */
    A = (double *)malloc(sizeof(double)*m*k);
    B = (double *)malloc(sizeof(double)*k*n);
    C = (double *)malloc(sizeof(double)*m*n);

    /* setup matrix (column major) */
    /* A is m*k matrix */
    for (j = 0; j < k; j++) {
        for (i = 0; i < m; i++) {
            A[i+j*m] = 1.0;
        }
    }
    /* B is k*n matrix */
    for (j = 0; j < n; j++) {
        for (i = 0; i < k; i++) {
            B[i+j*k] = 10.0;
        }
    }
    /* C is m*n matrix */
    for (j = 0; j < n; j++) {
        for (i = 0; i < m; i++) {
            C[i+j*m] = 0.0;
        }
    }

    /* Repeat same computation for 5 times */
    for (i = 0; i < 5; i++) {
        struct timeval st;
        struct timeval et;
        double sec;

        gettimeofday(&st, NULL); /* get start time */
        matmul();
        gettimeofday(&et, NULL); /* get start time */

        sec = time_diff_sec(st, et);
        printf("Matmul took %lf sec --> %.3lf GFlops\n",
               sec, 2.0*(double)m*(double)n*(double)k/sec/1e+9);
    }

    free(A);
    free(B);
    free(C);
    return 0;
}
