import re
import os
import argparse
import sys
import pickle

# Parameters
# Path to doxygen folder to make doxygen files in: -o <path>
doxygen_src_path = 'docs/doxygen'
# Remove generated doxygen files: --clean
clean_generated_stuff = False
# Dump all defined symbols: --dump
dump_symbols = False
# Print symbol stats: --stats
print_stats = False
# Do not write warnings file: --nowarn
enable_warnings = True

# Constants
link_root = "http://websvn.kolibrios.org/filedetails.php?repname=Kolibri+OS&path=/kernel/trunk"

# fasm keywords
keywords = [
	# Generic keywords
	"align",
	"equ",
	"org",
	"while",
	"load",
	"store",
	"times",
	"repeat",
	"virtual",
	"display",
	"err",
	"assert",
	"if",
	# Instructions
	"aaa",
	"aad",
	"aam",
	"aas",
	"adc",
	"adcx",
	"add",
	"addpd",
	"addps",
	"addsd",
	"addss",
	"addsubpd",
	"addsubps",
	"adox",
	"aesdec",
	"aesdeclast",
	"aesenc",
	"aesenclast",
	"aesimc",
	"aeskeygenassist",
	"and",
	"andn",
	"andnpd",
	"andnps",
	"andpd",
	"andps",
	"arpl",
	"bextr",
	"blendpd",
	"blendps",
	"blendvpd",
	"blendvps",
	"blsi",
	"blsmsk",
	"blsr",
	"bndcl",
	"bndcn",
	"bndcu",
	"bndldx",
	"bndmk",
	"bndmov",
	"bndstx",
	"bound",
	"bsf",
	"bsr",
	"bswap",
	"bt",
	"btc",
	"btr",
	"bts",
	"bzhi",
	"call",
	"cbw",
	"cdq",
	"cdqe",
	"clac",
	"clc",
	"cld",
	"cldemote",
	"clflush",
	"clflushopt",
	"cli",
	"clts",
	"clwb",
	"cmc",
	"cmova",
	"cmovae",
	"cmovb",
	"cmovbe",
	"cmovc",
	"cmove",
	"cmovg",
	"cmovge",
	"cmovl",
	"cmovle",
	"cmovna",
	"cmovnae",
	"cmovnb",
	"cmovnbe",
	"cmovnc",
	"cmovne",
	"cmovng",
	"cmovnge",
	"cmovnl",
	"cmovnle",
	"cmovno",
	"cmovnp",
	"cmovns",
	"cmovnz",
	"cmovo",
	"cmovp",
	"cmovpe",
	"cmovpo",
	"cmovs",
	"cmovz",
	"cmp",
	"cmppd",
	"cmpps",
	"cmps",
	"cmpsb",
	"cmpsd",
	"cmpsd",
	"cmpsq",
	"cmpss",
	"cmpsw",
	"cmpxchg",
	"cmpxchg16b",
	"cmpxchg8b",
	"comisd",
	"comiss",
	"cpuid",
	"cqo",
	"crc32",
	"cvtdq2pd",
	"cvtdq2ps",
	"cvtpd2dq",
	"cvtpd2pi",
	"cvtpd2ps",
	"cvtpi2pd",
	"cvtpi2ps",
	"cvtps2dq",
	"cvtps2pd",
	"cvtps2pi",
	"cvtsd2si",
	"cvtsd2ss",
	"cvtsi2sd",
	"cvtsi2ss",
	"cvtss2sd",
	"cvtss2si",
	"cvttpd2dq",
	"cvttpd2pi",
	"cvttps2dq",
	"cvttps2pi",
	"cvttsd2si",
	"cvttss2si",
	"cwd",
	"cwde",
	"daa",
	"das",
	"dec",
	"div",
	"divpd",
	"divps",
	"divsd",
	"divss",
	"dppd",
	"dpps",
	"emms",
	"enter",
	"extractps",
	"f2xm1",
	"fabs",
	"fadd",
	"faddp",
	"fbld",
	"fbstp",
	"fchs",
	"fclex",
	"fcmova",
	"fcmovae",
	"fcmovb",
	"fcmovbe",
	"fcmovc",
	"fcmove",
	"fcmovg",
	"fcmovge",
	"fcmovl",
	"fcmovle",
	"fcmovna",
	"fcmovnae",
	"fcmovnb",
	"fcmovnbe",
	"fcmovnc",
	"fcmovne",
	"fcmovng",
	"fcmovnge",
	"fcmovnl",
	"fcmovnle",
	"fcmovno",
	"fcmovnp",
	"fcmovns",
	"fcmovnz",
	"fcmovo",
	"fcmovp",
	"fcmovpe",
	"fcmovpo",
	"fcmovs",
	"fcmovz",
	"fcom",
	"fcomi",
	"fcomip",
	"fcomp",
	"fcompp",
	"fcos",
	"fdecstp",
	"fdiv",
	"fdivp",
	"fdivr",
	"fdivrp",
	"ffree",
	"fiadd",
	"ficom",
	"ficomp",
	"fidiv",
	"fidivr",
	"fild",
	"fimul",
	"fincstp",
	"finit",
	"fist",
	"fistp",
	"fisttp",
	"fisub",
	"fisubr",
	"fld",
	"fld1",
	"fldcw",
	"fldenv",
	"fldl2e",
	"fldl2t",
	"fldlg2",
	"fldln2",
	"fldpi",
	"fldz",
	"fmul",
	"fmulp",
	"fnclex",
	"fninit",
	"fnop",
	"fnsave",
	"fnstcw",
	"fnstenv",
	"fnstsw",
	"fpatan",
	"fprem",
	"fprem1",
	"fptan",
	"frndint",
	"frstor",
	"fsave",
	"fscale",
	"fsin",
	"fsincos",
	"fsqrt",
	"fst",
	"fstcw",
	"fstenv",
	"fstp",
	"fstsw",
	"fsub",
	"fsubp",
	"fsubr",
	"fsubrp",
	"ftst",
	"fucom",
	"fucomi",
	"fucomip",
	"fucomp",
	"fucompp",
	"fwait",
	"fxam",
	"fxch",
	"fxrstor",
	"fxsave",
	"fxtract",
	"fyl2x",
	"fyl2xp1",
	"gf2p8affineinvqb",
	"gf2p8affineqb",
	"gf2p8mulb",
	"haddpd",
	"haddps",
	"hlt",
	"hsubpd",
	"hsubps",
	"idiv",
	"imul",
	"in",
	"inc",
	"ins",
	"insb",
	"insd",
	"insertps",
	"insw",
	"int",
	"int1",
	"int3",
	"into",
	"invd",
	"invlpg",
	"invpcid",
	"iret",
	"iretd",
	"jmp",
	"ja",
	"jae",
	"jb",
	"jbe",
	"jc",
	"jcxz",
	"jecxz",
	"je",
	"jg",
	"jge",
	"jl",
	"jle",
	"jna",
	"jnae",
	"jnb",
	"jnbe",
	"jnc",
	"jne",
	"jng",
	"jnge",
	"jnl",
	"jnle",
	"jno",
	"jnp",
	"jns",
	"jnz",
	"jo",
	"jp",
	"jpe",
	"jpo",
	"js",
	"jz",
	"kaddb",
	"kaddd",
	"kaddq",
	"kaddw",
	"kandb",
	"kandd",
	"kandnb",
	"kandnd",
	"kandnq",
	"kandnw",
	"kandq",
	"kandw",
	"kmovb",
	"kmovd",
	"kmovq",
	"kmovw",
	"knotb",
	"knotd",
	"knotq",
	"knotw",
	"korb",
	"kord",
	"korq",
	"kortestb",
	"kortestd",
	"kortestq",
	"kortestw",
	"korw",
	"kshiftlb",
	"kshiftld",
	"kshiftlq",
	"kshiftlw",
	"kshiftrb",
	"kshiftrd",
	"kshiftrq",
	"kshiftrw",
	"ktestb",
	"ktestd",
	"ktestq",
	"ktestw",
	"kunpckbw",
	"kunpckdq",
	"kunpckwd",
	"kxnorb",
	"kxnord",
	"kxnorq",
	"kxnorw",
	"kxorb",
	"kxord",
	"kxorq",
	"kxorw",
	"lahf",
	"lar",
	"lddqu",
	"ldmxcsr",
	"lds",
	"lea",
	"leave",
	"les",
	"lfence",
	"lfs",
	"lgdt",
	"lgs",
	"lidt",
	"lldt",
	"lmsw",
	"lock",
	"lods",
	"lodsb",
	"lodsd",
	"lodsq",
	"lodsw",
	"loop",
	"loopa",
	"loopae",
	"loopb",
	"loopbe",
	"loopc",
	"loope",
	"loopg",
	"loopge",
	"loopl",
	"loople",
	"loopna",
	"loopnae",
	"loopnb",
	"loopnbe",
	"loopnc",
	"loopne",
	"loopng",
	"loopnge",
	"loopnl",
	"loopnle",
	"loopno",
	"loopnp",
	"loopns",
	"loopnz",
	"loopo",
	"loopp",
	"looppe",
	"looppo",
	"loops",
	"loopz",
	"lsl",
	"lss",
	"ltr",
	"lzcnt",
	"maskmovdqu",
	"maskmovq",
	"maxpd",
	"maxps",
	"maxsd",
	"maxss",
	"mfence",
	"minpd",
	"minps",
	"minsd",
	"minss",
	"monitor",
	"mov",
	"movapd",
	"movaps",
	"movbe",
	"movd",
	"movddup",
	"movdir64b",
	"movdiri",
	"movdq2q",
	"movdqa",
	"movdqu",
	"movhlps",
	"movhpd",
	"movhps",
	"movlhps",
	"movlpd",
	"movlps",
	"movmskpd",
	"movmskps",
	"movntdq",
	"movntdqa",
	"movnti",
	"movntpd",
	"movntps",
	"movntq",
	"movq",
	"movq",
	"movq2dq",
	"movs",
	"movsb",
	"movsd",
	"movsd",
	"movshdup",
	"movsldup",
	"movsq",
	"movss",
	"movsw",
	"movsx",
	"movsxd",
	"movupd",
	"movups",
	"movzx",
	"mpsadbw",
	"mul",
	"mulpd",
	"mulps",
	"mulsd",
	"mulss",
	"mulx",
	"mwait",
	"neg",
	"nop",
	"not",
	"or",
	"orpd",
	"orps",
	"out",
	"outs",
	"outsb",
	"outsd",
	"outsw",
	"pabsb",
	"pabsd",
	"pabsq",
	"pabsw",
	"packssdw",
	"packsswb",
	"packusdw",
	"packuswb",
	"paddb",
	"paddd",
	"paddq",
	"paddsb",
	"paddsw",
	"paddusb",
	"paddusw",
	"paddw",
	"palignr",
	"pand",
	"pandn",
	"pause",
	"pavgb",
	"pavgw",
	"pblendvb",
	"pblendw",
	"pclmulqdq",
	"pcmpeqb",
	"pcmpeqd",
	"pcmpeqq",
	"pcmpeqw",
	"pcmpestri",
	"pcmpestrm",
	"pcmpgtb",
	"pcmpgtd",
	"pcmpgtq",
	"pcmpgtw",
	"pcmpistri",
	"pcmpistrm",
	"pdep",
	"pext",
	"pextrb",
	"pextrd",
	"pextrq",
	"pextrw",
	"phaddd",
	"phaddsw",
	"phaddw",
	"phminposuw",
	"phsubd",
	"phsubsw",
	"phsubw",
	"pinsrb",
	"pinsrd",
	"pinsrq",
	"pinsrw",
	"pmaddubsw",
	"pmaddwd",
	"pmaxsb",
	"pmaxsd",
	"pmaxsq",
	"pmaxsw",
	"pmaxub",
	"pmaxud",
	"pmaxuq",
	"pmaxuw",
	"pminsb",
	"pminsd",
	"pminsq",
	"pminsw",
	"pminub",
	"pminud",
	"pminuq",
	"pminuw",
	"pmovmskb",
	"pmovsx",
	"pmovzx",
	"pmuldq",
	"pmulhrsw",
	"pmulhuw",
	"pmulhw",
	"pmulld",
	"pmullq",
	"pmullw",
	"pmuludq",
	"pop",
	"popa",
	"popad",
	"popcnt",
	"popf",
	"popfd",
	"popfq",
	"por",
	"prefetchw",
	"prefetchh",
	"psadbw",
	"pshufb",
	"pshufd",
	"pshufhw",
	"pshuflw",
	"pshufw",
	"psignb",
	"psignd",
	"psignw",
	"pslld",
	"pslldq",
	"psllq",
	"psllw",
	"psrad",
	"psraq",
	"psraw",
	"psrld",
	"psrldq",
	"psrlq",
	"psrlw",
	"psubb",
	"psubd",
	"psubq",
	"psubsb",
	"psubsw",
	"psubusb",
	"psubusw",
	"psubw",
	"ptest",
	"ptwrite",
	"punpckhbw",
	"punpckhdq",
	"punpckhqdq",
	"punpckhwd",
	"punpcklbw",
	"punpckldq",
	"punpcklqdq",
	"punpcklwd",
	"push",
	"pushw",
	"pushd",
	"pusha",
	"pushad",
	"pushf",
	"pushfd",
	"pushfq",
	"pxor",
	"rcl",
	"rcpps",
	"rcpss",
	"rcr",
	"rdfsbase",
	"rdgsbase",
	"rdmsr",
	"rdpid",
	"rdpkru",
	"rdpmc",
	"rdrand",
	"rdseed",
	"rdtsc",
	"rdtscp",
	"rep",
	"repe",
	"repne",
	"repnz",
	"repz",
	"ret",
	"rol",
	"ror",
	"rorx",
	"roundpd",
	"roundps",
	"roundsd",
	"roundss",
	"rsm",
	"rsqrtps",
	"rsqrtss",
	"sahf",
	"sal",
	"sar",
	"sarx",
	"sbb",
	"scas",
	"scasb",
	"scasd",
	"scasw",
	"seta",
	"setae",
	"setb",
	"setbe",
	"setc",
	"sete",
	"setg",
	"setge",
	"setl",
	"setle",
	"setna",
	"setnae",
	"setnb",
	"setnbe",
	"setnc",
	"setne",
	"setng",
	"setnge",
	"setnl",
	"setnle",
	"setno",
	"setnp",
	"setns",
	"setnz",
	"seto",
	"setp",
	"setpe",
	"setpo",
	"sets",
	"setz",
	"sfence",
	"sgdt",
	"sha1msg1",
	"sha1msg2",
	"sha1nexte",
	"sha1rnds4",
	"sha256msg1",
	"sha256msg2",
	"sha256rnds2",
	"shl",
	"shld",
	"shlx",
	"shr",
	"shrd",
	"shrx",
	"shufpd",
	"shufps",
	"sidt",
	"sldt",
	"smsw",
	"sqrtpd",
	"sqrtps",
	"sqrtsd",
	"sqrtss",
	"stac",
	"stc",
	"std",
	"sti",
	"stmxcsr",
	"stos",
	"stosb",
	"stosd",
	"stosq",
	"stosw",
	"str",
	"sub",
	"subpd",
	"subps",
	"subsd",
	"subss",
	"swapgs",
	"syscall",
	"sysenter",
	"sysexit",
	"sysret",
	"test",
	"tpause",
	"tzcnt",
	"ucomisd",
	"ucomiss",
	"ud",
	"umonitor",
	"umwait",
	"unpckhpd",
	"unpckhps",
	"unpcklpd",
	"unpcklps",
	"valignd",
	"valignq",
	"vblendmpd",
	"vblendmps",
	"vbroadcast",
	"vcompresspd",
	"vcompressps",
	"vcvtpd2qq",
	"vcvtpd2udq",
	"vcvtpd2uqq",
	"vcvtph2ps",
	"vcvtps2ph",
	"vcvtps2qq",
	"vcvtps2udq",
	"vcvtps2uqq",
	"vcvtqq2pd",
	"vcvtqq2ps",
	"vcvtsd2usi",
	"vcvtss2usi",
	"vcvttpd2qq",
	"vcvttpd2udq",
	"vcvttpd2uqq",
	"vcvttps2qq",
	"vcvttps2udq",
	"vcvttps2uqq",
	"vcvttsd2usi",
	"vcvttss2usi",
	"vcvtudq2pd",
	"vcvtudq2ps",
	"vcvtuqq2pd",
	"vcvtuqq2ps",
	"vcvtusi2sd",
	"vcvtusi2ss",
	"vdbpsadbw",
	"verr",
	"verw",
	"vexpandpd",
	"vexpandps",
	"vextractf128",
	"vextractf32x4",
	"vextractf32x8",
	"vextractf64x2",
	"vextractf64x4",
	"vextracti128",
	"vextracti32x4",
	"vextracti32x8",
	"vextracti64x2",
	"vextracti64x4",
	"vfixupimmpd",
	"vfixupimmps",
	"vfixupimmsd",
	"vfixupimmss",
	"vfmadd132pd",
	"vfmadd132ps",
	"vfmadd132sd",
	"vfmadd132ss",
	"vfmadd213pd",
	"vfmadd213ps",
	"vfmadd213sd",
	"vfmadd213ss",
	"vfmadd231pd",
	"vfmadd231ps",
	"vfmadd231sd",
	"vfmadd231ss",
	"vfmaddsub132pd",
	"vfmaddsub132ps",
	"vfmaddsub213pd",
	"vfmaddsub213ps",
	"vfmaddsub231pd",
	"vfmaddsub231ps",
	"vfmsub132pd",
	"vfmsub132ps",
	"vfmsub132sd",
	"vfmsub132ss",
	"vfmsub213pd",
	"vfmsub213ps",
	"vfmsub213sd",
	"vfmsub213ss",
	"vfmsub231pd",
	"vfmsub231ps",
	"vfmsub231sd",
	"vfmsub231ss",
	"vfmsubadd132pd",
	"vfmsubadd132ps",
	"vfmsubadd213pd",
	"vfmsubadd213ps",
	"vfmsubadd231pd",
	"vfmsubadd231ps",
	"vfnmadd132pd",
	"vfnmadd132ps",
	"vfnmadd132sd",
	"vfnmadd132ss",
	"vfnmadd213pd",
	"vfnmadd213ps",
	"vfnmadd213sd",
	"vfnmadd213ss",
	"vfnmadd231pd",
	"vfnmadd231ps",
	"vfnmadd231sd",
	"vfnmadd231ss",
	"vfnmsub132pd",
	"vfnmsub132ps",
	"vfnmsub132sd",
	"vfnmsub132ss",
	"vfnmsub213pd",
	"vfnmsub213ps",
	"vfnmsub213sd",
	"vfnmsub213ss",
	"vfnmsub231pd",
	"vfnmsub231ps",
	"vfnmsub231sd",
	"vfnmsub231ss",
	"vfpclasspd",
	"vfpclassps",
	"vfpclasssd",
	"vfpclassss",
	"vgatherdpd",
	"vgatherdpd",
	"vgatherdps",
	"vgatherdps",
	"vgatherqpd",
	"vgatherqpd",
	"vgatherqps",
	"vgatherqps",
	"vgetexppd",
	"vgetexpps",
	"vgetexpsd",
	"vgetexpss",
	"vgetmantpd",
	"vgetmantps",
	"vgetmantsd",
	"vgetmantss",
	"vinsertf128",
	"vinsertf32x4",
	"vinsertf32x8",
	"vinsertf64x2",
	"vinsertf64x4",
	"vinserti128",
	"vinserti32x4",
	"vinserti32x8",
	"vinserti64x2",
	"vinserti64x4",
	"vmaskmov",
	"vmovdqa32",
	"vmovdqa64",
	"vmovdqu16",
	"vmovdqu32",
	"vmovdqu64",
	"vmovdqu8",
	"vpblendd",
	"vpblendmb",
	"vpblendmd",
	"vpblendmq",
	"vpblendmw",
	"vpbroadcast",
	"vpbroadcastb",
	"vpbroadcastd",
	"vpbroadcastm",
	"vpbroadcastq",
	"vpbroadcastw",
	"vpcmpb",
	"vpcmpd",
	"vpcmpq",
	"vpcmpub",
	"vpcmpud",
	"vpcmpuq",
	"vpcmpuw",
	"vpcmpw",
	"vpcompressd",
	"vpcompressq",
	"vpconflictd",
	"vpconflictq",
	"vperm2f128",
	"vperm2i128",
	"vpermb",
	"vpermd",
	"vpermi2b",
	"vpermi2d",
	"vpermi2pd",
	"vpermi2ps",
	"vpermi2q",
	"vpermi2w",
	"vpermilpd",
	"vpermilps",
	"vpermpd",
	"vpermps",
	"vpermq",
	"vpermt2b",
	"vpermt2d",
	"vpermt2pd",
	"vpermt2ps",
	"vpermt2q",
	"vpermt2w",
	"vpermw",
	"vpexpandd",
	"vpexpandq",
	"vpgatherdd",
	"vpgatherdd",
	"vpgatherdq",
	"vpgatherdq",
	"vpgatherqd",
	"vpgatherqd",
	"vpgatherqq",
	"vpgatherqq",
	"vplzcntd",
	"vplzcntq",
	"vpmadd52huq",
	"vpmadd52luq",
	"vpmaskmov",
	"vpmovb2m",
	"vpmovd2m",
	"vpmovdb",
	"vpmovdw",
	"vpmovm2b",
	"vpmovm2d",
	"vpmovm2q",
	"vpmovm2w",
	"vpmovq2m",
	"vpmovqb",
	"vpmovqd",
	"vpmovqw",
	"vpmovsdb",
	"vpmovsdw",
	"vpmovsqb",
	"vpmovsqd",
	"vpmovsqw",
	"vpmovswb",
	"vpmovusdb",
	"vpmovusdw",
	"vpmovusqb",
	"vpmovusqd",
	"vpmovusqw",
	"vpmovuswb",
	"vpmovw2m",
	"vpmovwb",
	"vpmultishiftqb",
	"vprold",
	"vprolq",
	"vprolvd",
	"vprolvq",
	"vprord",
	"vprorq",
	"vprorvd",
	"vprorvq",
	"vpscatterdd",
	"vpscatterdq",
	"vpscatterqd",
	"vpscatterqq",
	"vpsllvd",
	"vpsllvq",
	"vpsllvw",
	"vpsravd",
	"vpsravq",
	"vpsravw",
	"vpsrlvd",
	"vpsrlvq",
	"vpsrlvw",
	"vpternlogd",
	"vpternlogq",
	"vptestmb",
	"vptestmd",
	"vptestmq",
	"vptestmw",
	"vptestnmb",
	"vptestnmd",
	"vptestnmq",
	"vptestnmw",
	"vrangepd",
	"vrangeps",
	"vrangesd",
	"vrangess",
	"vrcp14pd",
	"vrcp14ps",
	"vrcp14sd",
	"vrcp14ss",
	"vreducepd",
	"vreduceps",
	"vreducesd",
	"vreducess",
	"vrndscalepd",
	"vrndscaleps",
	"vrndscalesd",
	"vrndscaless",
	"vrsqrt14pd",
	"vrsqrt14ps",
	"vrsqrt14sd",
	"vrsqrt14ss",
	"vscalefpd",
	"vscalefps",
	"vscalefsd",
	"vscalefss",
	"vscatterdpd",
	"vscatterdps",
	"vscatterqpd",
	"vscatterqps",
	"vshuff32x4",
	"vshuff64x2",
	"vshufi32x4",
	"vshufi64x2",
	"vtestpd",
	"vtestps",
	"vzeroall",
	"vzeroupper",
	"wait",
	"wbinvd",
	"wrfsbase",
	"wrgsbase",
	"wrmsr",
	"wrpkru",
	"xabort",
	"xacquire",
	"xadd",
	"xbegin",
	"xchg",
	"xend",
	"xgetbv",
	"xlat",
	"xlatb",
	"xor",
	"xorpd",
	"xorps",
	"xrelease",
	"xrstor",
	"xrstors",
	"xsave",
	"xsavec",
	"xsaveopt",
	"xsaves",
	"xsetbv",
	"xtest"
]

