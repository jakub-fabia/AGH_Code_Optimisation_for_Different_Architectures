CC      := gcc
LDLIBS  := -lm
OPT     ?=

SRCS    := chol1.c chol2.c chol3.c chol4.c chol5.c chol6.c
BINS    := $(SRCS:.c=)

CFLAGS_chol6 := -mavx

.PHONY: all clean test test-O2 test-native test-mfma test-all

all: $(BINS)

%: %.c
	$(CC) $(OPT) $($(addprefix CFLAGS_,$*)) $< -o $@ $(LDLIBS)

test: all
	@for b in $(BINS); do echo "== ./$$b =="; ./$$b 4000; done

test-O2:
	$(MAKE) clean
	$(MAKE) test OPT="-O2"

test-native:
	$(MAKE) clean
	$(MAKE) test OPT="-O2 -march=native"

test-mfma:
	$(MAKE) clean
	$(MAKE) test OPT="-O2 -mfma"

test-all: test test-O2 test-native test-mfma

clean:
	rm -f $(BINS) a.out
