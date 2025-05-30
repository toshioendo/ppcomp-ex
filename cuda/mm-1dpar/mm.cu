#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <cuda.h>
#include <cuda_runtime.h>

int m;
int n;
int k;
double *A;
double *B;
double *C;

double *DA;
double *DB;
double *DC;

#define BS (64)

double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec-st.tv_sec)+(et.tv_usec-st.tv_usec)/1000000.0;
}

__global__ void matmul_kernel(double *DA, double *DB, double *DC, int m, int n, int k)
{
    int i, j, l;
    i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= m) return; // do nothing

    int lda = m;
    int ldb = k;
    int ldc = m;

    for (j = 0; j < n; j++) {
        double cij = DC[i+j*ldc];
        for (l = 0; l < k; l++) {
            cij += DA[i+l*lda] * DB[l+j*ldb];
        }
        DC[i+j*ldc] = cij;
    }
}

int matmul()
{
    /* invoking (about) m threads */
    matmul_kernel<<<(m+BS-1)/BS, BS>>>(DA, DB, DC, m, n, k);
    return 0;
}

int main(int argc, char *argv[])
{
    int i, j;
    cudaError_t rc;

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

    /* allocate device memory */
    rc = cudaMalloc((void**)&DA, sizeof(double)*m*k);
    if (rc != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed\n"); exit(1);
    }
    rc = cudaMalloc((void**)&DB, sizeof(double)*k*n);
    if (rc != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed\n"); exit(1);
    }
    rc = cudaMalloc((void**)&DC, sizeof(double)*m*n);
    if (rc != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed\n"); exit(1);
    }

    /* Repeat same computation */
    for (i = 0; i < 5; i++) {
        struct timeval st, st2, et2, et;
        long flop;
        double insec, compsec, outsec, sec;

        gettimeofday(&st, NULL);

        /* copy input matrices from host to device */
        cudaMemcpy(DA, A, sizeof(double)*m*k, cudaMemcpyDefault);
        cudaMemcpy(DB, B, sizeof(double)*k*n, cudaMemcpyDefault);
        cudaMemcpy(DC, C, sizeof(double)*m*n, cudaMemcpyDefault);
        cudaDeviceSynchronize(); /* for precise time measurement */

        gettimeofday(&st2, NULL);

        /* computation */
        matmul();
        cudaDeviceSynchronize(); /* for precise time measurement */

        gettimeofday(&et2, NULL);

        /* copy output matrices from device to host */
        cudaMemcpy(C, DC, sizeof(double)*m*n, cudaMemcpyDefault);
        cudaDeviceSynchronize(); /* for precise time measurement */

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
    }

    cudaFree(DA);
    cudaFree(DB);
    cudaFree(DC);
    free(A);
    free(B);
    free(C);
    return 0;
}
