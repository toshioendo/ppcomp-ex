#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

int main(int argc, char *argv[])
{
    int i;
  printf("Hello OpenMP World\n");

//omp_set_num_threads(2);

  printf("Outside of parallel region. I'm %d-th thread out of %d threads\n",
	     omp_get_thread_num(), omp_get_num_threads());

#pragma omp parallel private(i)
  {
    for (i = 0; i < 5; i++) {
      printf("I'm %d-th thread out of %d threads, i=%d (&i=%p)\n",
	     omp_get_thread_num(), omp_get_num_threads(),i, &i);
      sleep(1);
      //sleep(omp_get_thread_num());

      omp_set_num_threads(i);
    
    }
  }

  printf("Good Bye OpenMP World\n");
  printf("Outside of parallel region. I'm %d-th thread out of %d threads\n",
	     omp_get_thread_num(), omp_get_num_threads());

  return 0;
}