fasm_types = [
	"db", "rb",
	"dw", "rw",
	"dd", "rd",
	"dp", "rp",
	"df", "rf",
	"dq", "rq",
	"dt", "rt",
	"du",
]

# Dict where an identifier is assicoated with a string
# The string contains characters specifying flags
# Available flags:
#  k - Keyword
#  m - Macro name
#  t - fasm data Type name (db, rq, etc.)
#  s - Struct type name
#  e - equated constant (name equ value)
#  = - set constants (name = value)
ID_KIND_KEYWORD = 'k'
ID_KIND_MACRO_NAME = 'm'
ID_KIND_FASM_TYPE = 't'
ID_KIND_STRUCT_NAME = 's'
ID_KIND_EQUATED_CONSTANT = 'e'
ID_KIND_SET_CONSTANT = '='
id2kind = {}

# Add kind flag to identifier in id2kind
def id_add_kind(identifier, kind):
	if identifier not in id2kind:
		id2kind[identifier] = ''
	id2kind[identifier] += kind

# Remove kind flag of identifier in id2kind
def id_remove_kind(identifier, kind):
	if identifier in id2kind:
		if kind in id2kind[identifier]:
			id2kind[identifier] = id2kind[identifier].replace(kind, '')

# Get kind of an identifier
def id_get_kind(identifier):
	if identifier in id2kind:
		return id2kind[identifier]
	else:
		return ''

