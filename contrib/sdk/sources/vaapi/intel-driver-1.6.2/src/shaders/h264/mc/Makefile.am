
INTEL_G4I = 

INTEL_G4A = null.g4a

INTEL_G4B = null.g4b

INTEL_G4B_GEN5 = null.g4b.gen5

INTEL_MC_G4B		= avc_mc.g4b
INTEL_MC_G4B_GEN5	= avc_mc.g4b.gen5
INTEL_MC_EXPORT		= export.inc
INTEL_MC_EXPORT_GEN5	= export.inc.gen5

INTEL_MC_ASM =	\
	add_Error_16x16_Y.asm	\
	add_Error_UV.asm	\
	AllAVC.asm	\
	AllAVCField.asm	\
	AllAVCFrame.asm	\
	AllAVCMBAFF.asm	\
	AllIntra.asm	\
	AVCMCInter.asm	\
	BSDReset.asm	\
	chromaMVAdjust.asm	\
	DCResetDummy.asm	\
	Decode_Chroma_Intra.asm	\
	EndIntraThread.asm	\
	initialize_MBPara.asm	\
	interpolate_C_2x2.asm	\
	interpolate_C_4x4.asm	\
	interpolate_Y_4x4.asm	\
	interpolate_Y_8x8.asm	\
	Intra_16x16.asm	\
	Intra_4x4.asm	\
	Intra_8x8.asm	\
	Intra_funcLib.asm	\
	Intra_PCM.asm	\
	intra_pred_16x16_Y.asm	\
	intra_Pred_4x4_Y_4.asm	\
	intra_Pred_8x8_Y.asm	\
	intra_Pred_Chroma.asm	\
	load_Intra_Ref_UV.asm	\
	load_Intra_Ref_Y.asm	\
	loadRef_C_10x5.asm	\
	loadRef_C_6x3.asm	\
	loadRef_Y_16x13.asm	\
	loadRef_Y_16x9.asm	\
	recon_C_4x4.asm	\
	recon_Y_8x8.asm	\
	roundShift_C_4x4.asm	\
	save_16x16_Y.asm	\
	save_4x4_Y.asm	\
	save_8x8_UV.asm	\
	save_8x8_Y.asm	\
	save_I_PCM.asm	\
	scoreboard.asm	\
	scoreboard_MBAFF.asm	\
	scoreboard_restore_AS.asm	\
	scoreboard_save_AS.asm	\
	scoreboard_sip.asm	\
	scoreboard_start_inter.asm	\
	scoreboard_start_intra.asm	\
	scoreboard_update.asm	\
	SetHWScoreboard.asm	\
	SetHWScoreboard_MBAFF.asm	\
	set_SB_offset.asm	\
	SetupForHWMC.asm	\
	weightedPred.asm	\
	writeRecon_C_8x4.asm	\
	writeRecon_Y_16x8.asm	\
	writeRecon_YC.asm

INTEL_MC_INC = \
	AllAVC_Build.inc			\
	AllAVC_Export.inc			\
	export.inc				\
	header.inc				\
	HwmcOnlyHeader.inc			\
	inter_Header.inc			\
	intra_Header.inc			\
	Scoreboard_header.inc			\
	SetHWScoreboard_header.inc		\
	$(NULL)

