MFC_CORE	= \
	end_thread.asm				\
	mfc_batchbuffer_head.asm		\
	mfc_batchbuffer_tail.asm
MFC_CORE_AVC	= \
	mfc_batchbuffer_avc_intra.asm		\
	mfc_batchbuffer_avc_inter.asm

MFC_CORE_HSW 	= \
	mfc_batchbuffer_hsw.asm

INTEL_G6B	= mfc_batchbuffer_avc_intra.g6b mfc_batchbuffer_avc_inter.g6b
INTEL_G6A	= mfc_batchbuffer_avc_intra.g6a mfc_batchbuffer_avc_inter.g6a
INTEL_GEN6_INC	= mfc_batchbuffer.inc
INTEL_GEN6_ASM	= $(INTEL_G6A:%.g6a=%.gen6.asm)

INTEL_G7B	= mfc_batchbuffer_avc_intra.g7b mfc_batchbuffer_avc_inter.g7b
INTEL_G7A	= mfc_batchbuffer_avc_intra.g7a mfc_batchbuffer_avc_inter.g7a
INTEL_GEN7_INC	= mfc_batchbuffer.inc
INTEL_GEN7_ASM	= $(INTEL_G7A:%.g7a=%.gen7.asm)

INTEL_G75B	= mfc_batchbuffer_hsw.g75b
INTEL_G75A	= mfc_batchbuffer_hsw.g75a
INTEL_GEN75_INC	= mfc_batchbuffer_hsw.inc
INTEL_GEN75_ASM	= $(INTEL_G75A:%.g75a=%.gen75.asm)

INTEL_G9B	= mfc_batchbuffer_avc_intra.g9b mfc_batchbuffer_avc_inter.g9b
INTEL_G9A	= mfc_batchbuffer_avc_intra.g9a mfc_batchbuffer_avc_inter.g9a
INTEL_GEN9_INC	= mfc_batchbuffer.inc
INTEL_GEN9_ASM	= $(INTEL_G9A:%.g9a=%.gen9.asm)

TARGETS  =
if HAVE_GEN4ASM
TARGETS += $(INTEL_G6B)
TARGETS += $(INTEL_G7B)
TARGETS += $(INTEL_G75B)
TARGETS += $(INTEL_G9B)
endif

all-local: $(TARGETS)

SUFFIXES = .g6a .g6b .g7a .g7b .gen6.asm .gen7.asm .g75a .g75b .gen75.asm .g9a .g9b .gen9.asm

if HAVE_GEN4ASM
$(INTEL_GEN6_ASM): $(MFC_CORE) $(MFC_CORE_AVC) $(INTEL_GEN6_INC)
.g6a.gen6.asm:
	$(AM_V_GEN)cpp -P -DDEV_SNB $< > _mfc0.$@ 	&& \
	m4 _mfc0.$@ > $@				&& \
	rm _mfc0.$@
.gen6.asm.g6b:
	$(AM_V_GEN)$(GEN4ASM) -g 6 -o $@ $<

$(INTEL_GEN7_ASM): $(MFC_CORE) $(MFC_CORE_AVC) $(INTEL_GEN7_INC)
.g7a.gen7.asm:
	$(AM_V_GEN)cpp -P -DDEV_IVB $< > _mfc0.$@ 	&& \
	m4 _mfc0.$@ > $@				&& \
	rm _mfc0.$@
.gen7.asm.g7b:
	$(AM_V_GEN)$(GEN4ASM) -g 7 -o $@ $<

$(INTEL_GEN75_ASM): $(MFC_CORE_HSW) $(INTEL_GEN75_INC)
.g75a.gen75.asm:
	$(AM_V_GEN)cpp -P $< > _mfc0.$@			&& \
	m4 _mfc0.$@ > $@				&& \
	rm _mfc0.$@
.gen75.asm.g75b:
	$(AM_V_GEN)$(GEN4ASM) -g 7.5 -o $@ $<

$(INTEL_GEN9_ASM): $(MFC_CORE) $(MFC_CORE_AVC) $(INTEL_GEN9_INC)
.g9a.gen9.asm:
	$(AM_V_GEN)cpp -P -DDEV_IVB $< > _mfc0.$@ 	&& \
	m4 _mfc0.$@ > $@				&& \
	rm _mfc0.$@
.gen9.asm.g9b:
	$(AM_V_GEN)$(GEN4ASM) -g 9 -o $@ $<

endif

CLEANFILES = $(INTEL_GEN6_ASM) $(INTEL_GEN7_ASM) $(INTEL_GEN75_ASM) $(INTEL_GEN9_ASM)

EXTRA_DIST = \
	$(INTEL_G6A)		\
	$(INTEL_G6B)		\
	$(INTEL_G7A)		\
	$(INTEL_G7B)		\
	$(INTEL_G75A)		\
	$(INTEL_G75B)		\
	$(INTEL_GEN6_INC)	\
	$(INTEL_GEN7_INC)	\
	$(INTEL_GEN75_INC)	\
	$(INTEL_G9A)		\
	$(INTEL_G9B)		\
	$(INTEL_GEN9_INC)	\
	$(MFC_CORE)		\
	$(MFC_CORE_AVC)		\
	$(MFC_CORE_HSW)		\
	$(NULL)

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = Makefile.in