for keyword in keywords:
	id_add_kind(keyword, ID_KIND_KEYWORD)

for fasm_type in fasm_types:
	id_add_kind(fasm_type, ID_KIND_FASM_TYPE)

# Warning list
warnings = ""

# Parse arguments
parser = argparse.ArgumentParser()
parser.add_argument("-o", help="Doxygen output folder")
parser.add_argument("--clean", help="Remove generated files", action="store_true")
parser.add_argument("--dump", help="Dump all defined symbols", action="store_true")
parser.add_argument("--stats", help="Print symbol stats", action="store_true")
parser.add_argument("--nowarn", help="Do not write warnings file", action="store_true")
parser.add_argument("--noemit", help="Do not emit doxygen files (for testing)", action="store_true")
args = parser.parse_args()
doxygen_src_path = args.o if args.o else 'docs/doxygen'
clean_generated_stuff = args.clean
dump_symbols = args.dump
print_stats = args.stats
enable_warnings = not args.nowarn
noemit = args.noemit

# Variables, functions, labels, macros, structure types
elements = []

class LegacyAsmReader:
	def __init__(self, file):
		self.file = file
		self.lines = open(file, "r", encoding="utf-8").readlines()
		self.line_idx = 0
		self.i = 0

	def curr(self):
		try: return self.lines[self.line_idx][self.i]
		except: return ''

	def step(self):
		c = self.curr()
		self.i += 1
		# Wrap the line if '\\' followed by whitespaces and/or comment
		while self.curr() == '\\':
			i_of_backslash = self.i
			self.i += 1
			while self.curr().isspace():
				self.i += 1
			if self.curr() == ';' or self.curr() == '':
				self.line_idx += 1
				self.i = 0
			else:
				# There's something other than a comment after the backslash
				# So don't interpret the backslash as a line wrap
				self.i = i_of_backslash
				break
		return c

	def nextline(self):
		c = self.curr()
		while c != '':
			c = self.step()
		self.line_idx += 1
		self.i = 0

	def no_lines(self):
		if self.line_idx >= len(self.lines):
			return True
		return False

	def location(self):	
		return f"{self.file}:{self.line_idx + 1}"

	def skip_spaces(self):
		while self.curr().isspace():
			self.step()

