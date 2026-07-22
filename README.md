# AGH Code Optimisation for Different Architectures

Coursework for the AGH UST course **Code Optimisation for Different Architectures**
(*Optymalizacja Kodu Numerycznego na Różne Architektury*, OKNRA).

The exercises explore how compiler flags, memory-access patterns, and SIMD
vectorisation affect the performance of numerical kernels on modern x86-64
hardware. Each part contains C/C++ implementations and a `Makefile` benchmark
harness; most parts also include a LaTeX report (`.tex`, in Polish).

## Layout

| Directory | Topic | Kernels | Report |
|-----------|-------|---------|--------|
| `lab_1/`  | Matrix multiplication — loop ordering & `-O2` | `mm1.c … mm9.c` | — |
| `lab_3/`  | Gaussian elimination — flag sweep (`-O2`, `-march=native`, `-mfma`, AVX intrinsics) | `ge1.c … ge8.c` | `lab3.tex` |
| `lab_4/`  | Cholesky factorization — up to AVX intrinsics (`chol6`) | `chol1.c … chol6.c` | — |
| `zad_1/`  | Profiling with `gprof` on a text-processing workload | `*/base.cpp` | `raport.tex` |
| `zad_2/`  | Gaussian elimination optimised in 7 stages + PAPI hardware counters | `1..7/ge.c` | `raport.tex` |

## Building & running

Each part is self-contained and driven by its `Makefile`.

```bash
# lab_1 — build normal + -O2 variants, then benchmark
cd lab_1 && make benchmark

# lab_3 — sweep all flag combinations into results.txt
cd lab_3 && make test SIZE=1500

# lab_4 — build and run all Cholesky versions (optionally with flags)
cd lab_4 && make test            # or: make test-all

# zad_2 — build, verify correctness, sweep metrics, PAPI counters
cd zad_2 && make verify && make metrics && make papi
```

`zad_1` and `zad_2` use Python helper scripts for driving benchmarks and
collecting metrics. `zad_2` pins its Python environment with
[uv](https://github.com/astral-sh/uv) (`pyproject.toml` + `uv.lock`); the
`Makefile` targets invoke `uv run` automatically.

Run `make clean` in any directory to remove build outputs and generated data.

## Notes

- Requires `gcc`/`g++` with AVX support (`lab_3`, `lab_4`, `zad_2`) and, for the
  PAPI targets, the [PAPI](https://icl.utk.edu/papi/) library (`-lpapi`).
- Compiled binaries, `gmon.out`, the large generated `zad_1/*/file.txt`
  (~52 MB each, regenerate via `zad_1/gen_file.py`), and other run artifacts are
  git-ignored — see `.gitignore`.
