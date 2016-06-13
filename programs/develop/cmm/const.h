#define	FALSE		0
#define	TRUE		1
#define	LOCAL		-1
#define DYNAMIC_POST 2
#define DYNAMIC_VAR  3
#define USED_DIN_VAR 4

#define r_undef 0
#define r8 1
#define r16 2
#define r32 4
#define r64 8
#define r128 16

#define CODE16 0x100
#define CODE32 0x200
#define THIS_PARAM 0x300
#define THIS_REG   0x400
#define THIS_NEW   0x500
#define THIS_ZEROSIZE 0x600

enum{
	dm_other,
	dm_none,
	dm_def0,
	dm_def,
	dm_if,
};

enum{
	STDLEX,
	RESLEX,
	DEFLEX,
	DEFLEX2,
	ASMLEX,
};

enum{
	zero_term,
	dos_term,
	no_term,
	s_unicod=4
};

enum{
	NOTINITVAR,
	INITVAR,
	USEDVAR,
	INITUSEVAR
};

enum{
	tp_ucnovn,
	tp_declare,
	tp_localvar,
	tp_paramvar,
	tp_postvar,
	tp_gvar,
	tp_label,
	tp_stopper,
	tp_opperand,
	tp_compare,
	tp_modif,
	tp_ofs,
	tp_classvar,
};

enum{
	stdcompr,
	voidcompr=4,
	zerocompr=8,
	ecxzcompr=0xc,
	cxzcompr=0x10,
	ecxnzcompr=0x14,
	cxnzcompr=0x18
};

enum{
	tk_eof,         tk_number,      tk_string,      tk_id,          tk_ID,
	tk_assign,      tk_swap,        tk_minus,       tk_plus,        tk_minusminus,
	tk_plusplus,    tk_mult,        tk_div,         tk_mod,         tk_multminus,
	tk_divminus,    tk_modminus,    tk_rr,          tk_ll,          tk_rrminus,
  tk_llminus,     tk_minusequals, tk_plusequals,  tk_rrequals,    tk_llequals,
	tk_or,          tk_and,         tk_xor,         tk_not,         tk_orminus,
	tk_andminus,    tk_xorminus,    tk_orequals,    tk_andequals,   tk_xorequals,
	tk_equalto,     tk_notequal,    tk_greater,     tk_greaterequal,tk_less,
//40
	tk_lessequal,   tk_oror,        tk_andand,      tk_openbrace,   tk_closebrace,
	tk_openbracket, tk_closebracket,tk_openblock,   tk_closeblock,  tk_colon,

	tk_semicolon,   tk_camma,       tk_period,      tk_at,          tk_numsign,
  tk_dollar,      tk_question,    tk_tilda,
																							    tk_void,        tk_char,
//60
	tk_byte,        tk_int,         tk_word,        tk_long,        tk_dword,
	tk_float,       tk_qword,       tk_double,
								                                  tk_if,          tk_IF,
	tk_else,        tk_ELSE,        tk_loop,        tk_do,          tk_while,
	tk_return,      tk_from,        tk_extract,     tk_interrupt,   tk_far,
//80
	tk_enum,        tk_seg,         tk_debugreg,    tk_controlreg,  tk_testreg,
	tk_undefproc,   tk_proc,        tk_interruptproc,
								                                  tk_bits,        tk_charvar,
//90
	tk_bytevar,     tk_intvar,      tk_wordvar,     tk_longvar,     tk_dwordvar,
	tk_floatvar,    tk_qwordvar,    tk_doublevar,   tk_reg,         tk_beg,
//100
	tk_reg32,       tk_rmnumber,    tk_postnumber,  tk_locallabel,  tk_overflowflag,
	tk_notoverflowflag,tk_carryflag,tk_notcarryflag,tk_zeroflag,    tk_notzeroflag,
//110
	tk_comment1,    tk_comment2,		tk_minusflag,   tk_plusflag,    tk_RETURN,
	tk_dataptr,     tk_codeptr,     tk_postptr,     tk_stackstart,  tk_inline,
//120
	tk_GOTO,        tk_goto,        tk_BREAK,       tk_break,       tk_CONTINUE,
	tk_continue,    tk_WHILE,       tk_FOR,         tk_for,         tk_asm,
//130
	tk_switch,      tk_case,        tk_default,     tk_mmxreg,      tk_fpust,
	tk_multequals,  tk_divequals,   tk_struct,      tk_structvar,   tk_sizeof,
//140
	tk_undefofs,    tk_pointer,     tk_localpointer,tk_parampointer,tk_apiproc,
	tk_extern,      tk_declare,     tk_pascal,      tk_cdecl,       tk_stdcall,
//150
	tk_fastcall,    tk_union,       tk_export,      tk_multipoint,  tk_LOOPNZ,
	tk_loopnz,      tk_idasm,       tk_short,       tk_unsigned,    tk_signed,
//160
	tk_file,        tk_line,        tk_SWITCH,      tk_CASE,        tk_xmmreg,
	tk_dblcolon,    tk_static,      tk_baseclass,   tk_rescommand,  tk_endline,
//170
  tk_singlquote,	tk_new,         tk_delete,      tk_macro,       tk_reg64,
	tk_newline,     tk_apioffset,    tokens
};

