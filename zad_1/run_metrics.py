#!/usr/bin/env python3
"""Sweep over (size x content-profile) configurations, run every compiled
version, and write aggregated timing + memory metrics into each dir's
metrics.txt. Inspired by how-to-optimize-gemm: vary the problem parameters
and record best / mean runtime per point so versions can be compared on the
same axes.

Run after `make all`. Invoked as `make metrics`.
"""

import os
import subprocess
import sys
import time
from statistics import mean

HERE = os.path.dirname(os.path.abspath(__file__))
VERSIONS = ['1', '2', '3', '4', '5', '6']
RUNS = 3

# (label, size_mb)
SIZES = [
    ('tiny',   1.0),
    ('small',  5.0),
    ('medium', 25.0),
    ('large',  50.0),
]

# (label, punct, dup, ctrl) — ratios of modifications injected by gen_file.py.
# baseline matches the default; the others exaggerate one axis at a time.
PROFILES = [
    ('baseline',    0.25, 0.20, 0.02),
    ('dup-heavy',   0.10, 0.60, 0.02),
    ('punct-heavy', 0.60, 0.10, 0.02),
    ('clean',       0.05, 0.02, 0.00),
]

TIME_FMT = '%e %M %P %U %S'
HEADER = (
    '# config         size_mb  punct  dup    ctrl   runs  best_s   mean_s   '
    'max_rss_kb  user_s  sys_s   cpu_pct  out_bytes\n'
)
ROW_FMT = (
    '{cfg:<15} {size:<8} {punct:<6} {dup:<6} {ctrl:<6} {runs:<5} '
    '{best:<8.3f} {mean:<8.3f} {rss:<11} {user:<7.3f} {sys:<7.3f} '
    '{cpu:<8} {out:<10}\n'
)


def regenerate(size_mb, punct, dup, ctrl):
    subprocess.run(
        ['python3', os.path.join(HERE, 'gen_file.py'),
         '--size-mb', str(size_mb),
         '--punct', str(punct),
         '--dup', str(dup),
         '--ctrl', str(ctrl)],
        check=True,
        stdout=subprocess.DEVNULL,
    )


def measure(version_dir, out_path):
    """Run ./base once under /usr/bin/time and return parsed metrics."""
    proc = subprocess.run(
        ['/usr/bin/time', '-f', TIME_FMT, './base'],
        cwd=version_dir,
        stdout=open(out_path, 'wb'),
        stderr=subprocess.PIPE,
        check=True,
    )
    line = proc.stderr.decode().strip().splitlines()[-1]
    wall, rss, cpu, user, sys_ = line.split()
    return {
        'wall': float(wall),
        'rss': int(rss),
        'cpu': cpu,
        'user': float(user),
        'sys': float(sys_),
        'out_bytes': os.path.getsize(out_path),
    }


def aggregate(runs):
    return {
        'best_wall': min(r['wall'] for r in runs),
        'mean_wall': mean(r['wall'] for r in runs),
        'rss': max(r['rss'] for r in runs),
        'user': mean(r['user'] for r in runs),
        'sys': mean(r['sys'] for r in runs),
        'cpu': runs[-1]['cpu'],
        'out_bytes': runs[-1]['out_bytes'],
    }


def main():
    for v in VERSIONS:
        base = os.path.join(HERE, v, 'base')
        if not os.path.exists(base):
            print(f'ERROR: {base} missing — run `make all` first.', file=sys.stderr)
            sys.exit(1)

    metrics_paths = {v: os.path.join(HERE, v, 'metrics.txt') for v in VERSIONS}
    timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
    for v, path in metrics_paths.items():
        with open(path, 'w') as f:
            f.write(f'# V{v} metrics — collected {timestamp}\n')
            f.write(f'# runs per config: {RUNS}\n')
            f.write(HEADER)

    total_points = len(SIZES) * len(PROFILES)
    point = 0
    for size_label, size_mb in SIZES:
        for prof_label, punct, dup, ctrl in PROFILES:
            point += 1
            cfg_label = f'{size_label}-{prof_label}'
            print(f'[{point:>2}/{total_points}] {cfg_label:<22} '
                  f'size={size_mb}MB punct={punct} dup={dup} ctrl={ctrl}',
                  flush=True)
            regenerate(size_mb, punct, dup, ctrl)

            for v in VERSIONS:
                version_dir = os.path.join(HERE, v)
                out_path = os.path.join(version_dir, '.metrics_stdout')
                runs = []
                for _ in range(RUNS):
                    runs.append(measure(version_dir, out_path))
                os.remove(out_path)
                agg = aggregate(runs)
                row = ROW_FMT.format(
                    cfg=cfg_label,
                    size=size_mb,
                    punct=punct,
                    dup=dup,
                    ctrl=ctrl,
                    runs=RUNS,
                    best=agg['best_wall'],
                    mean=agg['mean_wall'],
                    rss=agg['rss'],
                    user=agg['user'],
                    sys=agg['sys'],
                    cpu=agg['cpu'],
                    out=agg['out_bytes'],
                )
                with open(metrics_paths[v], 'a') as f:
                    f.write(row)
                print(f'    V{v}: best={agg["best_wall"]:.3f}s  '
                      f'rss={agg["rss"]}KB  out={agg["out_bytes"]}B',
                      flush=True)

    print('\nWrote metrics.txt in each version directory.')


if __name__ == '__main__':
    main()
