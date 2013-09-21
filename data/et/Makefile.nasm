# This is similar to Makefile.fasm, look there for comments.
define nasm_meta_rule
.deps/$(4).Po: $(2) Makefile.nasm .deps/.dir
	nasm -I$$(dir $(2)) -o "$(1)" -M $$< > .deps/$(4).Tpo 2>/dev/null; \
	if [ $$$$? -eq 0 ]; then perl -ln -e 'next unless $$$$_;' \
	-e 'm/((.*): )?(.*)/;$$$$a=$$$$2 if $$$$2;push @b,$$$$3;' \
	-e 'END{$$$$b=join "\n",@b;print "$$$$a .deps/$(4).Po: $$$$b\n$$$$b:"}' \
	.deps/$(4).Tpo > .deps/$(4).Po; fi
	rm -f .deps/$(4).Tpo
$(1): $(2) Makefile.nasm $$(call respace,$$(addsuffix .dir,$(3)))
	nasm -I$$(dir $(2)) -o "$$@" $$<
	kpack --nologo "$$@"
-include .deps/$(4).Po
endef

$(foreach f,$(NASM_PROGRAMS),$(eval $(call nasm_meta_rule,$(fbinary),$(fsource),$(binarydir),$(progname))))
