#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <omp.h>

double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec-st.tv_sec)+(et.tv_usec-st.tv_usec)/1000000.0;
}

double pi(int n)
{
    int i;
    double sum = 0.0;
    double dx = 1.0 / (double)n;

#pragma omp parallel
    {
        double local_sum = 0.0;
#pragma omp for
        for (i = 0; i < n; i++) {
            double x;
            double y;
            x = (double)i * dx;
            y = sqrt(1.0 - x*x);

            local_sum += dx*y;
        }

#pragma omp critical
        sum += local_sum;
    }

    return 4.0*sum;
}

int main(int argc, char *argv[])
{
    int n;
    int i;

    if (argc < 2) {
        printf("Specify #divisions.\n");
        exit(1);
    }

    n = atoi(argv[1]);

    /* Repeat same computation for 5 times */
    for (i = 0; i < 5; i++) {
        struct timeval st;
        struct timeval et;
        double sec;
        double res;

        gettimeofday(&st, NULL); /* get start time */
        res = pi(n);
        gettimeofday(&et, NULL); /* get end time */

        sec = time_diff_sec(st, et);
        printf("Result=%.15lf: Pi took %lf sec --> %.3lf Gsamples/sec\n",
               res, sec, (double)n/sec/1e+9);
    }
    return 0;
}
