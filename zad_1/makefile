.PHONY: all run test metrics clean

all:
	@for dir in */; do \
		(cd "$$dir" && g++ base.cpp -o base -pg); \
	done

run:
	@for dir in */; do \
		(cd "$$dir" && ./base); \
	done

test:
	@for dir in */; do \
		(cd "$$dir" && gprof base >> raport.txt); \
	done

metrics: all
	@python3 run_metrics.py

clean:
	@for dir in */; do \
		rm -f "$$dir/base" "$$dir/gmon.out" "$$dir/raport.txt" "$$dir/metrics.txt" "$$dir/file.txt"; \
	done
