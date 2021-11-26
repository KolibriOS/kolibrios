ifndef COVERAGE
UMKA_SHELL=../umka_shell
else
UMKA_SHELL=sudo taskset 1 ../umka_shell -c
endif

xfs_tests      := $(addsuffix .out.log, $(basename $(wildcard *\#xfs_*.t)))
s05k_tests     := $(addsuffix .out.log, $(basename $(wildcard *\#s05k_*.t)))
s4k_tests      := $(addsuffix .out.log, $(basename $(wildcard *\#s4k_*.t)))
f30_tests      := $(addsuffix .out.log, $(basename $(wildcard *\#f30_*.t)))
f70_tests      := $(addsuffix .out.log, $(basename $(wildcard *\#f70_*.t)))
f70s0_tests    := $(addsuffix .out.log, $(basename $(wildcard *\#f70s0_*.t)))
f70s1_tests    := $(addsuffix .out.log, $(basename $(wildcard *\#f70s1_*.t)))
f70s5_tests    := $(addsuffix .out.log, $(basename $(wildcard *\#f70s5_*.t)))
lookup_tests   := $(addsuffix .out.log, $(basename $(wildcard *\#lookup_*.t)))
bug_tests      := $(addsuffix .out.log, $(basename $(wildcard *\#bug_*.t)))
xattr_tests    := $(addsuffix .out.log, $(basename $(wildcard *\#xattr_*.t)))
unicode_tests  := $(addsuffix .out.log, $(basename $(wildcard *\#unicode_*.t)))
v5_tests       := $(addsuffix .out.log, $(basename $(wildcard *\#v5_*.t)))
draw_tests     := $(addsuffix .out.log, $(basename $(wildcard *\#draw_*.t)))
coverage_tests := $(addsuffix .out.log, $(basename $(wildcard *\#coverage_*.t)))
i40_tests      := $(addsuffix .out.log, $(basename $(wildcard *\#i40_*.t)))
net_tests      := $(addsuffix .out.log, $(basename $(wildcard *\#net_*.t)))
arp_tests      := $(addsuffix .out.log, $(basename $(wildcard *\#arp_*.t)))
acpi_tests     := $(addsuffix .out.log, $(basename $(wildcard *\#acpi_*.t)))

all: xfs s05k s4k f30 f70 f70s0 f70s1 f70s5 lookup bug xattr unicode v5 draw coverage i40 net arp acpi
	@echo all tests passed

xfs: $(xfs_tests)
s05k: $(s05k_tests)
s4k: $(s4k_tests)
f30: $(f30_tests)
f70: $(f70_tests)
f70s0: $(f70s0_tests)
f70s1: $(f70s1_tests)
f70s5: $(f70s5_tests)
lookup: $(lookup_tests)
bug: $(bug_tests)
xattr: $(xattr_tests)
unicode: $(unicode_tests)
v5: $(v5_tests)
draw: $(draw_tests)
coverage: $(coverage_tests)
i40: $(i40_tests)
net: $(net_tests)
arp: $(arp_tests)
acpi: $(acpi_tests)

%.out.log: %.t
	$(UMKA_SHELL) < $*.t > $@
	@ cmp $*.{out,ref}.log
	@ if [ -f "$*.ref.png" ]; then cmp $*.{out,ref}.png; fi

collect_coverage:
	../covpreproc ../umka.lst 0x34 coverage.* > ../umka.cov

clean:
	rm -f ???_*.out.{log,png} coverage.*

update_logs:
	for f in *ref.log; do cp $$(basename $$f .ref.log).out.log $$f; done

.PHONY: clean collect_coverage update_logs
