#!/usr/bin/env python3
"""Zbiera liczniki sprzetowe (PAPI) dla kazdej wersji eliminacji Gaussa.

Wymaga binariow zbudowanych z -DUSE_PAPI (cel `make papi`, ktory wola ten
skrypt). Dla dwoch rozmiarow --- jednego mieszczacego sie w L3 (n=1000) i
jednego przekraczajacego L3 (n=2000) --- uruchamia wersje i parsuje
zmierzone FLOP-y, cykle, FLOP/cykl, IPC oraz WYSYCENIE jednostek FP
(= FLOP/cykl / 16). Wynik trafia do papi.txt.
"""

import os
import subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
VERSIONS = ['1', '2', '3', '4', '5', '6', '7']
SIZES = [1000, 2000]

KEYS = ('PAPI_FP_OPS', 'PAPI_TOT_CYC', 'FLOP_per_cycle', 'IPC',
        'Saturation_pct', 'Measured_GFLOPs')


def run(version, size):
    out = subprocess.run(
        [os.path.join(HERE, version, 'ge_papi'), str(size)],
        capture_output=True, text=True, check=True,
    )
    d = {}
    for line in out.stdout.splitlines():
        for k in KEYS:
            if (k + ':') in line:
                d[k] = float(line.split(k + ':', 1)[1])
    if 'niedostepne' in out.stdout:
        d['unavailable'] = True
    return d


def main():
    lines = []
    hdr = (f'{"ver":>4} {"n":>5} {"FP_OPS":>12} {"cycles":>12} '
           f'{"FLOP/cyc":>9} {"IPC":>6} {"sat%":>7} {"GFLOP/s":>9}')
    lines.append('# Liczniki sprzetowe PAPI (PAPI_FP_OPS, PAPI_TOT_CYC, PAPI_TOT_INS)')
    lines.append('# Pulap FP rdzenia Zen3+: 16 FLOP/cykl (AVX2: 4 double x 2 FMA x 2)')
    lines.append('# sat% = (FLOP/cykl) / 16')
    lines.append(hdr)
    lines.append('-' * len(hdr))

    for size in SIZES:
        for v in VERSIONS:
            d = run(v, size)
            if d.get('unavailable'):
                lines.append(f'  V{v} {size:>5}   PAPI niedostepne')
                continue
            lines.append(
                f'  V{v} {size:>5} {d["PAPI_FP_OPS"]:>12.3e} {d["PAPI_TOT_CYC"]:>12.3e} '
                f'{d["FLOP_per_cycle"]:>9.3f} {d["IPC"]:>6.2f} {d["Saturation_pct"]:>7.2f} '
                f'{d["Measured_GFLOPs"]:>9.2f}')
        lines.append('-' * len(hdr))

    text = '\n'.join(lines)
    print(text)
    with open(os.path.join(HERE, 'papi.txt'), 'w') as f:
        f.write(text + '\n')
    print('\nZapisano papi.txt')


if __name__ == '__main__':
    main()