class AsmReaderRecognizingStrings(LegacyAsmReader):
	def __init__(self, file):
		super().__init__(file)
		self.in_string = None
		self.should_recognize_strings = True

	def step(self):
		c = super().step()
		if self.should_recognize_strings and (c == '"' or c == "'"):
			# If just now we was at the double or single quotation mark
			# and we aren't in a string yet
			# then say "we are in a string openned with this quotation mark now"
			if self.in_string == None:
				self.in_string = c
			# If just now we was at the double or single quotation mark
			# and we are in the string entered with the same quotation mark
			# then say "we aren't in a string anymore"
			elif self.in_string == c:
				self.in_string = None
		return c

class AsmReaderReadingComments(AsmReaderRecognizingStrings):
	def __init__(self, file):
		super().__init__(file)
		self.status = dict()
		self.status_reset()
		self.comment = ''

	def status_reset(self):
		# If the line has non-comment code
		self.status_has_code = False
		# If the line has a comment at the end
		self.status_has_comment = False
		# Let it recognize strings further, we are definitely out of a comment
		self.should_recognize_strings = True

	def status_set_has_comment(self):
		self.status_has_comment = True
		# Don't let it recognize strings cause we are in a comment now
		self.should_recognize_strings = False

	def status_set_has_code(self):
		self.status_has_code = True

	def update_status(self):
		# If we aren't in a comment and we aren't in a string - say we are now in a comment if ';' met
		if not self.status_has_comment and not self.in_string and self.curr() == ';':
			self.status_set_has_comment()
		# Else if we are in a comment - collect the comment
		elif self.status_has_comment:
			self.comment += self.curr()
		# Else if there's some non-whitespace character out of a comment
		# then the line has code
		elif not self.status_has_comment and not self.curr().isspace():
			self.status_set_has_code()

	def step(self):
		# Get to the next character
		c = super().step()
		# Update status of the line according to the next character
		self.update_status()
		return c

	def nextline(self):
		super().nextline()
		# If the line we leave was not a comment-only line
		# then forget the collected comment
		# Otherwise the collected comment should be complemented by comment from next line in step()
		if self.status_has_code:
			self.comment = ''
		# Reset the line status (now it's the status of the new line)
		self.status_reset()
		# Set new status for this line according to the first character in the line
		self.update_status()

