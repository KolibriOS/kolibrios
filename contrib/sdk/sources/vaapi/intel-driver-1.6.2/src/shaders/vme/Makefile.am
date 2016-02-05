VME_CORE	= batchbuffer.asm intra_frame.asm inter_frame.asm
VME7_CORE	= batchbuffer.asm intra_frame_ivb.asm inter_frame_ivb.asm inter_bframe_ivb.asm mpeg2_inter_ivb.asm
VME75_CORE	= batchbuffer.asm intra_frame_haswell.asm inter_frame_haswell.asm inter_bframe_haswell.asm mpeg2_inter_haswell.asm
VME8_CORE	= intra_frame_gen8.asm	inter_frame_gen8.asm inter_bframe_gen8.asm mpeg2_inter_gen8.asm vp8_intra_frame_gen8.asm vp8_inter_frame_gen8.asm
VME9_CORE	= $(VME8_CORE)

INTEL_G6B	= batchbuffer.g6b intra_frame.g6b inter_frame.g6b
INTEL_G6A	= batchbuffer.g6a intra_frame.g6a inter_frame.g6a
INTEL_GEN6_INC	= batchbuffer.inc vme.inc
INTEL_GEN6_ASM	= $(INTEL_G6A:%.g6a=%.gen6.asm)

INTEL_G7B	= batchbuffer.g7b intra_frame.g7b inter_frame.g7b intra_frame_ivb.g7b inter_frame_ivb.g7b inter_bframe_ivb.g7b mpeg2_inter_ivb.g7b
INTEL_G7A	= batchbuffer.g7a intra_frame.g7a inter_frame.g7a intra_frame_ivb.g7a inter_frame_ivb.g7a inter_bframe_ivb.g7a mpeg2_inter_ivb.g7a
INTEL_GEN7_INC	= batchbuffer.inc vme.inc vme7_mpeg2.inc vme7.inc
INTEL_GEN7_ASM	= $(INTEL_G7A:%.g7a=%.gen7.asm)

INTEL_G75B	= batchbuffer.g75b intra_frame_haswell.g75b inter_frame_haswell.g75b inter_bframe_haswell.g75b mpeg2_inter_haswell.g75b
INTEL_G75A	= batchbuffer.g75a intra_frame_haswell.g75a inter_frame_haswell.g75a inter_bframe_haswell.g75a mpeg2_inter_haswell.g75a
INTEL_GEN75_INC	= batchbuffer.inc vme75.inc vme75_mpeg2.inc
INTEL_GEN75_ASM	= $(INTEL_G75A:%.g75a=%.gen75.asm)


INTEL_G8B	= intra_frame_gen8.g8b inter_frame_gen8.g8b inter_bframe_gen8.g8b mpeg2_inter_gen8.g8b vp8_intra_frame_gen8.g8b vp8_inter_frame_gen8.g8b
INTEL_G8A	= intra_frame_gen8.g8a inter_frame_gen8.g8a inter_bframe_gen8.g8a mpeg2_inter_gen8.g8a vp8_intra_frame_gen8.g8a vp8_inter_frame_gen8.g8a
INTEL_GEN8_INC	= vme8.inc vme75_mpeg2.inc
INTEL_GEN8_ASM	= $(INTEL_G8A:%.g8a=%.gen8.asm)


INTEL_G9B	= intra_frame_gen9.g9b inter_frame_gen9.g9b inter_bframe_gen9.g9b mpeg2_inter_gen9.g9b vp8_intra_frame_gen9.g9b vp8_inter_frame_gen9.g9b
INTEL_G9A	= intra_frame_gen9.g9a inter_frame_gen9.g9a inter_bframe_gen9.g9a mpeg2_inter_gen9.g9a vp8_intra_frame_gen9.g9a vp8_inter_frame_gen9.g9a
INTEL_GEN9_INC	= $(INTEL_GEN8_INC)
INTEL_GEN9_ASM	= $(INTEL_G9A:%.g9a=%.gen9.asm)


