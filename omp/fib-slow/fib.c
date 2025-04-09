#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec-st.tv_sec)+(et.tv_usec-st.tv_usec)/1000000.0;
}

long fib_r(int n)
{
    long f1, f2;
    if (n <= 1) return n;

#pragma omp task shared(f1)
    f1 = fib_r(n-1);
    
#pragma omp task shared(f2)
    f2 = fib_r(n-2);

#pragma omp taskwait

    return f1+f2;
}

long fib(int n)
{
    long ans;
#pragma omp parallel
#pragma omp single
    {
        ans = fib_r(n);
    }

    return ans;
}

int main(int argc, char *argv[])
{
    int n = 30;
    long ans;
    int i;

    if (argc >= 2) {
        n = atoi(argv[1]);
    }

    for (i = 0; i < 5; i++) {
        struct timeval st;
        struct timeval et;
        double sec;
        double res;

        gettimeofday(&st, NULL); /* get start time */
        ans = fib(n);
        gettimeofday(&et, NULL); /* get start time */
        sec = time_diff_sec(st, et);

        printf("fib(%d) = %ld: fib took %lf sec\n",
               n, ans, sec);

    }

    return 0;
}
