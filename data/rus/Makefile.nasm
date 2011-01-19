# This is similar to Makefile.fasm, look there for comments.
define nasm_meta_rule
.deps/$(4).Po: $(2) Makefile.nasm .deps/.dir
	nasm -I$$(dir $(2)) -o "$(1)" -M $$< > .deps/$(4).Tpo 2>/dev/null; if [ $$$$? -eq 0 ]; \
	then sed 's|\(.*\):|\1 .deps/$(4).Po:|' .deps/$(4).Tpo > .deps/$(4).Po; fi
	rm -f .deps/$(4).Tpo
$(1): $(2) Makefile.nasm $$(call respace,$$(addsuffix .dir,$(3)))
	nasm -I$$(dir $(2)) -o "$$@" $$<
	kpack --nologo "$$@"
-include .deps/$(4).Po
endef

$(foreach f,$(NASM_PROGRAMS),$(eval $(call nasm_meta_rule,$(fbinary),$(fsource),$(binarydir),$(progname))))
