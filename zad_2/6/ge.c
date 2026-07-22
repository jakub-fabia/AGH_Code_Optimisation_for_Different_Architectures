#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <emmintrin.h>
#include "../papi_count.h"

#define NR 8
#define MC 128
#define NC 256

#define IDX(i, j, n) ((i) * (n) + (j))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

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

static void PackPivotRow(int n, const double *src, double *dst)
{
  for (int j = 0; j < n; j++)
    dst[j] = src[j];
}

static void PackMultipliers(int m, const double *A, int SIZE,
                            int row0, int k, double pivot, double *mult)
{
  for (int i = 0; i < m; i++)
    mult[i] = A[IDX(row0 + i, k, SIZE)] / pivot;
}

static void UpdateKernel(int m, int n, double *C, int ldc,
                         const double *pivot_row, const double *mult)
{
  int i, j;
  for (j = 0; j + NR <= n; j += NR) {
    __m128d p01 = _mm_loadu_pd(pivot_row + j);
    __m128d p23 = _mm_loadu_pd(pivot_row + j + 2);
    __m128d p45 = _mm_loadu_pd(pivot_row + j + 4);
    __m128d p67 = _mm_loadu_pd(pivot_row + j + 6);
    for (i = 0; i < m; i++) {
      double *c = C + i * ldc + j;
      __m128d mi = _mm_set1_pd(mult[i]);
      _mm_storeu_pd(c,     _mm_sub_pd(_mm_loadu_pd(c),     _mm_mul_pd(p01, mi)));
      _mm_storeu_pd(c + 2, _mm_sub_pd(_mm_loadu_pd(c + 2), _mm_mul_pd(p23, mi)));
      _mm_storeu_pd(c + 4, _mm_sub_pd(_mm_loadu_pd(c + 4), _mm_mul_pd(p45, mi)));
      _mm_storeu_pd(c + 6, _mm_sub_pd(_mm_loadu_pd(c + 6), _mm_mul_pd(p67, mi)));
    }
  }
  for (; j < n; j++) {
    register double pj = pivot_row[j];
    for (i = 0; i < m; i++)
      C[i * ldc + j] -= pj * mult[i];
  }
}

int ge(double *A, int SIZE)
{
  int k, ii, jj;
  double packed_pivot[NC];
  double packed_mult[MC];

  for (k = 0; k < SIZE; k++) {
    double pivot = A[IDX(k, k, SIZE)];
    for (ii = k + 1; ii < SIZE; ii += MC) {
      int ib = MIN(MC, SIZE - ii);
      PackMultipliers(ib, A, SIZE, ii, k, pivot, packed_mult);
      for (jj = k + 1; jj < SIZE; jj += NC) {
        int jb = MIN(NC, SIZE - jj);
        PackPivotRow(jb, &A[IDX(k, jj, SIZE)], packed_pivot);
        UpdateKernel(ib, jb, &A[IDX(ii, jj, SIZE)], SIZE,
                     packed_pivot, packed_mult);
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
      matrix[IDX(i, j, SIZE)] = rand();

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
      check = check + matrix[IDX(i, j, SIZE)];
  printf("Check: %le \n", check);

  if (argc >= 3) {
    FILE *f = fopen(argv[2], "wb");
    fwrite(matrix, sizeof(double), (size_t) SIZE * SIZE, f);
    fclose(f);
  }

  fflush(stdout);
  return iret;
}
