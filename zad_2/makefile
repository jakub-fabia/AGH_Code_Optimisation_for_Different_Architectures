.PHONY: all verify metrics papi clean

CC = gcc

# Flagi kompilacji dobrane per-wersja (kazda wersja izoluje jeden krok
# optymalizacji; flagi SIMD wlaczane tam, gdzie sa potrzebne).
FLAGS_1 = -O2
FLAGS_2 = -O2
FLAGS_3 = -O2
FLAGS_4 = -O2
FLAGS_5 = -O2
FLAGS_6 = -O2 -msse3
FLAGS_7 = -O2 -mavx2 -mfma -march=native

all:
	$(CC) $(FLAGS_1) 1/ge.c -o 1/ge -lm
	$(CC) $(FLAGS_2) 2/ge.c -o 2/ge -lm
	$(CC) $(FLAGS_3) 3/ge.c -o 3/ge -lm
	$(CC) $(FLAGS_4) 4/ge.c -o 4/ge -lm
	$(CC) $(FLAGS_5) 5/ge.c -o 5/ge -lm
	$(CC) $(FLAGS_6) 6/ge.c -o 6/ge -lm
	$(CC) $(FLAGS_7) 7/ge.c -o 7/ge -lm

# Weryfikacja poprawnosci wzgledem wersji referencyjnej (V1).
verify: all
	@uv run python verify.py

# Sweep wydajnosci po rozmiarach macierzy.
metrics: all
	@uv run python run_metrics.py

# Pomiar licznikow sprzetowych (PAPI): rekompilacja z -DUSE_PAPI + -lpapi,
# nastepnie zebranie FLOP/cykl, IPC i wysycenia jednostek FP.
papi:
	$(CC) $(FLAGS_1) -DUSE_PAPI 1/ge.c -o 1/ge_papi -lpapi -lm
	$(CC) $(FLAGS_2) -DUSE_PAPI 2/ge.c -o 2/ge_papi -lpapi -lm
	$(CC) $(FLAGS_3) -DUSE_PAPI 3/ge.c -o 3/ge_papi -lpapi -lm
	$(CC) $(FLAGS_4) -DUSE_PAPI 4/ge.c -o 4/ge_papi -lpapi -lm
	$(CC) $(FLAGS_5) -DUSE_PAPI 5/ge.c -o 5/ge_papi -lpapi -lm
	$(CC) $(FLAGS_6) -DUSE_PAPI 6/ge.c -o 6/ge_papi -lpapi -lm
	$(CC) $(FLAGS_7) -DUSE_PAPI 7/ge.c -o 7/ge_papi -lpapi -lm
	@uv run python run_papi.py

clean:
	@for d in 1 2 3 4 5 6 7; do rm -f $$d/ge $$d/ge_papi $$d/metrics.txt; done
	@rm -f papi.txt /tmp/ge_ref_*.bin /tmp/ge_ver_*.bin
