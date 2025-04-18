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
    struct timeval st, st2, et2, et;
    long flop;
    double insec, compsec, outsec, sec;

    gettimeofday(&st, NULL);
#pragma acc data copyin(A[0:m*k],B[0:k*n]),copy(C[0:m*n])
    { // copyin (CPU -> GPU) here
        gettimeofday(&st2, NULL);
        
#pragma acc kernels
#pragma acc loop independent
        for (j = 0; j < n; j++) {
#pragma acc loop seq
            for (l = 0; l < k; l++) {
#pragma acc loop independent
                for (i = 0; i < m; i++) {
                    double ail = A[i+l*lda];
                    double blj = B[l+j*ldb];
                    C[i+j*ldc] += ail*blj;
                }
            }
        }

        gettimeofday(&et2, NULL);
    } // copyout (GPU -> CPU) here
    gettimeofday(&et, NULL);

    flop = 2.0*(double)m*(double)n*(double)k;
    insec = time_diff_sec(st, st2);
    compsec = time_diff_sec(st2, et2);
    outsec = time_diff_sec(et2, et);
    sec = time_diff_sec(st, et); // insec + compsec + outsec;
    printf("copyin: %lf sec, compute: %lf sec, copyout: %lf sec\n",
           insec, compsec, outsec);
    printf("Matmul took %lf sec --> %.3lf GFlops  (with data transfer)\n",
           sec, (double)flop/(double)sec/1e+9);
    printf("            %lf sec --> %.3lf GFlops  (without data transfer)\n",
           compsec, (double)flop/(double)compsec/1e+9);
    
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
        matmul();
    }

    free(A);
    free(B);
    free(C);
    return 0;
}
