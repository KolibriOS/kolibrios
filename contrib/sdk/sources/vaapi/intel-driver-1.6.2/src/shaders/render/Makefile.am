
INTEL_G4I =			\
	exa_wm.g4i		\
	exa_wm_affine.g4i	\
	exa_wm_yuv_color_balance.gxa \
	exa_yuv_rgb.gxa		\
	exa_yuv_gen4.g4i	\
	exa_yuv_gen6.g4i

INTEL_G4A =				\
	exa_sf.g4a			\
	exa_wm_xy.g4a			\
	exa_wm_src_affine.g4a		\
	exa_wm_src_sample_argb.g4a	\
	exa_wm_src_sample_planar.g4a	\
	exa_wm_yuv_color_balance.g4a	\
	exa_wm_yuv_rgb.g4a		\
	exa_wm_write.g4a

INTEL_G4S = $(INTEL_G4A:%.g4a=%.g4s)

INTEL_G4B =				\
	exa_sf.g4b			\
	exa_wm_xy.g4b			\
	exa_wm_src_affine.g4b		\
	exa_wm_src_sample_argb.g4b	\
	exa_wm_src_sample_planar.g4b	\
	exa_wm_yuv_color_balance.g4b	\
	exa_wm_yuv_rgb.g4b		\
	exa_wm_write.g4b

INTEL_G4B_GEN5 =				\
	exa_sf.g4b.gen5				\
	exa_wm_xy.g4b.gen5			\
	exa_wm_src_affine.g4b.gen5		\
	exa_wm_src_sample_argb.g4b.gen5	\
	exa_wm_src_sample_planar.g4b.gen5	\
	exa_wm_yuv_color_balance.g4b.gen5	\
	exa_wm_yuv_rgb.g4b.gen5			\
	exa_wm_write.g4b.gen5

INTEL_G6I = $(INTEL_G4I)

INTEL_G6A =				\
	exa_wm_src_affine.g6a 		\
	exa_wm_src_sample_argb.g6a 	\
	exa_wm_src_sample_planar.g6a 	\
	exa_wm_write.g6a 		\
	exa_wm_yuv_color_balance.g6a	\
	exa_wm_yuv_rgb.g6a

INTEL_G6S = $(INTEL_G6A:%.g6a=%.g6s)

INTEL_G6B =				\
	exa_wm_src_affine.g6b 		\
	exa_wm_src_sample_argb.g6b 	\
	exa_wm_src_sample_planar.g6b 	\
	exa_wm_write.g6b 		\
	exa_wm_yuv_color_balance.g6b	\
	exa_wm_yuv_rgb.g6b

INTEL_G7I = $(INTEL_G4I)		\
	exa_wm_write.g7i

INTEL_G7A =				\
	exa_wm_src_affine.g7a 		\
	exa_wm_src_sample_argb.g7a 	\
	exa_wm_src_sample_planar.g7a 	\
	exa_wm_write.g7a 		\
	exa_wm_yuv_color_balance.g7a	\
	exa_wm_yuv_rgb.g7a

INTEL_G7S = $(INTEL_G7A:%.g7a=%.g7s)

INTEL_G7B =				\
	exa_wm_src_affine.g7b 		\
	exa_wm_src_sample_argb.g7b 	\
	exa_wm_src_sample_planar.g7b 	\
	exa_wm_write.g7b 		\
	exa_wm_yuv_color_balance.g7b	\
	exa_wm_yuv_rgb.g7b

# XXX: only regenerate binary for EU code containing JMPI instructions
INTEL_G7B_HASWELL = \
	exa_wm_src_sample_planar.g7b.haswell	\
	exa_wm_yuv_color_balance.g7b.haswell	\
	$(NULL)

INTEL_G8A =				\
	exa_wm_src_affine.g8a 		\
	exa_wm_src_sample_planar.g8a 	\
	exa_wm_src_sample_argb.g8a 	\
	exa_wm_yuv_color_balance.g8a	\
	exa_wm_write.g8a 		\
	exa_wm_yuv_rgb.g8a

INTEL_G8S = $(INTEL_G8A:%.g8a=%.g8s)