// 16-bit word regs
#define	AX		0
#define	CX		1
#define	DX		2
#define	BX		3
#define	SP		4
#define	BP		5
#define	SI		6
#define	DI		7

// 8-bit byte regs
#define	AL		0
#define	CL		1
#define	DL		2
#define	BL		3
#define	AH		4
#define	CH		5
#define	DH		6
#define	BH		7

// 386+ 32-bit regs
#define	EAX		0
#define	ECX		1
#define	EDX		2
#define	EBX		3
#define	ESP		4
#define	EBP		5
#define	ESI		6
#define	EDI		7

//конроль разрушения регистров
#define dEAX 1
#define dECX 2
#define dEDX 4
#define dEBX 8
#define dESP 16
#define dEBP 32
#define dESI 64
#define dEDI 128

#define	ES		0
#define	CS		1
#define	SS		2
#define	DS		3
#define	FS		4
#define	GS		5
#define VARPOST 9
#define USEDSTR 10
// 386+ control regs
#define	CR0		0
#define	CR1		1
#define	CR2		2
#define	CR3		3
#define	CR4		4
#define	CR5		5
#define	CR6		6
#define	CR7		7

// 386+ test regs
#define	TR0		0
#define	TR1		1
#define	TR2		2
#define	TR3		3
#define	TR4		4
#define	TR5		5
#define	TR6		6
#define	TR7		7

// 386+ debug regs
#define	DR0		0
#define	DR1		1
#define	DR2		2
#define	DR3		3
#define	DR4		4
#define	DR5		5
#define	DR6		6
#define	DR7		7

//переменная, указатель, дальний указатель,сегментный указатель.
#define	variable	0
#define	pointer		1
//#define	farpointer	2

// define exit codes
#define	e_ok		0
#define	e_outofmemory	1
#define	e_cannotopeninput	2
#define	e_toomanyerrors	3
#define	e_internalerror	4
#define	e_noinputspecified	5
#define	e_unknowncommandline	6
#define e_entrynotfound 7
#define	e_outputtoobig	8
#define e_notcreateoutput 9
#define e_preprocess 10
#define	e_someerrors	13
#define	e_badinputfilename	14
#define	e_symbioerror	15
#define	e_manyinclude	16

#define	rm_mod00	0
#define	rm_mod01	64
#define	rm_mod10	128
#define	rm_mod11	192
#define	rm_d16		6
#define	rm_BXSI		0
#define	rm_BXDI		1
#define	rm_BPSI		2
#define	rm_BPDI		3
#define	rm_SI		4
#define	rm_DI		5
#define	rm_BP		6
#define	rm_BX		7
#define	rm_sib		4
#define	rm_d32		5
#define	rm_EBP		5

// posttype values for call or jump types or post variables
enum{
CALL_NONE,	//0
CALL_SHORT,	// 1
BREAK_SHORT,	// 2
CONTINUE_SHORT,	// 3
CALL_NEAR,	// 4
//CALL_NEARD,
JMP_NEAR,	// 5
BREAK_NEAR,	// 6
CONTINUE_NEAR,	// 7

CALL_32,	//8
//CALL_32D,
JMP_32,	// 9
BREAK_32,	// 10
CONTINUE_32,	// 11
UNDEF_OFSET,	//12
POST_STRING,	//13
POST_STRING32,
CALL_EXT,	// 14
EXT_VAR,	// 15

CALL_32I,	// 16

POST_VAR,	// 17
FIX_VAR,	// 18
FIX_CODE,	// 19

POST_VAR32,	// 20
FIX_VAR32,	// 21
FIX_CODE32,	// 22

POST_FLOATNUM,	// 23
DATABLOCK_VAR,	//24
DATABLOCK_VAR32,	//25
DIN_VAR,	//26
DIN_VAR32,	 //27

CODE_SIZE,
CODE_SIZE32,
DATA_SIZE,
DATA_SIZE32,
POST_SIZE,
POST_SIZE32,
STACK_SIZE,
STACK_SIZE32,

FIX_CODE_ADD,
//DATABLOCK_STRING,	// 26
//DATABLOCK_STRING32	// 27
};

