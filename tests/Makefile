TESTS=$(sort $(basename $(wildcard *.cc)))

CXX = g++
CFLAGS = -Wall -O2 -g

all: $(addprefix test-,$(TESTS))

check: all
	@echo "Running unit tests..."
	@set -e; for test in $(TESTS); do \
	  echo "Testing: $$test"; \
	  ./test-$$test test-$$test.ppm; \
	done
	@echo "Success."

.PHONY: all check

pictie.o: ../pictie.h ../pictie.cc
	$(CXX) $(CFLAGS) $(CPPFLAGS) -I.. -o $@ -c ../pictie.cc

test-%: %.cc pictie.o
	$(CXX) $(CFLAGS) $(CPPFLAGS) -I.. -o $@ pictie.o $<

clean:
	rm -f $(addprefix test-,$(TESTS)) pictie.o
