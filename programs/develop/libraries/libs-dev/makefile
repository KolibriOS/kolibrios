LIBPREFIX=lib
LIBEXT=obj
TESTPREFIX=test

.PHONY: all

all: libraries tests

libraries:
	@echo "building libraries..."
	@mkdir -p bin
	@for i in gfx img ini io; do \
	  echo "  $(LIBPREFIX)$${i}"; \
	  cd "$(LIBPREFIX)$${i}"; \
	  fasm "$(LIBPREFIX)$${i}.asm" "../bin/$(LIBPREFIX)$${i}.$(LIBEXT)" >/dev/null; \
	  cd ..; \
	done

tests:
	@echo "building tests..."
	@mkdir -p bin/.test
	@for i in 001 002; do \
	  echo "  $${i}"; \
	  cd ".test/$${i}"; \
	  fasm "$(TESTPREFIX)$${i}.asm" "../../bin/.test/$(TESTPREFIX)$${i}" >/dev/null; \
	  cd ../..; \
	done

clean:
	rm -rf bin