#define POINTER 0x8000
#define NOTPOINTER (~POINTER)

// format of output file
#define	file_exe	0
#define	file_com	1
#define	file_sys	2
#define	file_rom	3
#define	file_w32	4
#define file_d32	5
#define file_meos 6
#define file_bin  7

//типы моделей памяти
#define	TINY		0
#define	SMALL		1


//флаги
#define	f_reloc		1	//адрес может измениться
#define	f_typeproc	6	//тип вызова процедуры
#define	f_far		8	//дальняя процедура
#define	f_extern	0x10	//внешняя процедура, переменная
#define	f_interrupt	0x20	//прерывание
#define	f_export	0x40	//экспортируемая процедура
#define f_inline  0x80	//inline процедура
#define f_retproc 0xf00	//возврат флага из процедуры
#define f_static  0x1000	//
#define f_classproc 0x2000	//процедура из класса
//флаги процедур в структурах
#define fs_constructor 0x4000
#define fs_destructor 0x8000

#define f_useidx 0x10000	//переменная использует индекс []

//типы процедур
#define	tp_pascal	0
#define	tp_cdecl	2
#define	tp_stdcall	4
#define	tp_fastcall	6

//типы вызова API процедур
#define	API_FAST	1
#define	API_JMP		2

#define USEALLREG    8//0
//#define USEFIRST4REG 1
//#define USEONLY_AX   2

//нумерация списка директив
enum{
	d_ctrl,  d_jump, d_command,d_argc, d_resize,
	d_resmes,d_stack,d_start,	 d_atr,  d_name,
	d_com,   d_atex, d_dseg,   d_rsize,d_mdr,
	d_stm,   d_fca,  d_suv,    d_us,   d_ib,
	d_end1,

	d_align=d_end1, d_aligner,d_alignw,   //d_beep,
	d_code,  d_define, d_DOS,   d_endif,  d_ifdef,
	d_ifndef,d_incl,   d_error, /*d_pause,*/  d_print,
	d_prnex, d_random, d_speed, d_8086,   d_8088,
	d_80186, d_80286,  d_80386, d_80486,  d_80586,
	d_80686, d_80786,  d_sdp,   d_warning,d_ip,
	d_iav,   d_am32,   d_undef,	d_alignc,
	d_fut,   d_dstr,   d_cv,   	d_else, 	d_wmb,
	d_pragma,d_inline, d_if,    d_elif,   d_end};

enum{
	a_add,a_or, a_adc,a_sbb,a_and, a_sub, a_xor, a_cmp,
	a_not,a_neg,a_mul,a_div=a_mul+2,a_idiv,

	a_rol,a_ror,a_rcl,a_rcr,a_shl, a_shr, a_sar=a_shr+2,

	a_bt, a_bts,a_btr,a_btc,

	a_inc,a_dec,

	a_test,a_imul,

	a_shld,a_shrd,

	a_daa,a_das,a_aaa,a_aas,a_aam, a_aad,
  a_movzx,a_movsx,a_cbw,a_cwde,a_cwd,a_cdq,
	a_bswap,a_xlat,

	a_bsf,a_bsr,

	a_cmpxchg,a_cmpxchg8b,a_xadd,
	a_nop,a_wait,a_lock,a_hlt,a_int,
	a_into,a_iret,a_iretd,
	a_popf,a_popfd,a_pushf,a_pushfd,a_sahf,a_lahf,
	a_cmc,a_clc,a_stc,a_cli,a_sti,a_cld,a_std,
	a_push,a_pusha,a_pushad,a_pop,a_popa,a_popad,
	a_xchg,a_mov,a_lea,

  a_lfs,a_lgs,a_lss,

	a_les,a_lds,