INTEL_ILDB_ASM = \
	../ildb/AVC_ILDB_Child_Field_UV.asm	\
	../ildb/AVC_ILDB_Child_Field_Y.asm	\
	../ildb/AVC_ILDB_Child_Mbaff_UV.asm	\
	../ildb/AVC_ILDB_Child_Mbaff_Y.asm	\
	../ildb/AVC_ILDB_Child_UV.asm	\
	../ildb/AVC_ILDB_Child_Y.asm	\
	../ildb/AVC_ILDB_Chroma_Core.asm	\
	../ildb/AVC_ILDB_Chroma_Core_Mbaff.asm	\
	../ildb/AVC_ILDB_CloseGateway.asm	\
	../ildb/AVC_ILDB_Dep_Check.asm	\
	../ildb/AVC_ILDB_Filter_Mbaff_UV_h.asm	\
	../ildb/AVC_ILDB_Filter_Mbaff_UV_v.asm	\
	../ildb/AVC_ILDB_Filter_Mbaff_Y_h.asm	\
	../ildb/AVC_ILDB_Filter_Mbaff_Y_v.asm	\
	../ildb/AVC_ILDB_Filter_UV_h.asm	\
	../ildb/AVC_ILDB_Filter_UV_v.asm	\
	../ildb/AVC_ILDB_Filter_Y_h.asm	\
	../ildb/AVC_ILDB_Filter_Y_v.asm	\
	../ildb/AVC_ILDB_ForwardMsg.asm	\
	../ildb/AVC_ILDB_Luma_Core.asm	\
	../ildb/AVC_ILDB_Luma_Core_Mbaff.asm	\
	../ildb/AVC_ILDB_LumaThrdLimit.asm	\
	../ildb/AVC_ILDB_OpenGateway.asm	\
	../ildb/AVC_ILDB_Root_Field_UV.asm	\
	../ildb/AVC_ILDB_Root_Field_Y.asm	\
	../ildb/AVC_ILDB_Root_Mbaff_UV.asm	\
	../ildb/AVC_ILDB_Root_Mbaff_Y.asm	\
	../ildb/AVC_ILDB_Root_UV.asm	\
	../ildb/AVC_ILDB_Root_Y.asm	\
	../ildb/AVC_ILDB_Spawn.asm	\
	../ildb/AVC_ILDB_SpawnChild.asm	\
	../ildb/AVC_ILDB_SpawnChromaRoot.asm	\
	../ildb/load_Cur_UV_8x8T.asm	\
	../ildb/load_Cur_UV_8x8T_Mbaff.asm	\
	../ildb/load_Cur_UV_Right_Most_2x8.asm	\
	../ildb/load_Cur_Y_16x16T.asm	\
	../ildb/load_Cur_Y_16x16T_Mbaff.asm	\
	../ildb/load_Cur_Y_Right_Most_4x16.asm	\
	../ildb/Load_ILDB_Cntrl_Data_16DW.asm	\
	../ildb/Load_ILDB_Cntrl_Data_22DW.asm	\
	../ildb/Load_ILDB_Cntrl_Data_64DW.asm	\
	../ildb/Load_ILDB_Cntrl_Data.asm	\
	../ildb/load_Left_UV_2x8T.asm	\
	../ildb/load_Left_UV_2x8T_Mbaff.asm	\
	../ildb/load_Left_Y_4x16T.asm	\
	../ildb/load_Left_Y_4x16T_Mbaff.asm	\
	../ildb/loadNV12_16x16T.asm	\
	../ildb/loadNV12_16x4.asm	\
	../ildb/load_Top_UV_8x2.asm	\
	../ildb/load_Top_UV_8x2_Mbaff.asm	\
	../ildb/load_Top_Y_16x4.asm	\
	../ildb/load_Top_Y_16x4_Mbaff.asm	\
	../ildb/save_Cur_UV_8x8.asm	\
	../ildb/save_Cur_UV_8x8_Mbaff.asm	\
	../ildb/save_Cur_Y_16x16.asm	\
	../ildb/save_Cur_Y_16x16_Mbaff.asm	\
	../ildb/save_Left_UV_8x2T.asm	\
	../ildb/save_Left_UV_8x2T_Mbaff.asm	\
	../ildb/save_Left_Y_16x4T.asm	\
	../ildb/save_Left_Y_16x4T_Mbaff.asm	\
	../ildb/saveNV12_16x16.asm	\
	../ildb/saveNV12_16x4.asm	\
	../ildb/saveNV12_16x4T.asm	\
	../ildb/save_Top_UV_8x2.asm	\
	../ildb/save_Top_UV_8x2_Mbaff.asm	\
	../ildb/save_Top_Y_16x4.asm	\
	../ildb/save_Top_Y_16x4_Mbaff.asm	\
	../ildb/SetupVPKernel.asm	\
	../ildb/Transpose_Cur_UV_2x8.asm	\
	../ildb/Transpose_Cur_UV_8x8.asm	\
	../ildb/Transpose_Cur_UV_Right_Most_2x8.asm	\
	../ildb/Transpose_Cur_Y_16x16.asm	\
	../ildb/Transpose_Cur_Y_4x16.asm	\
	../ildb/Transpose_Cur_Y_Right_Most_4x16.asm	\
	../ildb/Transpose_Left_UV_2x8.asm	\
	../ildb/Transpose_Left_Y_4x16.asm	\
	../ildb/TransposeNV12_16x16.asm	\
	../ildb/TransposeNV12_4x16.asm	\
	../ildb/writeURB.asm	\
	../ildb/writeURB_UV_Child.asm	\
	../ildb/writeURB_Y_Child.asm

