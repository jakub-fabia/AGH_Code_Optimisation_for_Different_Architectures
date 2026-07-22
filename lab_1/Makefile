CC = gcc
SRCS = $(wildcard *.c)
BINS = $(SRCS:.c=)
O2_BINS = $(BINS:%=%_O2)

.PHONY: all normal optimized benchmark clean

all: normal optimized

normal: $(BINS)

optimized: $(O2_BINS)

benchmark: all
	@for bin in $(BINS); do \
		echo "Running $$bin"; \
		./$$bin; \
		echo "Running $${bin}_O2"; \
		./$${bin}_O2; \
	done

%: %.c
	$(CC) $< -o $@

%_O2: %.c
	$(CC) -O2 $< -o $@

clean:
	rm -f $(BINS) $(O2_BINS)
