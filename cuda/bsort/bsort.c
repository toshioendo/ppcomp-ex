#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec-st.tv_sec)+(et.tv_usec-st.tv_usec)/1000000.0;
}

int init(double *data, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        data[i] = (double)rand() / RAND_MAX;
    }
    return 0;
}

int print(double *data, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        printf("%.4lf ", data[i]);
    }
    printf("\n");
    return 0;
}

/* sort data[0] ... data[N-1] */
void sort(double *data, int N)
{
    int N2 = 1;
    double INF = 1000000000.0; /* very large number */

    double *data2;
    int i, j, k, l, m;

    /* round up N to power of 2 */
    while(N2 < N) N2 <<= 1;
    /* allocate N2 size array */
    data2 = malloc(sizeof(double)*N2);

    /* Copy data[] to data2[] */
    for (i = 0; i < N; i++) data2[i] = data[i];
    for (i = N; i < N2; i++) data2[i] = INF;

    /* bitonic sort algorithm */
    for (i = 1; (1 << i) <= N2; i++) {
        for (j = i-1; j >= 0; j--) {
            int downbit = 1 << i;
            /* (k & downbit) == 0: ascending order (blue) */
            /* (k & downbit) != 0: descending order (green) */
            int dist = 1 << j;
            for (k = 0; k < N2; k++) {

                if ((k & dist) == 0) {
                    /* compare data2[k] and data2[k|dist] */
                    if ((k & downbit) == 0 && (data2[k] > data2[k|dist])) {
                        double tmp = data2[k];
                        data2[k] = data2[k|dist];
                        data2[k|dist] = tmp;
                    }
                    else if ((k & downbit) != 0 && (data2[k] < data2[k|dist])) {
                        double tmp = data2[k];
                        data2[k] = data2[k|dist];
                        data2[k|dist] = tmp;
                    }

                }
            }
        }
    }

    /* Copy data2[] to data[] */
    for(i = 0; i < N; i++) data[i] = data2[i];
    free(data2);
    return;
}

int check(double *data, int n)
{
    int i;
    int flag = 0;
    for (i = 0; i < n-1; i++) {
        if (data[i] > data[i+1]) {
            printf("Error: data[%d]=%.4lf, data[%d]=%.4lf\n",
                   i, data[i], i+1, data[i+1]);
            flag++;
        }
    }
    if (flag == 0) {
        printf("Data are sorted\n");
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int n = 10000;
    double *data;
    int i;

    if (argc >= 2) {
        n = atol(argv[1]);
    }

    data = malloc(sizeof(double)*n);

    for (i = 0; i < 5; i++) {
        struct timeval st;
        struct timeval et;
        double sec;

        init(data, n);
        /*print(data, n);*/
        gettimeofday(&st, NULL); /* get start time */
        sort(data, n);
        gettimeofday(&et, NULL); /* get start time */
        sec = time_diff_sec(st, et);

        printf("sorting %d data took %lf sec\n",
               n, sec);

        check(data, n);
        /*print(data, n);*/
    }

    free(data);

    return 0;
}