class AsmReaderFetchingIdentifiers(AsmReaderReadingComments):
	def __init__(self, file):
		super().__init__(file)

	def fetch_identifier(self):
		self.skip_spaces()
		result = ''
		while is_id(self.curr()):
			result += self.step()
		return result

class AsmReader(AsmReaderFetchingIdentifiers):
	def __init__(self, file):
		super().__init__(file)

created_files = []

class AsmElement:
	def __init__(self, location, name, comment):
		global warnings

		# If the element was constructed during this execution then the element is new
		self.new = True
		self.location = location
		self.file = self.location.split(':')[0].replace('\\', '/')
		self.line = self.location.split(':')[1]
		self.name = name
		self.comment = comment

		if self.comment == '':
			warnings += f'{self.location}: Undocumented element\n'

	def dump(self):
		print(f"\n{self.location}: {self.name}")
		print(f"{self.comment}")

	def emit(self, dest, doxycomment = '', declaration = ''):
		# Do not emit anything if the symbol is marked as hidden in its comment
		if '@dont_give_a_doxygen' in self.comment:
			return

		global warnings
		# Redefine default declaration
		if declaration == '':
			declaration = f'#define {self.name}'
		# Check doxycomment
		if not doxycomment.endswith('\n'):
			doxycomment += '\n'
		if doxycomment.split('@brief ')[1][0].islower():
			warnings += f"{self.location}: Brief comment starting from lowercase\n"
		# Build contents to emit
		contents = ''
		contents += '/**\n'
		contents += doxycomment
		contents += (f"@par Source\n" +
		             f"<a href='{link_root}/{self.file}#line-{self.line}'>{self.file}:{self.line}</a>\n")
		contents += '*/\n'
		contents += declaration
		contents += '\n\n'
		# Get path to file to emit this
		full_path = dest + '/' + self.file
		# Remove the file on first access if it was created by previous generation
		if full_path not in created_files:
			if os.path.isfile(full_path):
				os.remove(full_path)
			created_files.append(full_path)
		# Create directories need for the file
		os.makedirs(os.path.dirname(full_path), exist_ok=True)
		f = open(full_path, "a")
		contents = ''.join([i if ord(i) < 128 else '?' for i in contents])
		f.write(contents)
		f.close()

class AsmVariable(AsmElement):
	def __init__(self, location, name, comment, type, init):
		super().__init__(location, name, comment)
		self.type = type
		self.init = init

	def dump(self):
		super().dump()
		print(f"(Variable)\n---")

	def emit(self, dest):
		# Build doxycomment specific for the variable
		doxycomment = ''
		doxycomment += self.comment
		if '@brief' not in doxycomment:
			doxycomment = '@brief ' + doxycomment
		doxycomment += (f"@par Initial value\n" +
		                f"{self.init}\n")
		# Build the declaration
		name = self.name.replace(".", "_")
		var_type = self.type.replace(".", "_")
		declaration = f"{var_type} {name};"
		# Emit this
		super().emit(dest, doxycomment, declaration)