INTEL_G8B =				\
	exa_wm_src_affine.g8b 		\
	exa_wm_src_sample_planar.g8b 	\
	exa_wm_src_sample_argb.g8b 	\
	exa_wm_yuv_color_balance.g8b	\
	exa_wm_yuv_rgb.g8b		\
	exa_wm_write.g8b 

INTEL_G9A =				\
	exa_wm_src_affine.g9a 		\
	exa_wm_src_sample_planar.g9a 	\
	exa_wm_src_sample_argb.g9a 	\
	exa_wm_yuv_color_balance.g9a	\
	exa_wm_write.g9a 		\
	exa_wm_yuv_rgb.g9a

INTEL_G9I = $(INTEL_G4I)

INTEL_G9S = $(INTEL_G9A:%.g9a=%.g9s)

INTEL_G9B =				\
	exa_wm_src_affine.g9b 		\
	exa_wm_src_sample_planar.g9b 	\
	exa_wm_src_sample_argb.g9b 	\
	exa_wm_yuv_color_balance.g9b	\
	exa_wm_yuv_rgb.g9b		\
	exa_wm_write.g9b

TARGETS  =
if HAVE_GEN4ASM
TARGETS += $(INTEL_G4B)
TARGETS += $(INTEL_G4B_GEN5)
TARGETS += $(INTEL_G6B)
TARGETS += $(INTEL_G7B)
TARGETS += $(INTEL_G7B_HASWELL)
TARGETS += $(INTEL_G8B)
TARGETS += $(INTEL_G9B)
endif

all-local: $(TARGETS)

SUFFIXES = .g4a .g4s .g4b .g4b.gen5 .g6a .g6s .g6b .g7a .g7s .g7b .g7b.haswell .g8a .g8b .g8s

if HAVE_GEN4ASM
$(INTEL_G4S): $(INTEL_G4A) $(INTEL_G4I)
.g4a.g4s:
	$(AM_V_GEN)m4 -I$(srcdir) $< > $@
.g4s.g4b:
	$(AM_V_GEN)$(GEN4ASM) -o $@ $<
.g4s.g4b.gen5:
	$(AM_V_GEN)$(GEN4ASM) -g 5 -o $@ $<

$(INTEL_G6S): $(INTEL_G6A) $(INTEL_G6I)
.g6a.g6s:
	$(AM_V_GEN)m4 -I$(srcdir) $< > $@
.g6s.g6b:
	$(AM_V_GEN)$(GEN4ASM) -g 6 -o $@ $<

$(INTEL_G7S): $(INTEL_G7A) $(INTEL_G7I)
.g7a.g7s:
	$(AM_V_GEN)m4 -I$(srcdir) $< > $@
.g7s.g7b:
	$(AM_V_GEN)$(GEN4ASM) -g 7 -o $@ $<
.g7s.g7b.haswell:
	$(AM_V_GEN)$(GEN4ASM) -g 7.5 -o $@ $<


$(INTEL_G8S): $(INTEL_G8A) $(INTEL_G8I)
.g8a.g8s:
	$(AM_V_GEN)m4 -I$(srcdir) $< > $@
.g8s.g8b:
	$(AM_V_GEN)$(GEN4ASM) -g 8 -o $@ $<

$(INTEL_G9S): $(INTEL_G9A) $(INTEL_G9I)
.g9a.g9s:
	$(AM_V_GEN)m4 -I$(srcdir) $< > $@
.g9s.g9b:
	$(AM_V_GEN)$(GEN4ASM) -g 9 -o $@ $<

endif

CLEANFILES = \
	$(INTEL_G4S)		\
	$(INTEL_G6S)		\
	$(INTEL_G7S)		\
	$(INTEL_G8S)		\
	$(INTEL_G9S)		\
	$(NULL)

EXTRA_DIST = \
	$(INTEL_G4A)		\
	$(INTEL_G4B)		\
	$(INTEL_G4B_GEN5)	\
	$(INTEL_G4I)		\
	$(INTEL_G6A)		\
	$(INTEL_G6B)		\
	$(INTEL_G7A)		\
	$(INTEL_G7B)		\
	$(INTEL_G7B_HASWELL)	\
	$(INTEL_G7I)		\
	$(INTEL_G8A)		\
	$(INTEL_G8B)		\
	$(INTEL_G9A)		\
	$(INTEL_G9B)		\
	$(NULL)

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = Makefile.in
