CFLAGS = -D_USE_LIBM_MATH_H
CSFLAGS = $(CFLAGS)

THIS_SRCS = e_acosh.c e_acos.s e_asin.s e_atan2.s e_atanh.c e_cosh.c e_exp.s \
	ef_acos.c ef_acosh.c ef_asin.c ef_atan2.s ef_atanh.c ef_cosh.c ef_exp.s \
	ef_fmod.s ef_gamma.c ef_hypot.c ef_j0.c ef_j1.c ef_jn.c ef_lgamm.c ef_log10.s \
	ef_log.s e_fmod.s ef_pow.c ef_remai.s ef_rem_p.c ef_scalb.s ef_sinh.c \
	ef_sqrt.s e_gamma.c e_hypot.c e_j0.c e_j1.c e_jn.c e_lgamma.c e_log10.s \
	e_log.s e_pow.c e_remain.s e_rem_pi.c erf_gamm.c erf_lgam.c er_gamma.c \
	er_lgamm.c e_scalb.s e_sinh.c e_sqrt.s k_cos.c kf_cos.c kf_rem_p.c kf_sin.c \
	kf_tan.c k_rem_pi.c k_sin.c k_standa.c k_tan.c s_asinh.c s_atan.s s_cbrt.c \
	s_ceil.s s_copysi.s s_cos.s s_erf.c s_expm1.s s_fabs.c sf_asinh.c sf_atan.s \
	sf_cbrt.c sf_ceil.s sf_copys.s sf_cos.s sf_erf.c sf_expm1.s sf_fabs.c \
	sf_finit.s sf_floor.s sf_frexp.c sf_ilogb.s s_finite.s sf_isinf.c sf_isnan.c \
	sf_ldexp.c sf_log1p.s sf_logb.s s_floor.s sf_modf.c sf_nexta.c s_frexp.c \
	sf_rint.s sf_scalb.s sf_signi.s sf_sin.s sf_tanh.c sf_tan.s s_ilogb.s s_infini.c \
	s_isinf.c s_isnan.c s_ldexp.c s_libver.c s_log1p.s s_logb.s s_mather.c \
	s_modf.c s_nextaf.c s_rint.s s_scalbn.s s_signga.c s_signif.s s_sin.s \
	s_tanh.c s_tan.s w_acos.c w_acosh.c w_asin.c w_atan2.c w_atanh.c \
	w_cabs.c w_cosh.c w_drem.c w_exp.c wf_acos.c wf_acosh.c wf_asin.c \
	wf_atan2.c wf_atanh.c wf_cabs.c wf_cosh.c wf_drem.c wf_exp.c wf_fmod.c \
	wf_gamma.c wf_hypot.c wf_j0.c wf_j1.c wf_jn.c wf_lgamm.c wf_log10.c wf_log.c \
	w_fmod.c wf_pow.c wf_remai.c wf_scalb.c wf_sinh.c wf_sqrt.c w_gamma.c w_hypot.c \
	w_j0.c w_j1.c w_jn.c w_lgamma.c w_log10.c w_log.c w_pow.c w_remain.c \
	wrf_gamm.c wrf_lgam.c wr_gamma.c wr_lgamm.c w_scalb.c w_sinh.c \
	w_sqrt.c

include $(MENUET_LIBC_TOPDIR)/Make.rules

mk_lib: gen_tmp all
	make -f Makefile-link OUTFILE="libm.a"
ifdef ON_MINGW
	copy libm.a $(MENUETDEV)\lib
	del libm.a
else
	mv -f libm.a $(MENUETDEV)/lib
endif

dll: _gen_tmp all
	make -f Makefile-link-dll OUTFILE="libm.so"
ifdef ON_MINGW
	copy libm.so $(MENUETDEV)\lib
	del libm.so
else
	mv -f libm.so $(MENUETDEV)/lib
endif

_gen_tmp:
ifdef ON_MINGW
	@$(D_ECHO) > ..\tmp_make
else
	@$(D_ECHO) > ../tmp_make
endif

gen_tmp:
ifdef ON_MINGW
	@echo foo = bar> ../tmp_make
	@..\m_echo ..\tmp_make B_MENUET_LIBC_OBJS = 
else
	@echo "foo = bar" > ../tmp_make
	@../m_echo ../tmp_make B_MENUET_LIBC_OBJS = 
endif
