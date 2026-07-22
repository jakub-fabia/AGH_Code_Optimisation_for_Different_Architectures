#!/usr/bin/env python3
"""Sweep wydajnosci po rozmiarach macierzy dla kazdej wersji eliminacji Gaussa.

Inspirowane how-to-optimize-gemm: zmieniamy jeden parametr problemu (rozmiar
macierzy SIZE) i dla kazdego punktu zapisujemy najlepszy/sredni czas, GFLOP/s
oraz skalar Check (do szybkiej kontroli poprawnosci). Wyniki ladowane sa do
metrics.txt w katalogu kazdej wersji.

Uruchamiane przez `make metrics` (czyli `uv run python run_metrics.py`).
"""

import os
import subprocess
import sys
import time
from statistics import mean

HERE = os.path.dirname(os.path.abspath(__file__))
VERSIONS = ['1', '2', '3', '4', '5', '6', '7']
RUNS = 3
SIZES = [500, 1000, 1500, 2000]

HEADER = '# size   runs  best_s    mean_s    gflops    check\n'
ROW = '{size:<7} {runs:<5} {best:<9.4f} {mean:<9.4f} {gf:<9.3f} {check:.6e}\n'


def measure(version, size):
    out = subprocess.run(
        [os.path.join(HERE, version, 'ge'), str(size)],
        capture_output=True, text=True, check=True,
    )
    # Uwaga: program drukuje "call GE" bez znaku nowej linii, wiec etykieta
    # "Time:" nie zaczyna linii — szukamy tokenow w dowolnym miejscu.
    wall = gflops = check = None
    for line in out.stdout.splitlines():
        if 'GFLOP/s:' in line:
            gflops = float(line.split('GFLOP/s:', 1)[1])
        elif 'Time:' in line:
            wall = float(line.split('Time:', 1)[1])
        elif 'Check:' in line:
            check = float(line.split('Check:', 1)[1])
    return {'wall': wall, 'gflops': gflops, 'check': check}


def main():
    for v in VERSIONS:
        exe = os.path.join(HERE, v, 'ge')
        if not os.path.exists(exe):
            print(f'ERROR: {exe} brak — uruchom `make all` najpierw.', file=sys.stderr)
            sys.exit(1)

    paths = {v: os.path.join(HERE, v, 'metrics.txt') for v in VERSIONS}
    ts = time.strftime('%Y-%m-%d %H:%M:%S')
    for v, path in paths.items():
        with open(path, 'w') as f:
            f.write(f'# V{v} metryki — zebrane {ts}\n')
            f.write(f'# przebiegi na punkt: {RUNS}\n')
            f.write(HEADER)

    for size in SIZES:
        print(f'SIZE = {size}', flush=True)
        for v in VERSIONS:
            runs = [measure(v, size) for _ in range(RUNS)]
            best = min(r['wall'] for r in runs)
            mean_wall = mean(r['wall'] for r in runs)
            # GFLOP/s odpowiadajace najlepszemu czasowi.
            best_gf = max(r['gflops'] for r in runs)
            check = runs[-1]['check']
            with open(paths[v], 'a') as f:
                f.write(ROW.format(size=size, runs=RUNS, best=best,
                                   mean=mean_wall, gf=best_gf, check=check))
            print(f'    V{v}: best={best:.4f}s  {best_gf:.2f} GFLOP/s  '
                  f'check={check:.3e}', flush=True)

    print('\nZapisano metrics.txt w katalogu kazdej wersji.')


if __name__ == '__main__':
    main()