class AsmFunction(AsmElement):
	def __init__(self, location, name, comment, calling_convention, args, used_regs):
		super().__init__(location, name, comment)
		self.calling_convention = calling_convention
		self.args = args
		self.used_regs = used_regs

	def dump(self):
		super().dump()
		print(f"(Function)\n---")

	def emit(self, dest):
		# Build doxycomment specific for the variable
		doxycomment = ''
		doxycomment += self.comment
		if '@brief' not in doxycomment:
			doxycomment = '@brief ' + doxycomment
		# If there was no arguments, maybe that's just a label
		# then parse parameters from its comment
		if len(self.args) == 0 and '@param' in self.comment:
			i = 0
			while '@param' in self.comment[i:]:
				i = self.comment.index('@param', i)
				# Skip '@param'
				i += len('@param')
				# Skip spaces after '@param'
				while self.comment[i].isspace():
					i += 1
				# Get the parameter name
				name = ''
				while is_id(self.comment[i]):
					name += self.comment[i]
					i += 1
				# Save the parameter
				self.args.append((name, 'arg_t'))
		# Build the arg list for declaration
		arg_list = '('
		if len(self.args) > 0:
			argc = 0
			for arg in self.args:
				if argc != 0:
					arg_list += ", "
				arg_list += f"{arg[1]} {arg[0]}"
				argc += 1
		arg_list += ')'
		# Build the declaration
		name = self.name.replace(".", "_")
		declaration = f"void {name}{arg_list};"
		# Emit this
		super().emit(dest, doxycomment, declaration)

class AsmLabel(AsmElement):
	def __init__(self, location, name, comment):
		super().__init__(location, name, comment)

	def dump(self):
		super().dump()
		print(f"(Label)\n---")

	def emit(self, dest):
		# Build doxycomment specific for the variable
		doxycomment = ''
		doxycomment += self.comment
		if '@brief' not in doxycomment:
			doxycomment = '@brief ' + doxycomment
		# Build the declaration
		name = self.name.replace(".", "_")
		declaration = f"label {name};"
		# Emit this
		super().emit(dest, doxycomment, declaration)

class AsmMacro(AsmElement):
	def __init__(self, location, name, comment, args):
		super().__init__(location, name, comment)
		self.args = args

	def dump(self):
		super().dump()
		print(f"(Macro)\n---")

	def emit(self, dest):
		# Construct arg list without '['s, ']'s and '*'s
		args = [arg for arg in self.args if arg not in "[]*"]
		# Construct C-like arg list
		arg_list = ""
		if len(args) > 0:
			arg_list += '('
			argc = 0
			for arg in args:
				if argc != 0:
					arg_list += ", "
				arg_list += arg
				argc += 1
			arg_list += ')'
		# Build doxycomment
		doxycomment = ''
		doxycomment += self.comment
		if '@brief' not in doxycomment:
			doxycomment = '@brief ' + doxycomment
		# Build declaration
		declaration = f"#define {self.name}{arg_list}"
		# Emit this
		super().emit(dest, doxycomment, declaration)

class AsmStruct(AsmElement):
	def __init__(self, location, name, comment, members):
		super().__init__(location, name, comment)
		self.members = members

	def dump(self):
		super().dump()
		print(f"(Struct)\n---")

	def emit(self, dest):
		# Build doxycomment
		doxycomment = ''
		doxycomment += self.comment
		if '@brief' not in doxycomment:
			doxycomment = '@brief ' + doxycomment
		doxycomment += '\n'
		# Build declaration
		declaration = f"struct {self.name}" + " {\n"
		for member in self.members:
			if type(member) == AsmVariable:
				declaration += f'\t{member.type} {member.name}; /**< {member.comment} */\n'
		declaration += '};'
		# Emit this
		super().emit(dest, doxycomment, declaration)

class AsmUnion(AsmElement):
	def __init__(self, location, name, comment, members):
		super().__init__(location, name, comment)
		self.members = members

	def dump(self):
		super().dump()
		print(f"(Union)\n---")

	def emit(self, dest):
		# Build doxycomment
		doxycomment = ''
		doxycomment += self.comment
		if '@brief' not in doxycomment:
			doxycomment = '@brief ' + doxycomment
		# Build declaration
		declaration = f"union {self.name}" + " {};"
		# Emit this
		super().emit(dest, doxycomment, declaration)

class VariableNameIsMacroName:
	def __init__(self, name):
		self.name = name

def is_id(c):
	return c.isprintable() and c not in "+-/*=<>()[]{};:,|&~#`'\" \n\r\t\v"

def is_starts_as_id(s):
	return not s[0].isdigit()

def parse_after_macro(r):
	location = r.location()

	# Skip spaces after the "macro" keyword
	r.skip_spaces()
	# Read macro name
	name = ""
	while is_id(r.curr()) or r.curr() == '#':
		name += r.step()
	# Skip spaces after macro name
	r.skip_spaces()
	# Find all arguments
	args = []
	arg = ''
	while r.curr() and r.curr() != ';' and r.curr() != '{':
		# Collect identifier
		if is_id(r.curr()):
			arg += r.step()
		# Save the collected identifier
		elif r.curr() == ',':
			args.append(arg)
			arg = ''
			r.step()
		# Just push the '['
		elif r.curr() == '[':
			args.append(r.step())
		# Just push the identifier and get ']' ready to be pushed on next comma
		elif r.curr() == ']':
			args.append(arg)
			arg = r.step()
		# Just push the identifier and get '*' ready to be pushed on next comma
		elif r.curr() == '*':
			args.append(arg)
			arg = r.step()
		# Just skip whitespaces
		elif r.curr().isspace():
			r.step()
		# Something unexpected
		else:
			raise Exception(f"Unexpected symbol '{r.curr()}' at index #{r.i} " + 
			                f"in the macro declaration at {location} " +
			                f"(line: {r.lines[r.line_idx]})\n''")
	# Append the last argument
	if arg != '':
		args.append(arg)
	# Skip t spaces after the argument list
	r.skip_spaces()
	# Get a comment if it is: read till the end of the line and get the comment from the reader
	while r.curr() != '':
		r.step()
	comment = r.comment
	# Find end of the macro
	prev = ''
	while True:
		if r.curr() == '}' and prev != '\\':
			break
		elif r.curr() == '':
			prev = ''
			r.nextline()
			continue
		prev = r.step()
	# Build the output
	return AsmMacro(location, name, comment, args)

