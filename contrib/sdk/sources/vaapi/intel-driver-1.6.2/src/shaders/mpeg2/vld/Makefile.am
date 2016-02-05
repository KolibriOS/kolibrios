INTEL_G4I =					\
	addidct.g4i				\
	do_iq_intra.g4i				\
	do_iq_non_intra.g4i			\
	field_addidct.g4i			\
	field_motion_uv.g4i			\
	field_motion_y.g4i			\
	field_read_x0y0_uv.g4i			\
	field_read_x0y0_y.g4i			\
	field_read_x0y1_uv.g4i			\
	field_read_x0y1_y.g4i			\
	field_read_x1y0_uv.g4i			\
	field_read_x1y0_y.g4i			\
	field_read_x1y1_uv.g4i			\
	field_read_x1y1_y.g4i			\
	idct.g4i				\
	iq_intra.g4i				\
	iq_non_intra.g4i			\
	motion_field_uv.g4i			\
	motion_field_y.g4i			\
	motion_frame_uv.g4i			\
	motion_frame_y.g4i			\
	read_field_x0y0_uv.g4i			\
	read_field_x0y0_y.g4i			\
	read_field_x0y1_uv.g4i			\
	read_field_x0y1_y.g4i			\
	read_field_x1y0_uv.g4i			\
	read_field_x1y0_y.g4i			\
	read_field_x1y1_uv.g4i			\
	read_field_x1y1_y.g4i			\
	read_frame_x0y0_uv.g4i			\
	read_frame_x0y0_y.g4i			\
	read_frame_x0y1_uv.g4i			\
	read_frame_x0y1_y.g4i			\
	read_frame_x1y0_uv.g4i			\
	read_frame_x1y0_y.g4i			\
	read_frame_x1y1_uv.g4i			\
	read_frame_x1y1_y.g4i			\
	$(NULL)

INTEL_G4A = frame_intra.g4a 			\
	    frame_frame_pred_forward.g4a	\
	    frame_frame_pred_backward.g4a	\
	    frame_frame_pred_bidirect.g4a	\
	    frame_field_pred_forward.g4a	\
	    frame_field_pred_backward.g4a	\
	    frame_field_pred_bidirect.g4a	\
	    lib.g4a				\
	    field_intra.g4a			\
	    field_forward.g4a			\
	    field_forward_16x8.g4a		\
	    field_backward.g4a			\
	    field_backward_16x8.g4a		\
	    field_bidirect.g4a			\
	    field_bidirect_16x8.g4a		\
	    null.g4a

INTEL_G4S = $(INTEL_G4A:%.g4a=%.g4s)

INTEL_G4B = frame_intra.g4b 			\
	    frame_frame_pred_forward.g4b	\
	    frame_frame_pred_backward.g4b	\
	    frame_frame_pred_bidirect.g4b	\
	    frame_field_pred_forward.g4b	\
	    frame_field_pred_backward.g4b	\
	    frame_field_pred_bidirect.g4b	\
	    lib.g4b				\
	    field_intra.g4b			\
	    field_forward.g4b			\
	    field_forward_16x8.g4b		\
	    field_backward.g4b			\
	    field_backward_16x8.g4b		\
	    field_bidirect.g4b			\
	    field_bidirect_16x8.g4b

INTEL_G4B_GEN5 = frame_intra.g4b.gen5 		\
	    frame_frame_pred_forward.g4b.gen5	\
	    frame_frame_pred_backward.g4b.gen5	\
	    frame_frame_pred_bidirect.g4b.gen5	\
	    frame_field_pred_forward.g4b.gen5	\
	    frame_field_pred_backward.g4b.gen5	\
	    frame_field_pred_bidirect.g4b.gen5	\
	    lib.g4b.gen5			\
	    field_intra.g4b.gen5		\
	    field_forward.g4b.gen5		\
	    field_forward_16x8.g4b.gen5		\
	    field_backward.g4b.gen5		\
	    field_backward_16x8.g4b.gen5	\
	    field_bidirect.g4b.gen5		\
	    field_bidirect_16x8.g4b.gen5

TARGETS  =
if HAVE_GEN4ASM
TARGETS += $(INTEL_G4B)
TARGETS += $(INTEL_G4B_GEN5)
endif

all-local: $(TARGETS)

SUFFIXES = .g4a .g4s .g4b .g4b.gen5

if HAVE_GEN4ASM
$(INTEL_G4S): $(INTEL_G4A) $(INTEL_G4I)
.g4a.g4s:
	$(AM_V_GEN)m4 -I$(srcdir) $< > $@
.g4s.g4b:
	$(AM_V_GEN)$(GEN4ASM) -o $@ $<
.g4s.g4b.gen5:
	$(AM_V_GEN)$(GEN4ASM) -g 5 -o $@ $<
endif

CLEANFILES = $(INTEL_G4S)

EXTRA_DIST = \
	$(INTEL_G4A)		\
	$(INTEL_G4B)		\
	$(INTEL_G4B_GEN5)	\
	$(INTEL_G4I)		\
	$(NULL)

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = Makefile.in
