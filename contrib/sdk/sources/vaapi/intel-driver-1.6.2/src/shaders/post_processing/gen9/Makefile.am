SRCDIR = $(top_srcdir)/src/shaders/post_processing/gen8
VPATH = $(SRCDIR)

INTEL_PP_G9B = \
	pl2_to_pl2.g9b		\
	pl2_to_pl3.g9b		\
	pl3_to_pl2.g9b		\
	pl3_to_pl3.g9b		\
	pl2_to_rgbx.g9b		\
	rgbx_to_nv12.g9b	\
	pl2_to_pa.g9b		\
	pl3_to_pa.g9b		\
	pa_to_pl2.g9b		\
	pa_to_pl3.g9b		\
	pa_to_pa.g9b		\
	$(NULL)

INTEL_PP_G8A = \
	EOT.g8a				\
	PL2_AVS_Buf_0.g8a		\
	PL2_AVS_Buf_1.g8a		\
	PL2_AVS_Buf_2.g8a		\
	PL2_AVS_Buf_3.g8a		\
	PL3_AVS_Buf_0.g8a		\
	PL3_AVS_Buf_1.g8a		\
	PL3_AVS_Buf_2.g8a		\
	PL3_AVS_Buf_3.g8a		\
	PA_AVS_Buf_0.g8a		\
	PA_AVS_Buf_1.g8a		\
	PA_AVS_Buf_2.g8a		\
	PA_AVS_Buf_3.g8a		\
	Save_AVS_NV12.g8a		\
	Save_AVS_PL3.g8a		\
	Save_AVS_RGBX.g8a		\
	Save_AVS_PA.g8a			\
	Set_AVS_Buf_0123_PL2.g8a	\
	Set_AVS_Buf_0123_PL3.g8a	\
	Set_AVS_Buf_0123_BGRA.g8a	\
	Set_AVS_Buf_0123_VYUA.g8a	\
	YUV_to_RGB.g8a			\
	RGB_to_YUV.g8a			\
	Set_Layer_0.g8a			\
	VP_Setup.g8a			\
	$(NULL)

INTEL_PP_G9A = $(INTEL_PP_G8A)
INTEL_PP_ASM = $(INTEL_PP_G9B:%.g9b=%.asm)
INTEL_PP_GEN9_ASM = $(INTEL_PP_G9B:%.g9b=%.g9s)

TARGETS  =
if HAVE_GEN4ASM
TARGETS += $(INTEL_PP_G9B)
endif

all-local: $(TARGETS)

SUFFIXES = .g9b .g9s .asm

$(INTEL_PP_GEN9_ASM): $(INTEL_PP_ASM) $(INTEL_PP_G9A)
.asm.g9s:
	$(AM_V_GEN)cpp $< > _pp0.$@;		\
	$(PYTHON2) $(top_srcdir)/src/shaders/gpp.py _pp0.$@ $@;	\
	rm _pp0.$@
.g9s.g9b:
	$(AM_V_GEN)$(GEN4ASM) -a -o $@ -g 9 $<

CLEANFILES = $(INTEL_PP_GEN9_ASM)

EXTRA_DIST = \
	$(INTEL_PP_G9B) \
	$(NULL)

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = Makefile.in