	a_adrsiz,
	a_in,a_out,a_insb,a_insw,a_insd,a_outsb,a_outsw,a_outsd,
	a_movsb,a_movsw,a_movsd,a_cmpsb,a_cmpsw,a_cmpsd,
	a_stosb,a_stosw,a_stosd,a_lodsb,a_lodsw,a_lodsd,
	a_scasb,a_scasw,a_scasd,a_repnz,a_rep,
	a_jcxz,a_jecxz,a_loop,a_loopd,a_loopz,a_loopnz,

	a_jo,a_jno,a_jc,a_jnc,a_jz,a_jnz,a_jna,a_ja,
	a_js,a_jns,a_jp,a_jnp,a_jl,a_jnl,a_jng,a_jg,

	a_seto,a_setno,a_setc,a_setnc,a_setz,a_setnz,a_setna,a_seta,
	a_sets,a_setns,a_setp,a_setnp,a_setl,a_setnl,a_setng,a_setg,

	/*a_jmps,a_jmpn,a_jmpf,*/a_jmp,
	a_call,/*a_callf,*/a_ret,a_retf,
	a_enter,a_leave,a_bound,a_arpl,
	a_sldt,a_str,a_lldt,a_ltr,a_verr,a_verw,

	a_lar,a_lsl,

	a_sgdt,a_sidt,a_lgdt,a_lidt,a_smsw,a_lmsw,a_clts,
	a_invd,a_wbinvd,//a_invlpd,
	a_wrmsr,a_cpuid,a_rdmsr,a_rdtsc,a_rsm,
	a_rdpmc,a_ud2,/*a_emmx,a_setalc,*/

	a_punpcklbw,a_punpcklwd,a_punpckldq,
	a_packsswb,
	a_pcmpgtb,a_pcmpgtw,a_pcmpgtd,
	a_packuswb,
	a_punpckhbw,a_punpckhwd,a_punpckhdq,
  a_packssdw,
	a_psrlw,a_psrld,a_psrlq,
	a_psraw,a_psrad,
	a_psllw,a_pslld,a_psllq,
	a_pcmpeqb,a_pcmpeqw,a_pcmpeqd,
	a_pmullw,
	a_movd,a_movq,
	a_psubusb,a_psubusw,
	a_emms,
	a_pand,
	a_paddusb,a_paddusw,
	a_pandn,a_pmulhw,
	a_psubsb,a_psubsw,
	a_por,
	a_paddsb,a_paddsw,
	a_pxor,
	a_pmaddwd,
	a_psubb,a_psubw,a_psubd,
	a_paddb,a_paddw,a_paddd,

	a_db,a_dw,a_dd,a_invlpg,a_loadall,a_opsiz,
	a_f2xm1,a_fabs,a_fadd,a_faddp,a_fbld,a_fbstp,a_fchs,a_fclex,a_fcom,
	a_fcomp,a_fnclex,a_fcompp,a_fcos,a_fdecstr,a_fdisi,a_fdiv,a_fdivp,
	a_fdivr,a_fdivrp,a_ffree,a_fiadd,a_ficom,a_ficomp,a_fidiv,a_fidivr,
	a_fild,a_fildq,a_fimul,a_fist,a_fistp,a_fisub,a_fisubr,a_feni,a_fincstr,
	a_finit,a_fninit,a_fld,a_fldcw,a_fldenv,a_fldlg2,a_fldln2,a_fldl2e,
	a_fldl2t,a_fldpi,a_fldz,a_fld1,a_fmul,a_fmulp,a_fnop,a_fpatan,a_fprem,
	a_fprem1,a_fptan,a_frndint,a_fsetpm,a_frstor,a_fsave,a_fnsave,a_fscale,
	a_fsin,a_fsincos,a_fsqrt,a_fst,a_fstcw,a_fnstcw,a_fstp,a_fstsw,a_fnstsw,
	a_fstenv,a_fnstenv,a_fsub,a_fsubp,a_fsubr,a_fsubrp,a_ftst,a_fucom,a_fucomp,
	a_fucompp,a_fxch,a_fwait,a_fxam,a_fxtract,a_fyl2x,a_fyl2xp1,
	a_sysenter,a_sysexit,a_fcmovb,a_fcmove,a_fcmovbe,a_fcmovu,a_fcmovnb,
	a_fcmovne,a_fcmovnbe,a_fcmovnu,a_fcomi,a_fcomip,a_fucomi,a_fucomip,
	a_fxrstor,a_fxsave,a_fndisi,a_fneni,a_fnsetpm,

