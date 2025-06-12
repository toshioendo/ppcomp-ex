#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <string.h>

int m;
int n; /* The number of columns of global B/C */
int ln; /* The number of columns of process local B/C */
int k;


double *A;
double *B; /* Used only in rank 0 */
double *C; /* Used only in rank 0 */

double *LB;
double *LC;

double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec-st.tv_sec)+(et.tv_usec-st.tv_usec)/1000000.0;
}

/* returns start/end point of the rank-th process */
/* results are put into *ps and *pe. */
/* rank should be 0 <= rank = nprocs */
void divide_length(int len, int rank, int nprocs, int *ps, int *pe)
{
    int llen = (len+nprocs-1)/nprocs;
    int s = llen*rank;
    int e = llen*(rank+1);

    if (s > len) s = len;
    if (e > len) e = len;

    *ps = s;
    *pe = e;
    return;
}

int matmul()
{
    int i, j, l;
    int lda = m;
    int ldb = k;
    int ldc = m;

    for (j = 0; j < ln; j++) {
        for (l = 0; l < k; l++) {
            double blj = LB[l+j*ldb];
            for (i = 0; i < m; i++) {
                double ail = A[i+l*lda];
                LC[i+j*ldc] += ail*blj;
            }
        }
    }
    return 0;
}

/* send data from rank 0 to other processes */
int comm1(int rank, int nprocs)
{
    int r;
    MPI_Status stat;
    /* A */
    if (rank == 0) {
        /* send entire A */
        for (r = 1; r < nprocs; r++) {
            MPI_Send(A, m*k, MPI_DOUBLE, r, 0, MPI_COMM_WORLD);
        }
    }
    else {
        /* receive A */
        MPI_Recv(A, m*k, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &stat);
    }

    /* B, C */
    if (rank == 0) {
        {
            int s, e;
            divide_length(n, rank, nprocs, &s, &e);
            /* on rank 0, partial B, C are copied to LB, LC */
            memcpy(LB, &B[0], k*(e-s) * sizeof(double));
            memcpy(LC, &C[0], m*(e-s) * sizeof(double));
        }
                   
        for (r = 1; r < nprocs; r++) {
            int s, e;
            divide_length(n, r, nprocs, &s, &e);
            /* rank r takes [s, e) */

            /* send partial B to rank p */
            MPI_Send(&B[k*s], k*(e-s), MPI_DOUBLE, r, 0, MPI_COMM_WORLD);
            /* send partial C to rank p */
            MPI_Send(&C[n*s], m*(e-s), MPI_DOUBLE, r, 0, MPI_COMM_WORLD);
        }
    }
    else {
        int s, e;
        divide_length(n, rank, nprocs, &s, &e);
        /* I take [s, e) */

        /* receive partial B (LB) from rank 0 */
        MPI_Recv(LB, k*(e-s), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &stat);
        /* receive partial C (LC) from rank 0 */
        MPI_Recv(LC, m*(e-s), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &stat);
    }

    return 0;
}

/* send matrix C from other processes to rank 0 */
int comm2(int rank, int nprocs)
{
    int r;
    MPI_Status stat;

    /*  C */
    if (rank == 0) {
        {
            int s, e;
            divide_length(n, rank, nprocs, &s, &e);
            /* on rank 0, LC is copied to partial C */
            memcpy(&C[0], LC, m*(e-s) * sizeof(double));
        }

        for (r = 1; r < nprocs; r++) {
            int s, e;
            divide_length(n, r, nprocs, &s, &e);
            /* rank r takes [s, e) */

            /* receive partial C from rank r */
            MPI_Recv(&C[n*s], m*(e-s), MPI_DOUBLE, r, 0, MPI_COMM_WORLD, &stat);
        }
    }
    else {
        int s, e;
        divide_length(n, rank, nprocs, &s, &e);
        /* I take [s, e) */

        /* send partial C (LC) to rank 0 */
        MPI_Send(LC, m*(e-s), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int i, j;
    int rank, nprocs;

    MPI_Init(&argc, &argv);

    if (argc < 4) {
        printf("Specify M, N, K.\n");
        MPI_Finalize();
        exit(1);
    }

    m = atoi(argv[1]);
    n = atoi(argv[2]);
    k = atoi(argv[3]);

    /* get the rank of this process */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    {
        int s, e;
        /* calculates the number of columns of process local B/C */
        divide_length(n, rank, nprocs, &s, &e);
        printf("rank %d takes [%d,%d)\n", rank, s, e);
        ln = e-s;
    }

    /* allocate matrix region */
    A = (double *)malloc(sizeof(double)*m*k);

    if (rank == 0) {
        B = (double *)malloc(sizeof(double)*k*n);
        C = (double *)malloc(sizeof(double)*m*n);
    }
    
    if (ln > 0) {
        LB = (double *)malloc(sizeof(double)*k*ln);
        LC = (double *)malloc(sizeof(double)*m*ln);
    }
    else {
        LB = NULL;
        LC = NULL;
    }

    /* setup matrix (column major) only in rank 0 */
    if (rank == 0) {
        /* A is m*k matrix */
        for (j = 0; j < k; j++) {
            for (i = 0; i < m; i++) {
                A[i+j*m] = 1.0;
            }
        }
        /* LB is k*ln matrix */
        for (j = 0; j < n; j++) {
            for (i = 0; i < k; i++) {
                B[i+j*k] = 10.0;
            }
        }
        /* LC is m*ln matrix */
        for (j = 0; j < n; j++) {
            for (i = 0; i < m; i++) {
                C[i+j*m] = 0.0;
            }
        }
    }

    /* Repeat same computation for 5 times */
    for (i = 0; i < 5; i++) {
        struct timeval st, st2, et2, et;
        long flop;
        double comm1sec, compsec, comm2sec, sec;

        MPI_Barrier(MPI_COMM_WORLD);
        gettimeofday(&st, NULL); /* get start time */

        comm1(rank, nprocs);
        
        MPI_Barrier(MPI_COMM_WORLD);
        gettimeofday(&st2, NULL); /* get start time */
        
        matmul();

        MPI_Barrier(MPI_COMM_WORLD);
        gettimeofday(&et2, NULL); /* get start time */

        comm2(rank, nprocs);
        
        MPI_Barrier(MPI_COMM_WORLD);
        gettimeofday(&et, NULL); /* get start time */

        if (rank == 0) {
            flop = 2.0*(double)m*(double)n*(double)k;
            comm1sec = time_diff_sec(st, st2);
            compsec = time_diff_sec(st2, et2);
            comm2sec = time_diff_sec(et2, et);
            sec = time_diff_sec(st, et); // comm1sec + compsec + comm2sec;
            printf("comm1: %lf sec, compute: %lf sec, comm2: %lf sec\n",
                   comm1sec, compsec, comm2sec);
            printf("Matmul took %lf sec --> %.3lf GFlops  (with communication)\n",
                   sec, (double)flop/(double)sec/1e+9);
            printf("            %lf sec --> %.3lf GFlops  (without communication)\n",
                   compsec, (double)flop/(double)compsec/1e+9);
        }
    }

    free(A);
    if (rank == 0) {
        free(B);
        free(C);
    }
    if (ln > 0) {
        free(LB);
        free(LC);
    }

    MPI_Finalize();
    return 0;
}
