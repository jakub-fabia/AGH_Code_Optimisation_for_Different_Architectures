CC      = gcc
LDFLAGS =

SIZE   ?= 1500
OUTPUT  = results.txt

PROGS = ge1 ge2 ge3 ge4 ge5 ge6 ge7 ge8

# Wszystkie kombinacje {-O2, -march=native, -mfma} + brak flag.
COMBOS = none O2 march mfma O2_march O2_mfma march_mfma O2_march_mfma

.PHONY: test clean

test:
	@echo "=== GE benchmarks: rozne kombinacje flag ===" > $(OUTPUT)
	@echo "SIZE = $(SIZE)"  >> $(OUTPUT)
	@echo "CC   = $(CC)"    >> $(OUTPUT)
	@echo "Data = $$(date)" >> $(OUTPUT)
	@echo "Uwaga: ge8 zawsze dostaje dodatkowo -mavx (intrinsics AVX inaczej nie skompiluja)." >> $(OUTPUT)
	@for combo in $(COMBOS); do \
	    case "$$combo" in \
	        none)          flags="";; \
	        O2)            flags="-O2";; \
	        march)         flags="-march=native";; \
	        mfma)          flags="-mfma";; \
	        O2_march)      flags="-O2 -march=native";; \
	        O2_mfma)       flags="-O2 -mfma";; \
	        march_mfma)    flags="-march=native -mfma";; \
	        O2_march_mfma) flags="-O2 -march=native -mfma";; \
	    esac; \
	    echo "" | tee -a $(OUTPUT); \
	    echo "============================================" | tee -a $(OUTPUT); \
	    echo "# Combo: $$combo   [ $$flags ]"               | tee -a $(OUTPUT); \
	    echo "============================================" | tee -a $(OUTPUT); \
	    for p in $(PROGS); do \
	        extra=""; \
	        [ "$$p" = "ge8" ] && extra="-mavx"; \
	        echo "--- $$p  (flags: $$flags $$extra) ---" | tee -a $(OUTPUT); \
	        if $(CC) $$flags $$extra -o $$p $$p.c $(LDFLAGS) 2>/tmp/ge_build.log; then \
	            ./$$p $(SIZE) 2>&1 | tee -a $(OUTPUT); \
	        else \
	            echo "COMPILE FAILED:" | tee -a $(OUTPUT); \
	            cat /tmp/ge_build.log  | tee -a $(OUTPUT); \
	        fi; \
	        echo "" | tee -a $(OUTPUT); \
	    done; \
	done
	@echo "Wyniki zapisane w $(OUTPUT)"

clean:
	rm -f $(PROGS) /tmp/ge_build.log
