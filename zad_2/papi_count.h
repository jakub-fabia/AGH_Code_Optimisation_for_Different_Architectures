/* papi_count.h — pomiar licznikow sprzetowych wokol wywolania ge().
 *
 * Domyslnie (bez -DUSE_PAPI) wszystkie funkcje sa pustymi inline'ami, wiec
 * `make all`, `make verify` i `make metrics` budują się i działają bez PAPI.
 * Z flagą -DUSE_PAPI (cel `make papi`) liczone są:
 *   PAPI_FP_OPS   — liczba operacji zmiennoprzecinkowych (zwalidowana: na tym
 *                   CPU mapuje się na RETIRED_SSE_AVX_FLOPS, FMA liczone x2),
 *   PAPI_TOT_CYC  — cykle rdzenia,
 *   PAPI_TOT_INS  — instrukcje (do IPC).
 * Na ich podstawie raportujemy FLOP/cykl, IPC, zmierzone GFLOP/s oraz
 * WYSYCENIE jednostek FP = (FLOP/cykl) / 16, gdzie 16 FLOP/cykl to pułap
 * rdzenia Zen3+ (AVX2: 4 double x 2 porty FMA x 2 FLOP).
 */
#ifndef PAPI_COUNT_H
#define PAPI_COUNT_H

#ifdef USE_PAPI
#include <stdio.h>
#include <papi.h>

#define PC_PEAK_FLOP_PER_CYCLE 16.0  /* Zen3+ 1 rdzen, double, AVX2+FMA */

static int  _pc_es = PAPI_NULL;
static int  _pc_ok = 0;
static long long _pc_v[3];

static void pc_init(void)
{
  if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
    fprintf(stderr, "PAPI: library_init failed\n"); return;
  }
  if (PAPI_create_eventset(&_pc_es) != PAPI_OK) {
    fprintf(stderr, "PAPI: create_eventset failed\n"); return;
  }
  int ev[3];
  if (PAPI_event_name_to_code("PAPI_FP_OPS",  &ev[0]) != PAPI_OK ||
      PAPI_event_name_to_code("PAPI_TOT_CYC", &ev[1]) != PAPI_OK ||
      PAPI_event_name_to_code("PAPI_TOT_INS", &ev[2]) != PAPI_OK) {
    fprintf(stderr, "PAPI: event name lookup failed\n"); return;
  }
  if (PAPI_add_events(_pc_es, ev, 3) != PAPI_OK) {
    fprintf(stderr, "PAPI: add_events failed (zbyt wiele liczników?)\n"); return;
  }
  _pc_ok = 1;
}

static void pc_start(void) { if (_pc_ok) PAPI_start(_pc_es); }

static void pc_stop_report(double secs)
{
  if (!_pc_ok) { printf("PAPI: niedostepne \n"); return; }
  if (PAPI_stop(_pc_es, _pc_v) != PAPI_OK) { printf("PAPI: stop failed \n"); return; }
  double flop = (double) _pc_v[0];
  double cyc  = (double) _pc_v[1];
  double ins  = (double) _pc_v[2];
  printf("PAPI_FP_OPS: %.6e \n", flop);
  printf("PAPI_TOT_CYC: %.6e \n", cyc);
  printf("FLOP_per_cycle: %.4f \n", flop / cyc);
  printf("IPC: %.4f \n", ins / cyc);
  printf("Saturation_pct: %.2f \n", 100.0 * (flop / cyc) / PC_PEAK_FLOP_PER_CYCLE);
  printf("Measured_GFLOPs: %.4f \n", flop / secs / 1.0e9);
}

#else  /* bez PAPI — puste inline'y */
static inline void pc_init(void) {}
static inline void pc_start(void) {}
static inline void pc_stop_report(double s) { (void) s; }
#endif

#endif /* PAPI_COUNT_H */
