#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "../papi_count.h"

#define IDX(i, j) ((i) * SIZE + (j))

static double gtod_ref_time_sec = 0.0;

double ge_flop(int SIZE)
{
    double n = (double) SIZE;
    return n * (n - 1.0) / 2.0 + (n - 1.0) * n * (2.0 * n - 1.0) / 3.0;
}

double dclock()
{
  double the_time, norm_sec;
  struct timeval tv;
  gettimeofday( &tv, NULL );
  if ( gtod_ref_time_sec == 0.0 )
    gtod_ref_time_sec = ( double ) tv.tv_sec;
  norm_sec = ( double ) tv.tv_sec - gtod_ref_time_sec;
  the_time = norm_sec + tv.tv_usec * 1.0e-6;
  return the_time;
}

int ge(double *A, int SIZE)
{
  register int i, j, k;
  register double multiplier;
  for (k = 0; k < SIZE; k++) {
    for (i = k + 1; i < SIZE; i++) {
      multiplier = A[IDX(i, k)] / A[IDX(k, k)];
      for (j = k + 1; j < SIZE; j++) {
        A[IDX(i, j)] = A[IDX(i, j)] - A[IDX(k, j)] * multiplier;
      }
    }
  }
  return 0;
}

int main(int argc, const char *argv[])
{
  int i, j, iret;
  double dtime;
  int SIZE = atoi(argv[1]);

  double *matrix = malloc((size_t) SIZE * SIZE * sizeof(double));
  srand(1);
  for (i = 0; i < SIZE; i++)
    for (j = 0; j < SIZE; j++)
      matrix[IDX(i, j)] = rand();

  pc_init();
  printf("call GE");
  pc_start();
  dtime = dclock();
  iret = ge(matrix, SIZE);
  dtime = dclock() - dtime;
  pc_stop_report(dtime);
  printf("Time: %le \n", dtime);
  printf("FLOP: %le \n", ge_flop(SIZE));
  printf("GFLOP/s: %le \n", ge_flop(SIZE) / dtime / 1.0e9);

  double check = 0.0;
  for (i = 0; i < SIZE; i++)
    for (j = 0; j < SIZE; j++)
      check = check + matrix[IDX(i, j)];
  printf("Check: %le \n", check);

  if (argc >= 3) {
    FILE *f = fopen(argv[2], "wb");
    fwrite(matrix, sizeof(double), (size_t) SIZE * SIZE, f);
    fclose(f);
  }

  fflush(stdout);
  return iret;
}
