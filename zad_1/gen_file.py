#!/usr/bin/env python3
"""Generate a deterministic file.txt of configurable size and content mix.

Used both as a one-shot generator and as a building block for run_metrics.py.
Usage:
    gen_file.py [--size-mb N] [--punct P] [--dup D] [--ctrl C] [--target DIR]
"""

import argparse
import os
import random
import shutil

VERSIONS = ['1', '2', '3', '4', '5', '6']

WORDS = [
    'hello', 'world', 'foo', 'bar', 'baz', 'lorem', 'ipsum', 'dolor', 'sit',
    'amet', 'consectetur', 'adipiscing', 'elit', 'sed', 'eiusmod', 'tempor',
    'incididunt', 'labore', 'magna', 'aliqua', 'quick', 'brown', 'fox',
    'jumps', 'over', 'lazy', 'dog', 'python', 'program', 'data', 'text',
    'alpha', 'beta', 'gamma', 'delta', 'epsilon', 'zeta', 'eta', 'theta',
]

PUNCT = ['.', ',', ';', ':', '!', '?', '-', '"', '(', ')', "'"]
WHITESPACE = [' ', '  ', '\t', '\n', ' \n', '\n\n', '\t ', ' ', ' ', ' ']
CONTROL_BYTES = [b'\x01', b'\x02', b'\x07', b'\x0b', b'\x0e', b'\xff', b'\x80']


def random_word():
    w = random.choice(WORDS)
    r = random.random()
    if r < 0.15:
        return w.upper()
    if r < 0.40:
        return w.capitalize()
    return w


def make_chunk(punct_rate, dup_rate, ctrl_rate):
    parts = []
    for _ in range(random.randint(50, 150)):
        w = random_word().encode('ascii')
        parts.append(w)
        if random.random() < dup_rate:
            parts.append(w)
        if random.random() < punct_rate:
            parts.append(random.choice(PUNCT).encode('ascii'))
        parts.append(random.choice(WHITESPACE).encode('ascii'))
        if random.random() < ctrl_rate:
            parts.append(random.choice(CONTROL_BYTES))
    return b''.join(parts)


def generate(size_bytes, punct_rate, dup_rate, ctrl_rate, target_dir):
    random.seed(42)
    pool = [make_chunk(punct_rate, dup_rate, ctrl_rate) for _ in range(500)]

    primary = os.path.join(target_dir, '1', 'file.txt')
    written = 0
    with open(primary, 'wb') as f:
        while written < size_bytes:
            chunk = random.choice(pool)
            f.write(chunk)
            written += len(chunk)

    for d in VERSIONS[1:]:
        shutil.copy(primary, os.path.join(target_dir, d, 'file.txt'))
    return written


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--size-mb', type=float, default=50.0)
    ap.add_argument('--punct', type=float, default=0.25)
    ap.add_argument('--dup', type=float, default=0.20)
    ap.add_argument('--ctrl', type=float, default=0.02)
    ap.add_argument('--target', default=os.path.dirname(os.path.abspath(__file__)))
    args = ap.parse_args()

    size_bytes = int(args.size_mb * 1024 * 1024)
    written = generate(size_bytes, args.punct, args.dup, args.ctrl, args.target)
    print(f'Generated {written} bytes ({written / (1024 * 1024):.2f} MB) '
          f'[punct={args.punct} dup={args.dup} ctrl={args.ctrl}]')


if __name__ == '__main__':
    main()