TARGETS  =
if HAVE_GEN4ASM
TARGETS += $(INTEL_G6B)
TARGETS += $(INTEL_G7B)
TARGETS += $(INTEL_G75B)
TARGETS += $(INTEL_G8B)
TARGETS += $(INTEL_G9B)
endif

all-local: $(TARGETS)

SUFFIXES = .g6a .g6b .g7a .g7b .gen6.asm .gen7.asm .g75a .g75b .gen75.asm .g8a .g8b .gen8.asm .g9a .g9b .gen9.asm

if HAVE_GEN4ASM
$(INTEL_GEN6_ASM): $(VME_CORE) $(INTEL_GEN6_INC)
.g6a.gen6.asm:
	$(AM_V_GEN)cpp -P -DDEV_SNB $< > _vme0.$@ 	&& \
	m4 _vme0.$@ > $@				&& \
	rm _vme0.$@
.gen6.asm.g6b:
	$(AM_V_GEN)$(GEN4ASM) -g 6 -o $@ $<

$(INTEL_GEN7_ASM): $(VME7_CORE) $(INTEL_GEN7_INC)
.g7a.gen7.asm:
	$(AM_V_GEN)cpp -P -DDEV_IVB $< > _vme0.$@ 	&& \
	m4 _vme0.$@ > $@				&& \
	rm _vme0.$@
.gen7.asm.g7b:
	$(AM_V_GEN)$(GEN4ASM) -g 7 -o $@ $<


$(INTEL_GEN75_ASM): $(VME75_CORE) $(INTEL_GEN75_INC)
.g75a.gen75.asm:
	$(AM_V_GEN)cpp -P $< > _vme0.$@ 	&& \
	m4 _vme0.$@ > $@				&& \
	rm _vme0.$@
.gen75.asm.g75b:
	$(AM_V_GEN)$(GEN4ASM) -g 7.5 -o $@ $<

$(INTEL_GEN8_ASM): $(VME8_CORE) $(INTEL_GEN8_INC)
.g8a.gen8.asm:
	$(AM_V_GEN)cpp -P $< > _vme0.$@ 	&& \
	m4 _vme0.$@ > $@				&& \
	rm _vme0.$@
.gen8.asm.g8b:
	$(AM_V_GEN)$(GEN4ASM) -g 8 -o $@ $<

$(INTEL_GEN9_ASM): $(VME9_CORE) $(INTEL_GEN9_INC)
.g9a.gen9.asm:
	$(AM_V_GEN)cpp -P $< > _vme0.$@ 	&& \
	m4 _vme0.$@ > $@				&& \
	rm _vme0.$@
.gen9.asm.g9b:
	$(AM_V_GEN)$(GEN4ASM) -g 9 -o $@ $<

endif

CLEANFILES = $(INTEL_GEN6_ASM) $(INTEL_GEN7_ASM) $(INTEL_GEN75_ASM) $(INTEL_GEN8_ASM) $(INTEL_GEN9_ASM)

EXTRA_DIST = \
	$(INTEL_G6A)		\
	$(INTEL_G6B)		\
	$(INTEL_G75A)		\
	$(INTEL_G75B)		\
	$(INTEL_G7A)		\
	$(INTEL_G7B)		\
	$(INTEL_G8A)            \
	$(INTEL_G8B)            \
	$(INTEL_G9A)		\
	$(INTEL_G9B)		\
	$(INTEL_GEN6_INC)	\
	$(INTEL_GEN75_INC)	\
	$(INTEL_GEN7_INC)	\
	$(INTEL_GEN8_INC)       \
	$(INTEL_GEN9_INC)	\
	$(VME75_CORE)		\
	$(VME7_CORE)		\
	$(VME8_CORE)            \
	$(VME9_CORE)		\
	$(VME_CORE)		\
	$(NULL)

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = Makefile.in
