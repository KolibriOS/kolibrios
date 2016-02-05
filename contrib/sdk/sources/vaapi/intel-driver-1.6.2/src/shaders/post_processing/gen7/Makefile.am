INTEL_PP_G7B = \
	avs.g7b			\
	dndi.g7b		\
	nv12_dn_nv12.g7b	\
	pa_to_pl2.g7b		\
	pa_to_pl3.g7b		\
	pa_to_pa.g7b		\
	pl2_to_pa.g7b		\
	pl2_to_pl2.g7b		\
	pl2_to_pl3.g7b		\
	pl2_to_rgbx.g7b		\
	pl3_to_pa.g7b		\
	pl3_to_pl2.g7b		\
	pl3_to_pl3.g7b		\
	rgbx_to_nv12.g7b	\
	$(NULL)

INTEL_PP_G4A = \
	DI_Core.g4a			\
	DI_Save_NV12_16x4.g4a		\
	DI_Save_PA_16x4.g4a		\
	EOT.g4a				\
	NV12_DI_NV12.g4a		\
	NV12_DNDI_422CP.g4a		\
	NV12_DNDI_PA.g4a		\
	NV12_DNUV_NV12.g4a		\
	NV12_DN_422CP.g4a		\
	NV12_DN_NV12.g4a		\
	PA_AVS_Buf_0.g4a		\
	PA_AVS_Buf_1.g4a		\
	PA_AVS_Buf_2.g4a		\
	PA_AVS_Buf_3.g4a		\
	PA_DI_422CP.g4a			\
	PA_DI_PA.g4a			\
	PA_DNDI_422CP.g4a		\
	PA_DNDI_PA.g4a			\
	PA_DNUV_PA.g4a			\
	PA_DN_422CP.g4a			\
	PA_DN_PA.g4a			\
	PL2_AVS_Buf_0.g4a		\
	PL2_AVS_Buf_1.g4a		\
	PL2_AVS_Buf_2.g4a		\
	PL2_AVS_Buf_3.g4a		\
	PL3_AVS_Buf_0.g4a		\
	PL3_AVS_Buf_1.g4a 		\
	PL3_AVS_Buf_2.g4a		\
	PL3_AVS_Buf_3.g4a		\
	PL3_DNDI_422CP.g4a		\
	PL3_DNDI_PA.g4a			\
	PL3_DNUV_PL3.g4a		\
	PL3_DN_422CP.g4a		\
	PL3_DN_PL3.g4a			\
	PL_DI_422CP.g4a			\
	PL_DI_PA.g4a			\
	RGB_to_YUV.g4a			\
	Save_AVS_PA.g4a  		\
	Save_AVS_PL3.g4a		\
	Save_AVS_NV12.g4a		\
	Save_AVS_RGB.g4a		\
	Save_AVS_RGBX.g4a		\
	Set_AVS_Buf_0123_BGRA.g4a	\
	Set_AVS_Buf_0123_PL2.g4a	\
	Set_AVS_Buf_0123_PL3.g4a	\
	Set_AVS_Buf_0123_VUYA.g4a	\
	Set_AVS_Buf_0123_VYUA.g4a	\
	Set_Layer_0.g4a			\
	VP_Setup.g4a			\
	YUV_to_RGB.g4a			\
	$(NULL)

INTEL_PP_ASM = $(INTEL_PP_G7B:%.g7b=%.asm)
INTEL_PP_GEN7_ASM = $(INTEL_PP_G7B:%.g7b=%.g7s)

INTEL_PP_G75B = $(INTEL_PP_G7B:%.g7b=%.g75b)

TARGETS  =
if HAVE_GEN4ASM
TARGETS += $(INTEL_PP_G7B) $(INTEL_PP_G75B)
endif

all-local: $(TARGETS)

SUFFIXES = .g7b .g7s .asm

if HAVE_GEN4ASM
$(INTEL_PP_GEN7_ASM): $(INTEL_PP_ASM) $(INTEL_PP_G4A)
.asm.g7s:
	$(AM_V_GEN)cpp $< > _pp0.$@;		\
	$(PYTHON2) $(top_srcdir)/src/shaders/gpp.py _pp0.$@ $@;	\
	rm _pp0.$@
.g7s.g7b:
	$(AM_V_GEN)$(GEN4ASM) -a -o $@ -g 7 $<

.g7s.g75b:
	$(AM_V_GEN)$(GEN4ASM) -a -o $@ -g 7.5 $<
endif

CLEANFILES = $(INTEL_PP_GEN7_ASM)

EXTRA_DIST = \
	$(INTEL_PP_ASM)		\
	$(INTEL_PP_G4A)		\
	$(INTEL_PP_G75B)	\
	$(INTEL_PP_G7B)		\
	$(NULL)

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = Makefile.in