INTEL_MC_GEN5_ASM = avc_mc.gen5.asm

TARGETS  =
if HAVE_GEN4ASM
TARGETS += $(INTEL_MC_G4B_GEN5)
endif

all-local: $(TARGETS)

SUFFIXES = .g4a .g4b .gen5.asm

if HAVE_GEN4ASM
.g4a.g4b:
	$(AM_V_GEN)m4 $*.g4a > $*.g4m			&& \
	$(AM_V_GEN)$(GEN4ASM) -o $@ $*.g4m		&& \
	$(AM_V_GEN)$(GEN4ASM) -g 5 -o $@.gen5 $*.g4m	&& \
	rm $*.g4m

$(INTEL_MC_GEN5_ASM): $(INTEL_MC_ASM) $(INTEL_MC_INC) $(INTEL_ILDB_ASM)
	$(AM_V_GEN)cpp -DDEV_ILK -DBOOTSTRAP -I $(srcdir)/../ildb/ $(srcdir)/AllAVC.asm > _mc0.$@	&& \
	$(PYTHON2) $(top_srcdir)/src/shaders/gpp.py _mc0.$@ $@			&& \
	$(GEN4ASM) -l $(srcdir)/list -a -e tmp.$(INTEL_MC_EXPORT_GEN5) -g 5 $@	   \
		-o /dev/null						&& \
	mv tmp.$(INTEL_MC_EXPORT_GEN5) $(INTEL_MC_EXPORT_GEN5)		&& \
	cpp -DDEV_ILK -I $(srcdir)/../ildb/ $(srcdir)/AllAVC.asm > _mc1.$@	&& \
	$(PYTHON2) $(top_srcdir)/src/shaders/gpp.py _mc1.$@ $@			&& \
	rm _mc0.$@ _mc1.$@

$(INTEL_MC_G4B_GEN5): $(INTEL_MC_GEN5_ASM)
	$(AM_V_GEN)$(GEN4ASM) -l $(srcdir)/list -a -e tmp.$(INTEL_MC_EXPORT_GEN5) -g 5 $<	   \
		-o $@							&& \
	cat tmp.$(INTEL_MC_EXPORT_GEN5) | sed "s/_IP/_IP_GEN5/g"	   \
		> $(INTEL_MC_EXPORT_GEN5)				&& \
	rm tmp.$(INTEL_MC_EXPORT_GEN5)

$(INTEL_G4B): $(INTEL_G4I)
endif

CLEANFILES = $(INTEL_MC_GEN5_ASM)

EXTRA_DIST = \
	$(INTEL_G4A)		\
	$(INTEL_G4B)    	\
	$(INTEL_G4B_GEN5)	\
	$(INTEL_G4I)		\
	$(INTEL_MC_ASM)		\
	$(INTEL_MC_EXPORT)	\
	$(INTEL_MC_EXPORT_GEN5)	\
	$(INTEL_MC_G4B)		\
	$(INTEL_MC_G4B_GEN5)	\
	$(INTEL_MC_INC)		\
	list			\
	$(NULL)

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = Makefile.in