	a_cmovo,a_cmovno,a_cmovc,a_cmovnc,a_cmovz,a_cmovnz,a_cmovna,a_cmova,
	a_cmovs,a_cmovns,a_cmovp,a_cmovnp,a_cmovl,a_cmovnl,a_cmovng,a_cmovg,

//MMX Pentium III extention
	a_maskmovq,  a_movntq,    a_pavgb,      a_pavgw,      a_pextrw,  a_pinsrw,
	a_pmaxub,    a_pmaxsw,    a_pminub,     a_pminsw,     a_pmovmskb,a_pmulhuw,
	a_prefetcht0,a_prefetcht1,a_prefetcht2, a_prefetchnta,a_sfence,  a_psadbw,
	a_pshufw,
//XMM extentions Pentium III
	a_addps,    a_addss,   a_andnps,  a_andps,   a_cmpps,   a_cmpss,
	a_comiss,   a_cvtpi2ps,a_cvtps2pi,a_cvtsi2ss,a_cvtss2si,a_cvttps2pi,
	a_cvttss2si,a_divps,   a_divss,   a_ldmxcsr, a_maxps,   a_maxss,
	a_minps,    a_minss,   a_movaps,  a_movhlps, a_movhps,  a_movlhps,
	a_movlps,   a_movmskps,a_movss,   a_movups,  a_mulps,   a_mulss,
	a_movntps,  a_orps,    a_rcpps,   a_rcpss,   a_rsqrtps, a_rsqrtss,
	a_shufps,   a_sqrtps,  a_sqrtss,  a_stmxcsr, a_subps,   a_subss,
	a_ucomiss,  a_unpckhps,a_unpcklps,a_xorps,

// Pentium IV
	a_lfence,  a_mfence,    a_addpd,     a_addsd,     a_andpd,     a_andnpd,
	a_cmppd,   a_comisd,    a_cvtdq2pd,  a_cvtdq2ps,  a_cvtpd2dq,  a_cvtpd2pi,
	a_cvtpd2ps,a_cvtpi2pd,  a_cvtps2dq,  a_cvtps2pd,  a_cvtsd2si,  a_cvtsd2ss,
	a_cvtsi2sd,a_cvtss2sd,  a_cvttpd2pi, a_cvttpd2dq, a_cvttps2dq, a_cvttsd2si,
	a_divpd,   a_divsd,     a_maskmovdqu,a_maxpd,     a_maxsd,     a_minpd,
	a_minsd,   a_movapd,    a_movdqa,    a_movdqu,    a_movdq2q,   a_movhpd,
	a_movlpd,  a_movmskpd,  a_movntdq,   a_movntpd,   a_movnti,    a_movq2dq,
	a_movupd,  a_mulpd,     a_mulsd,     a_orpd,      a_pshufd,    a_pshufhw,
	a_pshuflw, a_pslldq,    a_psrldq,    a_shufpd,    a_sqrtpd,    a_sqrtsd,
	a_subpd,   a_subsd,     a_ucomisd,   a_unpckhpd,  a_unpcklpd,  a_xorpd,
	a_paddq,   a_pmuludq,   a_psubq,     a_punpckhqdq,a_punpcklqdq,a_clflush,
	a_monitor, a_mwait,     a_addsubpd,  a_addsubps,  a_cmpeqsd,   a_cmpltsd,
	a_cmplesd, a_cmpunordsd,a_cmpneqsd,  a_cmpnltsd,  a_cmpnlesd,  a_cmpordsd,
	a_cmpeqpd, a_cmpltpd,   a_cmplepd,   a_cmpunordpd,a_cmpneqpd,  a_cmpnltpd,
	a_cmpnlepd,a_cmpordpd,  a_cmpeqps,   a_cmpltps,   a_cmpleps,   a_cmpunordps,
	a_cmpneqps,a_cmpnltps,  a_cmpnleps,  a_cmpordps,  a_cmpeqss,   a_cmpltss,
	a_cmpless, a_cmpunordss,a_cmpneqss,  a_cmpnltss,  a_cmpnless,  a_cmpordss,
	a_haddpd,  a_haddps,    a_hsubpd,    a_hsubps,    a_lddqu,     a_movddup,
	a_movshdup,a_movsldup,  a_pause,

	a_end};

enum{
	pF3=1,
	pF2,
	p66
};