def parse_variable(r, first_word = None):
	global warnings
	location = r.location()

	# Skip spaces before variable name
	r.skip_spaces()
	# Get variable name
	name = ""
	# Read it if it was not supplied
	if first_word == None:
		while is_id(r.curr()):
			name += r.step()
	# Or use the supplied one instead
	else:
		name = first_word
	# Check the name
	# If it's 0 len, that means threr's something else than an identifier at the beginning
	if len(name) == 0:
		return None
	# If it starts from digit or othervice illegally it's illegal
	if not is_starts_as_id(name):
		return None
	# Get kind of the identifier from id2kind table
	kind = id_get_kind(name)
	# If it's a keyword, that's not a variable declaration
	if ID_KIND_KEYWORD in kind:
		return None
	# If it's a macro name, that's not a variable declaration
	if ID_KIND_MACRO_NAME in kind:
		return VariableNameIsMacroName(name)
	# If it's a datatype or a structure name that's not a variable declaration: that's just a data
	# don't document just a data for now
	if ID_KIND_STRUCT_NAME in kind or ID_KIND_FASM_TYPE in kind:
		return None
	# Skip spaces before type name
	r.skip_spaces()
	# Read type name
	var_type = ""
	while is_id(r.curr()):
		var_type += r.step()
	# Check the type name
	if len(var_type) == 0:
		# If there's no type identifier after the name
		# maybe the name is something meaningful for the next parser
		# return it
		return name
	# If it starts from digit or othervice illegally it's illegal
	if not is_starts_as_id(var_type):
		return None
	# Get kind of type identifier
	type_kind = id_get_kind(var_type)
	# If it's a keyword, that's not a variable declaration
	# return the two words of the lexical structure
	if ID_KIND_KEYWORD in type_kind:
		return (name, var_type)
	# Skip spaces before the value
	r.skip_spaces()
	# Read the value until the comment or end of the line
	value = ""
	while r.curr() != ';' and r.curr() != '' and r.curr() != '\n':
		value += r.step()
	# Skip spaces after the value
	r.skip_spaces()
	# Read till end of the line to get a comment from the reader
	while r.curr() != '':
		r.step()
	# Build the result
	return AsmVariable(location, name, r.comment, var_type, value)

def parse_after_struct(r, as_union = True):
	global warnings
	location = r.location()

	# Skip spaces after "struct" keyword
	r.skip_spaces()
	# Read struct name
	name = ""
	while is_id(r.curr()):
		name += r.step()
	# Read till end of the line and get the comment from the reader
	while r.curr() != '':
		r.step()
	comment = r.comment
	# Get to the next line to parse struct members
	r.nextline()
	# Parse struct members
	members = []
	while True:
		r.skip_spaces()
		var = parse_variable(r)
		if type(var) == AsmVariable:
			members.append(var)
		elif type(var) == str:
			if var == 'union':
				# Parse the union as a struct
				union = parse_after_struct(r, as_union = True)
				members.append(union)
				# Skip the ends of the union
				r.nextline()
			elif r.curr() == ':':
				warnings += f"{r.location()}: Skept the label in the struct\n"
			else:
				raise Exception(f"Garbage in struct member at {location} (got '{var}' identifier)")
		elif type(var) == VariableNameIsMacroName:
			if var.name == 'ends':
				break
		r.nextline()
	# Return the result
	if as_union:
		return AsmStruct(location, name, comment, members)
	else:
		return AsmUnion(location, name, comment, members)

def parse_after_proc(r):
	# Get proc name
	name = r.fetch_identifier()
	# Next identifier after the proc name
	identifier = r.fetch_identifier()
	# Check if the id is 'stdcall' or 'c' (calling convention specifier)
	# and if so - save the convention and lookup the next identifier
	calling_convention = ''
	if identifier == 'stdcall' or identifier == 'c':
		calling_convention = identifier
		# If next is a comma, just skip it
		if r.curr() == ',':
			r.step()
		# Read the next identifier
		identifier = r.fetch_identifier()
	# Check if the id is 'uses' (used register list specifier)
	# and if so save the used register list
	used_regs = []
	if identifier == 'uses':
		# Read the registers
		while True:
			reg_name = r.fetch_identifier()
			if reg_name != '':
				used_regs.append(reg_name)
			else:
				break
		# If next is a comma, just skip it
		if r.curr() == ',':
			r.step()
		# Read the next identifier
		identifier = r.fetch_identifier()
	# Check if there are argument identifiers
	args = []
	while identifier != '':
		arg_name = identifier
		arg_type = 'arg_t'
		# Skip spaces after argument name
		r.skip_spaces()
		# If there's a ':' after the name - the next identifier is type
		if r.curr() == ':':
			r.step()
			arg_type = r.fetch_identifier()
		# If there's a comma - there's one more argument
		# else no arguments anymore
		if r.curr() == ',':
			r.step()
			identifier = r.fetch_identifier()
		else:
			identifier = ''
		args.append((arg_name, arg_type))
	# Get to the end of the line and get a comment from the reader
	while r.curr() != '':
		r.step()
	comment = r.comment
	# Build the element
	return AsmFunction(r.location(), name, comment, calling_convention, args, used_regs)

def get_declarations(asm_file_contents, asm_file_name):
	r = AsmReader(asm_file_name)

	while not r.no_lines():
		# Skip leading spaces
		r.skip_spaces()
		# Skip the line if it's starting with a comment
		if r.curr() == ';':
			r.nextline()
			continue
		# Get first word
		first_word = ""
		while is_id(r.curr()):
			first_word += r.step()
		# Match macro declaration
		if first_word == "macro":
			macro = parse_after_macro(r)
			elements.append(macro)
			id_add_kind(macro.name, ID_KIND_MACRO_NAME)
		# Match structure declaration
		elif first_word == "struct":
			struct = parse_after_struct(r)
			elements.append(struct)
			id_add_kind(struct.name, ID_KIND_STRUCT_NAME)
		# Match function definition
		elif first_word == "proc":
			proc = parse_after_proc(r)
			elements.append(proc)
		elif first_word == 'format':
			# Skip the format directive
			pass
		elif first_word == 'include':
			# Skip the include directive
			pass
		elif first_word == 'if':
			# Skip the conditional directive
			pass
		elif first_word == 'repeat':
			# Skip the repeat directive
			pass
		elif first_word == 'purge':
			while True:
				# Skip spaces after the 'purge' keyword or after the comma what separated the previous macro name
				r.skip_spaces()
				# Get the purged macro name
				name = ''
				while is_id(r.curr()):
					name += r.step()
				# Remove the purged macro from the macro names list
				try:
					id_remove_kind(name, ID_KIND_MACRO_NAME)
				except:
					pass
				# Skip spaces after the name
				r.skip_spaces()
				# If it's comma (',') after then that's not the last purged macro, continue purging
				if r.curr() == ',':
					r.step()
					continue
				# Here we purged all the macros should be purged
				break
		# Match label or a variable
		elif len(first_word) != 0:
			# Skip spaces after the identifier
			r.skip_spaces()
			# Match a variable
			var = parse_variable(r, first_word)
			if type(var) == AsmVariable:
				elements.append(var)
			# If it wasn't a variable but there was an identifier
			# Maybe that's a label and the identifier is the label name
			# The parse_variable returns the first found or supplied identifier
			# In this case it returns the first_word which is supplied
			# If it didn't match a type identifier after the word
			elif type(var) == str:
				name = var
				# Match label beginning (':' after name)
				if r.curr() == ':':
					# Get to the end of the line and get the coment from the reader
					while r.curr() != '':
						r.step()
					comment = r.comment
					# Only handle non-local labels
					if name[0] != '.' and name != "@@" and name != "$Revision":
						if '@return' in comment or '@param' in comment:
							element = AsmFunction(r.location(), name, comment, '', [], [])
						else:
							element = AsmLabel(r.location(), name, comment)
						elements.append(element)
				elif r.curr() == '=':
					# Save the identifier as a set constant
					id_add_kind(first_word, ID_KIND_SET_CONSTANT)
			elif type(var) == tuple:
				(word_one, word_two) = var
				if word_two == 'equ':
					# Save the identifier as an equated constant
					id_add_kind(word_one, ID_KIND_EQUATED_CONSTANT)
		r.nextline()

