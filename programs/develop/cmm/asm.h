#define NUM_ASM_MODIF 12

char *asmmodif[]={
	"FAR",  "SHORT",  "NEAR", "DUP",   "INT","WORD","LONG","DWORD",
	"TBYTE","LDOUBLE","QWORD","DOUBLE"
};
//слова модификаторы
#define m_far   1
#define m_short 2
#define m_near  4
#define m_dup   8
#define m_int   16
#define m_word  32
#define m_long  64
#define m_dword 128
#define m_tbyte 256
#define m_ldouble 512
#define m_qword   1024
#define m_double  2048

//конроль разрушения регистров
#define d1par 256	//по первому операнду
#define d2par 512	//по 2 операнду

struct ASMPAR
{
	unsigned char num;  //число операндов младшие 4 бита минимальное число,
	                    // старшие 4 максимальное, если 0, то только младшее,
                            // если 15, то неограничено
	unsigned char chip; // необходимый cpu
	unsigned short reg; // разрушаемые регистры
	unsigned short mod; // слово-модификатор
}asmpar[]={
	2,0,d1par,0,	//a_add
	2,0,d1par,0,	//a_or
	2,0,d1par,0,	//a_adc
	2,0,d1par,0,	//a_sbb
	2,0,d1par,0,	//a_and
	2,0,d1par,0,	//a_sub
	2,0,d1par,0,	//a_xor
	2,0,d1par,0,	//a_cmp
	1,0,d1par,0,	//a_not
	1,0,d1par,0,	//a_neg
	1,0,dEAX|dEDX,0,	//a_mul
	0,0,0,0,
	1,0,dEAX|dEDX,0,	//a_div=a_mul+2
	1,0,dEAX|dEDX,0,	//a_idiv
	2,0,d1par,0,	//a_rol
	2,0,d1par,0,	//a_ror
	2,0,d1par,0,	//a_rcl
	2,0,d1par,0,	//a_rcr
	2,0,d1par,0,	//a_shl
	2,0,d1par,0,	//a_shr
	0,0,0,0,
	2,0,d1par,0,	//a_sar=a_shr+2
	2,3,0,0,	//a_bt
	2,3,d1par,0,	//a_bts
	2,3,d1par,0,	//a_btr
	2,3,d1par,0,	//a_btc
	1,0,d1par,0,	//a_inc
	1,0,d1par,0,	//a_dec
	2,0,0,0,	//a_test
	0x31,0,d1par|dEAX|dEDX,0,	//a_imul
	3,3,d1par,0,	//a_shld
	3,3,d1par,0,	//a_shrd
	0,0,dEAX,0,	//a_daa
	0,0,dEAX,0,	//a_das
	0,0,dEAX,0,	//a_aaa
	0,0,dEAX,0,	//a_aas
	0x10,0,dEAX,0,	//a_aam
	0x10,0,dEAX,0,	//a_aad
	2,3,d1par,0,	//a_movzx
	2,3,d1par,0,	//a_movsx
	0,0,dEAX,0,	//a_cbw
	0,3,dEAX,0,	//a_cwde
	0,0,dEDX,0,	//a_cwd
	0,3,dEDX,0,	//a_cdq,
	1,4,d1par,0,	//a_bswap
	0,0,dEAX,0,	//a_xlat
	2,3,d1par,0,	//a_bsf
	2,3,d1par,0,	//a_bsr
	2,4,d1par|dEAX,0,	//a_cmpxchg
	1,5,d1par|dEDX|dEAX,0,	//a_cmpxchg8b
	2,4,d1par|d2par,0,	//a_xadd
	0,0,0,0,	//a_nop
	0,0,0,0,	//a_wait
	0,0,0,0,	//a_lock
	0,0,0,0,	//a_hlt
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,0,	//a_int
	0,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,0,	//a_into
	0,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,0,	//a_iret
	0,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,0,	//a_iretd
	0,0,0,0,	//a_popf
	0,3,0,0,	//a_popfd
	0,0,0,0,	//a_pushf
	0,3,0,0,	//a_pushfd
	0,0,0,0,	//a_sahf
	0,0,dEAX,0,	//a_lahf
	0,0,0,0,	//a_cmc
	0,0,0,0,	//a_clc
	0,0,0,0,	//a_stc
	0,0,0,0,	//a_cli
	0,0,0,0,	//a_sti
	0,0,0,0,	//a_cld
	0,0,0,0,	//a_std
	0xF1,0,0,m_int|m_word|m_long|m_dword,	//a_push
	0,2,0,0,	//a_pusha
	0,3,0,0,	//a_pushad
	0xF1,0,d1par,0,	//a_pop
	0,2,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,0,	//a_popa
	0,3,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,0,	//a_popad
	2,0,d1par|d2par,0,	//a_xchg
	2,0,d1par,0,	//a_mov
	2,0,d1par,0,	//a_lea
	2,3,d1par,0,	//a_lfs
	2,3,d1par,0,	//a_lgs
	2,3,d1par,0,	//a_lss
	2,0,d1par,0,	//a_les
	2,0,d1par,0,	//a_lds
	0,3,0,0,	//a_adrsiz
	2,0,dEAX,0,	//a_in
	2,0,0,0,	//a_out
	0,2,dEAX|dEDI,0,	//a_insb
	0,2,dEAX|dEDI,0,	//a_insw
	0,3,dEAX|dEDI,0,	//a_insd
	0,2,dESI,0,	//a_outsb
	0,2,dESI,0,	//a_outsw
	0,3,dESI,0,	//a_outsd
	0,0,dEDI|dESI,0,	//a_movsb
	0,0,dEDI|dESI,0,	//a_movsw
	0x20,3,dEDI|dESI,0,	//a_movsd
	0,0,dEDI|dESI,0,	//a_cmpsb
	0,0,dEDI|dESI,0,	//a_cmpsw
	0x20,3,dEDI|dESI,0,	//a_cmpsd
	0,0,dEDI,0,	//a_stosb
	0,0,dEDI,0,	//a_stosw
	0,3,dEDI,0,	//a_stosd
	0,0,dEAX|dESI,0,	//a_lodsb
	0,0,dEAX|dESI,0,	//a_lodsw
	0,3,dEAX|dESI,0,	//a_lodsd
	0,0,dEDI,0,	//a_scasb
	0,0,dEDI,0,	//a_scasw
	0,3,dEDI,0,	//a_scasd
	0,0,dECX,0,	//a_repnz
	0,0,dECX,0,	//a_rep
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jcxz
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jecxz
	1,0,dECX,0,	//a_loop
	1,3,dECX,0,	//a_loopd
	1,0,dECX,0,	//a_loopz
	1,0,dECX,0,	//a_loopnz
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jo
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jno
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jc
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jnc
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jz
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jnz
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jna
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_ja
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_js
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jns
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jp
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jnp
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jl
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jnl
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jng
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jg
	1,3,d1par,0,	//a_seto
	1,3,d1par,0,	//a_setno
	1,3,d1par,0,	//a_setc
	1,3,d1par,0,	//a_setnc
	1,3,d1par,0,	//a_setz
	1,3,d1par,0,	//a_setnz
	1,3,d1par,0,	//a_setna
	1,3,d1par,0,	//a_seta
	1,3,d1par,0,	//a_sets
	1,3,d1par,0,	//a_setns
	1,3,d1par,0,	//a_setp
	1,3,d1par,0,	//a_setnp
	1,3,d1par,0,	//a_setl
	1,3,d1par,0,	//a_setnl
	1,3,d1par,0,	//a_setng
	1,3,d1par,0,	//a_setg
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near|m_short,	//a_jmp
	1,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,m_far|m_near,	//a_call/*a_callf,*/
	0x10,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,0,	//a_ret
	0x10,0,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,0,	//a_retf
	2,2,dEBP|dESP,0,	//a_enter
	0,2,dEBP|dESP,0,	//a_leave
	2,2,dEAX|dEBX|dECX|dEDX|dESI|dEDI|dEBP|dESP,0,	//a_bound
	2,2,d1par,0,	//a_arpl
	1,2,d1par,0,	//a_sldt
	1,2,d1par,0,	//a_str
	1,2,0,0,	//a_lldt
	1,2,0,0,	//a_ltr
	1,2,0,0,	//a_verr
	1,2,0,0,	//a_verw
	2,2,d1par,0,	//a_lar
	2,2,d1par,0,	//a_lsl
	1,2,d1par,0,	//a_sgdt
	1,2,d1par,0,	//a_sidt
	1,2,0,0,	//a_lgdt
	1,2,0,0,	//a_lidt
	1,2,d1par,0,	//a_smsw
	1,2,0,0,	//a_lmsw
	0,2,0,0,	//a_clts
	0,4,0,0,	//a_invd
	0,4,0,0,	//a_wbinvd
	//a_invlpd,
	0,5,0,0,	//a_wrmsr
	0,5,dEAX|dEBX|dECX|dEDX,0,	//a_cpuid
	0,5,dEAX|dEDX,0,	//a_rdmsr
	0,5,dEAX|dEDX,0,	//a_rdtsc
	0,5,0,0,	//a_rsm
	0,7,dEAX|dEDX,0,	//a_rdpmc
	0,7,0,0,	//a_ud2
	/*a_emmx,a_setalc,*/
	2,8,0,0,	//a_punpcklbw
	2,8,0,0,	//a_punpcklwd
	2,8,0,0,	//a_punpckldq
	2,8,0,0,	//a_packsswb
	2,8,0,0,	//a_pcmpgtb
	2,8,0,0,	//a_pcmpgtw
	2,8,0,0,	//a_pcmpgtd
	2,8,0,0,	//a_packuswb
	2,8,0,0,	//a_punpckhbw
	2,8,0,0,	//a_punpckhwd
	2,8,0,0,	//a_punpckhdq
	2,8,0,0,	//a_packssdw
	2,6,0,0,	//a_psrlw
	2,6,0,0,	//a_psrld
	2,6,0,0,	//a_psrlq
	2,6,0,0,	//a_psraw
	2,6,0,0,	//a_psrad
	2,6,0,0,	//a_psllw
	2,6,0,0,	//a_pslld
	2,6,0,0,	//a_psllq
	2,8,0,0,	//a_pcmpeqb
	2,8,0,0,	//a_pcmpeqw
	2,8,0,0,	//a_pcmpeqd
	2,8,0,0,	//a_pmullw
	2,6,d1par,0,	//a_movd
	2,6,d1par,0,	//a_movq
	2,8,0,0,	//a_psubusb
	2,8,0,0,	//a_psubusw
	0,6,0,0,	//a_emms
	2,8,0,0,	//a_pand
	2,8,0,0,	//a_paddusb
	2,8,0,0,	//a_paddusw
	2,8,0,0,	//a_pandn
	2,8,0,0,	//a_pmulhw
	2,8,0,0,	//a_psubsb
	2,8,0,0,	//a_psubsw
	2,8,0,0,	//a_por
	2,8,0,0,	//a_paddsb
	2,8,0,0,	//a_paddsw
	2,8,0,0,	//a_pxor
	2,8,0,0,	//a_pmaddwd
	2,8,0,0,	//a_psubb
	2,8,0,0,	//a_psubw
	2,8,0,0,	//a_psubd
	2,8,0,0,	//a_paddb
	2,8,0,0,	//a_paddw
	2,8,0,0,	//a_paddd
	0xF1,0,0,m_dup,	//a_db
	0xF1,0,0,m_dup,	//a_dw
	0xF1,3,0,m_dup,	//a_dd
	1,4,0,0,	//a_invlpg
	0,2,0,0,	//a_loadall
	0,3,0,0,	//a_opsiz
	0,2,0,0,	//a_f2xm1
	0,0,0,0,	//a_fabs
	0x21,0,0,m_double,	//a_fadd
	0x20,0,0,0,	//a_faddp
	1,0,0,m_qword|m_tbyte,	//a_fbld
	1,0,d1par,m_qword|m_tbyte,	//a_fbstp
	0,0,0,0,	//a_fchs
	0,0,0,0,	//a_fclex
	0x10,0,0,m_double,	//a_fcom
	0x10,0,0,m_double,	//a_fcomp
	0,0,0,0,	//a_fnclex
	0,0,0,0,	//a_fcompp
	0,3,0,0,	//a_fcos
	0,0,0,0,	//a_fdecstr
	0,0,0,0,	//a_fdisi
	0x21,0,0,m_double,	//a_fdiv
	0x20,0,0,0,	//a_fdivp
	0x21,0,0,m_double,	//a_fdivr
	0x20,0,0,0,	//a_fdivrp
	1,0,0,0,	//a_ffree
	1,0,0,0,	//a_fiadd
	1,0,0,0,	//a_ficom
	1,0,0,0,	//a_ficomp
	1,0,0,0,	//a_fidiv
	1,0,0,0,	//a_fidivr
	1,0,0,0,	//a_fild
	1,0,0,m_qword|m_tbyte,	//a_fildq
	1,0,0,0,	//a_fimul
	1,0,d1par,0,	//a_fist
	1,0,d1par,0,	//a_fistp
	1,0,0,0,	//a_fisub
	1,0,0,0,	//a_fisubr
	0,0,0,0,	//a_feni
	0,0,0,0,	//a_fincstr
	0,0,0,0,	//a_finit
	0,0,0,0,	//a_fninit
	1,0,0,m_qword|m_double|m_tbyte|m_ldouble,	//a_fld
	1,0,0,0,	//a_fldcw
	1,0,0,m_tbyte|m_qword,	//a_fldenv
	0,0,0,0,	//a_fldlg2
	0,0,0,0,	//a_fldln2
	0,0,0,0,	//a_fldl2e
	0,0,0,0,	//a_fldl2t
	0,0,0,0,	//a_fldpi
	0,0,0,0,	//a_fldz
	0,0,0,0,	//a_fld1
	0x21,0,0,0,	//a_fmul
	0x20,0,0,0,	//a_fmulp
	0,0,0,0,	//a_fnop
	0,0,0,0,	//a_fpatan
	0,0,0,0,	//a_fprem
	0,3,0,0,	//a_fprem1
	0,0,0,0,	//a_fptan
	0,0,0,0,	//a_frndint
	0,2,0,0,	//a_fsetpm
	1,0,0,m_tbyte|m_qword,	//a_frstor
	1,0,d1par,m_tbyte|m_qword,	//a_fsave
	1,0,d1par,m_tbyte|m_qword,	//a_fnsave
	0,0,0,0,	//a_fscale
	0,3,0,0,	//a_fsin
	0,3,0,0,	//a_fsincos
	0,0,0,0,	//a_fsqrt
	1,0,d1par,m_qword|m_double|m_tbyte|m_ldouble,	//a_fst
	1,0,d1par,0,	//a_fstcw
	1,0,d1par,0,	//a_fnstcw
	1,0,d1par,m_qword|m_double|m_tbyte|m_ldouble,	//a_fstp
	1,0,d1par,0,	//a_fstsw
	1,0,d1par,0,	//a_fnstsw
	1,0,d1par,m_tbyte|m_qword,	//a_fstenv
	1,0,d1par,m_tbyte|m_qword,	//a_fnstenv
	0x21,0,0,m_double,	//a_fsub
	0x20,0,0,0,	//a_fsubp
	0x21,0,0,m_double,	//a_fsubr
	0x20,0,0,0,	//a_fsubrp
	0,0,0,0,	//a_ftst
	0x10,0,0,0,	//a_fucom
	0x10,0,0,0,	//a_fucomp
	0,3,0,0,	//a_fucompp
	0x10,0,0,0,	//a_fxch
	0,0,0,0,	//a_fwait
	0,0,0,0,	//a_fxam
	0,0,0,0,	//a_fxtract
	0,0,0,0,	//a_fyl2x
	0,0,0,0,	//a_fyl2xp1
	0,7,0,0,	//a_sysenter
	0,7,0,0,	//a_sysexit
	0x21,7,0,0,	//a_fcmovb
	0x21,7,0,0,	//a_fcmove
	0x21,7,0,0,	//a_fcmovbe
	0x21,7,0,0,	//a_fcmovu
	0x21,7,0,0,	//a_fcmovnb
	0x21,7,0,0,	//a_fcmovne
	0x21,7,0,0,	//a_fcmovnbe
	0x21,7,0,0,	//a_fcmovnu
	0x21,7,0,0,	//a_fcomi
	0x21,7,0,0,	//a_fcomip
	0x21,7,0,0,	//a_fucomi
	0x21,7,0,0,	//a_fucomip
	1,8,0,0,	//a_fxrstor
	1,8,d1par,0,	//a_fxsave
	0,0,0,0,	//a_fndisi
	0,0,0,0,	//a_fneni
	0,2,0,0,	//a_fnsetpm
	2,7,d1par,0,	//a_cmovo
	2,7,d1par,0,	//a_cmovno
	2,7,d1par,0,	//a_cmovc
	2,7,d1par,0,	//a_cmovnc
	2,7,d1par,0,	//a_cmovz
	2,7,d1par,0,	//a_cmovnz
	2,7,d1par,0,	//a_cmovna
	2,7,d1par,0,	//a_cmova
	2,7,d1par,0,	//a_cmovs
	2,7,d1par,0,	//a_cmovns
	2,7,d1par,0,	//a_cmovp
	2,7,d1par,0,	//a_cmovnp
	2,7,d1par,0,	//a_cmovl
	2,7,d1par,0,	//a_cmovnl
	2,7,d1par,0,	//a_cmovng
	2,7,d1par,0,	//a_cmovg
	2,8,dEDI,0,	//a_maskmovq
	2,8,d1par,0,	//a_movntq
	2,8,0,0,	//a_pavgb
	2,8,0,0,	//a_pavgw
	3,8,d1par,0,	//a_pextrw
	3,8,0,0,	//a_pinsrw
	2,8,0,0,	//a_pmaxub
	2,8,0,0,	//a_pmaxsw
	2,8,0,0,	//a_pminub
	2,8,0,0,	//a_pminsw
	2,8,d1par,0,	//a_pmovmskb
	2,8,0,0,	//a_pmulhuw
	1,8,0,0,	//a_prefetcht0
	1,8,0,0,	//a_prefetcht1
	1,8,0,0,	//a_prefetcht2
	1,8,0,0,	//a_prefetchnta
	0,8,0,0,	//a_sfence
	2,8,0,0,	//a_psadbw
	3,8,0,0,	//a_pshufw
	2,8,0,0,	//a_addps
	2,8,0,0,	//a_addss
	2,8,0,0,	//a_andnps
	2,8,0,0,	//a_andps
	3,8,0,0,	//a_cmpps
	3,8,0,0,	//a_cmpss
	2,8,0,0,	//a_comiss
	2,8,0,0,	//a_cvtpi2ps
	2,8,0,0,	//a_cvtps2pi
	2,8,0,0,	//a_cvtsi2ss
	2,8,d1par,0,	//a_cvtss2si
	2,8,0,0,	//a_cvttps2pi
	2,8,d1par,0,	//a_cvttss2si
	2,8,0,0,	//a_divps
	2,8,0,0,	//a_divss
	1,8,0,0,	//a_ldmxcsr
	2,8,0,0,	//a_maxps
	2,8,0,0,	//a_maxss
	2,8,0,0,	//a_minps
	2,8,0,0,	//a_minss
	2,8,d1par,0,	//a_movaps
	2,8,0,0,	//a_movhlps
	2,8,d1par,0,	//a_movhps
	2,8,0,0,	//a_movlhps
	2,8,d1par,0,	//a_movlps
	2,8,d1par,0,	//a_movmskps
	2,8,d1par,0,	//a_movss
	2,8,d1par,0,	//a_movups
	2,8,0,0,	//a_mulps
	2,8,0,0,	//a_mulss,
	2,8,d1par,0,	//a_movntps
	2,8,0,0,	//a_orps
	2,8,0,0,	//a_rcpps
	2,8,0,0,	//a_rcpss
	2,8,0,0,	//a_rsqrtps
	2,8,0,0,	//a_rsqrtss
	3,8,0,0,	//a_shufps
	2,8,0,0,	//a_sqrtps
	2,8,0,0,	//a_sqrtss
	1,8,d1par,0,	//a_stmxcsr
	2,8,0,0,	//a_subps
	2,8,0,0,	//a_subss
	2,8,0,0,	//a_ucomiss
	2,8,0,0,	//a_unpckhps
	2,8,0,0,	//a_unpcklps
	2,8,0,0,	//a_xorps
// Pentium IV
	0,9,0,0,	//a_lfence
	0,9,0,0,	//a_mfence
	2,9,0,0,	//a_addpd
	2,9,0,0,	//a_addsd
	2,9,0,0,	//a_andpd
	2,9,0,0,	//a_andnpd
	3,9,0,0,	//a_cmppd
	2,9,0,0,	//a_comisd
	2,9,0,0,	//a_cvtdq2pd
	2,9,0,0,	//a_cvtdq2ps
	2,9,0,0,	//a_cvtpd2dq
	2,9,0,0,	//a_cvtpd2pi
	2,9,0,0,	//a_cvtpd2ps
	2,9,0,0,	//a_cvtpi2pd
	2,9,0,0,	//a_cvtps2dq
	2,9,0,0,	//a_cvtps2pd
	2,9,d1par,0,	//a_cvtsd2si
	2,9,0,0,	//a_cvtsd2ss
	2,9,0,0,	//a_cvtsi2sd
	2,9,0,0,	//a_cvtss2sd
	2,9,0,0,	//a_cvttpd2pi
	2,9,0,0,	//a_cvttpd2dq
	2,9,0,0,	//a_cvttps2dq
	2,9,d1par,0,	//a_cvttsd2si
	2,9,0,0,	//a_divpd
	2,9,0,0,	//a_divsd
	2,9,0,0,	//a_maskmovdqu
	2,9,0,0,	//a_maxpd
	2,9,0,0,	//a_maxsd
	2,9,0,0,	//a_minpd
	2,9,0,0,	//a_minsd
	2,9,d1par,0,	//a_movapd
	2,9,d1par,0,	//a_movdqa
	2,9,d1par,0,	//a_movdqu
	2,9,0,0,	//a_movdq2q
	2,9,d1par,0,	//a_movhpd
	2,9,d1par,0,	//a_movlpd
	2,9,d1par,0,	//a_movmskpd
	2,9,d1par,0,	//a_movntdq
	2,9,d1par,0,	//a_movntpd
	2,9,d1par,0,	//a_movnti
	2,9,0,0,	//a_movq2dq
	2,9,0,0,	//a_movupd
	2,9,0,0,	//a_mulpd
	2,9,0,0,	//a_mulsd
	2,9,0,0,	//a_orpd
	3,9,0,0,	//a_pshufd,
	3,9,0,0,	//a_pshufhw
	3,9,0,0,	//a_pshuflw
	2,9,0,0,	//a_pslldq
	2,9,0,0,	//a_psrldq
	3,9,0,0,	//a_shufpd
	2,9,0,0,	//a_sqrtpd
	2,9,0,0,	//a_sqrtsd
	2,9,0,0,	//a_subpd
	2,9,0,0,	//a_subsd
	2,9,0,0,	//a_ucomisd
	2,9,0,0,	//a_unpckhpd
	2,9,0,0,	//a_unpcklpd
	2,9,0,0,	//a_xorpd
	2,9,0,0,	//a_paddq
	2,9,0,0,	//a_pmuludq
	2,9,0,0,	//a_psubq
	2,9,0,0,	//a_punpckhqdq
	2,9,0,0,	//a_punpcklqdq
	1,9,0,0,	//a_clflush
	0,9,0,0,	//a_monitor
	0,9,0,0,	//a_mwait
	2,9,0,0,	//a_addsubpd
	2,9,0,0,	//a_addsubps
	2,9,0,0,	//a_cmpeqsd
	2,9,0,0,	//a_cmpltsd
	2,9,0,0,	//a_cmplesd
	2,9,0,0,	//a_cmpunordsd
	2,9,0,0,	//a_cmpneqsd
	2,9,0,0,	//a_cmpnltsd
	2,9,0,0,	//a_cmpnlesd
	2,9,0,0,	//a_cmpordsd
	2,9,0,0,	//a_cmpeqpd
	2,9,0,0,	//a_cmpltpd
	2,9,0,0,	//a_cmplepd
	2,9,0,0,	//a_cmpunordpd
	2,9,0,0,	//a_cmpneqpd
	2,9,0,0,	//a_cmpnltpd
	2,9,0,0,	//a_cmpnlepd
	2,9,0,0,	//a_cmpordpd
	2,9,0,0,	//a_cmpeqps
	2,9,0,0,	//a_cmpltps
	2,9,0,0,	//a_cmpleps
	2,9,0,0,	//a_cmpunordps
	2,9,0,0,	//a_cmpneqps
	2,9,0,0,	//a_cmpnltps
	2,9,0,0,	//a_cmpnleps
	2,9,0,0,	//a_cmpordps
	2,9,0,0,	//a_cmpeqss
	2,9,0,0,	//a_cmpltss
	2,9,0,0,	//a_cmpless
	2,9,0,0,	//a_cmpunordss
	2,9,0,0,	//a_cmpneqss
	2,9,0,0,	//a_cmpnltss
	2,9,0,0,	//a_cmpnless
	2,9,0,0,	//a_cmpordss
	2,9,0,0,	//a_haddpd
	2,9,0,0,	//a_haddps
	2,9,0,0,	//a_hsubpd
	2,9,0,0,	//a_hsubps
	2,9,0,0,	//a_lddqu
	2,9,0,0,	//a_movddup
	2,9,0,0,	//a_movshdup
	2,9,0,0,	//a_movsldup
	0,9,0,0	//a_pause
};

