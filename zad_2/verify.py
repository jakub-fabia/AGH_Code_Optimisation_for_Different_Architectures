#!/usr/bin/env python3
"""Prosty algorytm weryfikujacy poprawnosc zoptymalizowanych wersji.

Dla zadanego rozmiaru macierzy uruchamia wersje referencyjna (V1) oraz kazda
wersje zoptymalizowana na DOKLADNIE tej samej macierzy (srand(1) jest
deterministyczne), po czym porownuje wynikowe macierze element-po-elemencie.

Z powodu nielacznosci arytmetyki zmiennoprzecinkowej przegrupowanie operacji
(blokowanie, SIMD, FMA) zmienia najmlodsze bity wyniku, wiec porownanie
bit-w-bit jest zbyt restrykcyjne. Stosujemy blad wzgledny w normie maksimum:

    rel = max|opt - ref| / max|ref|

Wersja jest uznana za poprawna, gdy rel <= TOL. Dodatkowo porownywany jest
skalar Check (suma wszystkich elementow) zwracany przez program.

Uruchamiane przez `make verify` (czyli `uv run python verify.py`).
"""

import os
import subprocess
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
VERSIONS = ['1', '2', '3', '4', '5', '6', '7']
REF = '1'
SIZES = [200, 500, 1000]
# Tolerancja w normie maksimum. Margines 1e-6 odzwierciedla zle uwarunkowanie
# eliminacji Gaussa BEZ pivotingu na losowej macierzy (wspolczynnik wzrostu
# rosnie z n), przez co przegrupowanie operacji przez FMA (V7) zmienia wynik
# wzglednie o ~1e-8..1e-7. Wersje V2..V6 sa bit-w-bit identyczne z referencja.
TOL = 1.0e-6


def run(version, size, dump):
    """Uruchom wersje, zwroc skalar Check; macierz zrzucana do `dump`."""
    out = subprocess.run(
        [os.path.join(HERE, version, 'ge'), str(size), dump],
        capture_output=True, text=True, check=True,
    )
    for line in out.stdout.splitlines():
        if line.startswith('Check:'):
            return float(line.split(':', 1)[1])
    return None


def main():
    print(f'{"size":>6} {"ver":>4} {"max_abs":>12} {"rel_norm":>11} '
          f'{"check_rel":>11}  result')
    print('-' * 56)
    all_ok = True
    for size in SIZES:
        ref_dump = f'/tmp/ge_ref_{size}.bin'
        ref_check = run(REF, size, ref_dump)
        ref = np.fromfile(ref_dump, dtype=np.float64)
        scale = np.max(np.abs(ref))
        for v in VERSIONS:
            ver_dump = f'/tmp/ge_ver_{size}.bin'
            check = run(v, size, ver_dump)
            opt = np.fromfile(ver_dump, dtype=np.float64)
            max_abs = float(np.max(np.abs(opt - ref)))
            rel = max_abs / scale if scale > 0 else max_abs
            check_rel = abs(check - ref_check) / abs(ref_check) if ref_check else 0.0
            ok = rel <= TOL
            all_ok = all_ok and ok
            print(f'{size:>6} {("V" + v):>4} {max_abs:>12.3e} {rel:>11.3e} '
                  f'{check_rel:>11.3e}  {"OK" if ok else "FAIL"}')
        print('-' * 56)
    print(f'\nTolerancja rel_norm <= {TOL:.0e}')
    print('WSZYSTKIE WERSJE POPRAWNE' if all_ok else 'UWAGA: wykryto rozbieznosci')
    sys.exit(0 if all_ok else 1)


if __name__ == '__main__':
    main()