def it_neds_to_be_parsed(source_file):
	# If there's no symbols file saved - parse it anyway
	# cause we need to create the symbols file and use it
	# if we gonna generate proper doxygen
	if not os.path.isfile('asmxygen.elements.pickle'):
		return True
	dest = doxygen_src_path + '/' + source_file
	# If there's no the doxygen file it should be compiled to
	# then yes, we should compile it to doxygen
	if not os.path.isfile(dest):
		return True
	source_change_time = os.path.getmtime(source_file)
	dest_change_file = os.path.getmtime(dest)
	# If the source is newer than the doxygen it was compiled to
	# then the source should be recompiled (existing doxygen is old)
	if source_change_time > dest_change_file:
		return True
	return False

def handle_file(handled_files, asm_file_name, subdir = "."):
	global elements
	# Canonicalize the file path and get it relative to cwd
	cwd = os.path.abspath(os.path.dirname(sys.argv[0]))
	asm_file_name = os.path.realpath(asm_file_name)
	asm_file_name = asm_file_name[len(cwd) + 1:]
	# If it's lang.inc - skip it
	if asm_file_name == 'lang.inc':
		return
	# If the file was handled in this execution before - skip it
	if asm_file_name in handled_files:
		return
	# Say that the file was handled in this execution
	handled_files.append(asm_file_name)
	# Check if the file should be parsed (if it was modified or wasn't parsed yet)
	should_get_declarations = True
	if not it_neds_to_be_parsed(asm_file_name):
		print(f"Skipping {asm_file_name} (already newest)")
		should_get_declarations = False
	else:
		print(f"Handling {asm_file_name}")
		# Remove elements parsed from this file before if any
		elements_to_remove = [x for x in elements if x.location.split(':')[0] == asm_file_name]
		elements = [x for x in elements if x.location.split(':')[0] != asm_file_name]
		# Forget types of identifiers of names of the removed elements
		for element in elements_to_remove:
			if type(element) == AsmStruct:
				id_remove_kind(element.name, ID_KIND_STRUCT_NAME)
			elif type(element) == AsmMacro:
				id_remove_kind(element.name, ID_KIND_MACRO_NAME)
	# Read the source
	asm_file_contents = open(asm_file_name, "r", encoding="utf-8").read()
	# Find includes, fix their paths and handle em recoursively
	includes = re.findall(r'^include (["\'])(.*)\1', asm_file_contents, flags=re.MULTILINE)
	for include in includes:
		include = include[1].replace('\\', '/');
		full_path = subdir + '/' + include;
		# If the path isn't valid, maybe that's not relative path
		if not os.path.isfile(full_path):
			full_path = include
		new_subdir = full_path.rsplit('/', 1)[0]
		handle_file(handled_files, full_path, new_subdir)
	# Only collect declarations from the file if it wasn't parsed before
	if should_get_declarations and not clean_generated_stuff:
		get_declarations(asm_file_contents, asm_file_name)

kernel_files = []

# Load remembered list of symbols
if os.path.isfile('asmxygen.elements.pickle'):
	print('Reading existing dump of symbols')
	(elements, id2kind) = pickle.load(open('asmxygen.elements.pickle', 'rb'))

handle_file(kernel_files, "./kernel.asm");

if dump_symbols:
	stdout = sys.stdout
	sys.stdout = open('asmxygen.dump.txt', 'w', encoding = 'utf-8')
	for asm_element in elements:
		asm_element.dump()
	sys.stdout = stdout

if clean_generated_stuff:
	kernel_files_set = set(kernel_files)
	for file in kernel_files:
		doxygen_file = f"{doxygen_src_path}/{file}"
		if (os.path.isfile(doxygen_file)):
			print(f"Removing {file}... ", end = '')
			os.remove(doxygen_file)
			print("Done.")
elif not noemit:
	print(f"Writing doumented sources to {doxygen_src_path}")

	i = 0
	new_elements = [x for x in elements if x.new]
	for element in new_elements:
		print(f"[{i + 1}/{len(new_elements)}] Emitting {element.name} from {element.location}")
		element.emit(doxygen_src_path)
		i += 1

	print(f"Writing dump of symbols to asmxygen.elements.pickle")

	# Now when the new elements already was written, there's no new elements anymore
	for element in elements:
		element.new = False
	pickle.dump((elements, id2kind), open('asmxygen.elements.pickle', 'wb'))

if print_stats:
	var_count = 0
	mac_count = 0
	lab_count = 0
	fun_count = 0
	uni_count = 0
	str_count = 0
	for element in elements:
		if type(element) == AsmVariable:
			var_count += 1
		elif type(element) == AsmMacro:
			mac_count += 1
		elif type(element) == AsmLabel:
			lab_count += 1
		elif type(element) == AsmFunction:
			fun_count += 1
		elif type(element) == AsmUnion:
			uni_count += 1
		elif type(element) == AsmStruct:
			str_count += 1
	print(f'Parsed variable count: {var_count}')
	print(f'Parsed macro count: {mac_count}')
	print(f'Parsed label count: {lab_count}')
	print(f'Parsed function count: {fun_count}')
	print(f'Parsed union type count: {uni_count}')
	print(f'Parsed structure type count: {str_count}')

if enable_warnings:
	open('asmxygen.txt', "w", encoding = "utf-8").write(warnings)
