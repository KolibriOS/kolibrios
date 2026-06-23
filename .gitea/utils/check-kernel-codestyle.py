#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
# SPDX-FileCopyrightText: 2026 KolibriOS team

# Kernel codestyle checker (port of the former checker.pl).
# Reads an assembly source file and validates the KolibriOS kernel style rules.
# Usage: check-kernel-codestyle.py <file>   (falls back to stdin if none given)
# Exits 0 when clean; on the first violation prints "Style error ..." to stderr
# and exits 1.

import re
import sys

# Full FASM mnemonic set, taken verbatim from the original checker.pl. Only these
# tokens are treated as instructions for the indent/spacing rules below.
_MNEMONICS = r"""stdcall|ccall|invoke|cinvoke|bt|in|ja|jb|jc|je|jg|jl|jo|jp|js|jz|or|aaa|aad|aam|aas|adc|add|and|bsf|bsr|btc|btr|bts|cbw|cdq|clc|cld|cli|cmc|cmp|cqo|cwd|daa|das|dec|div|fld|fst|hlt|inc|ins|int|jae|jbe|jge|jle|jmp|jna|jnb|jnc|jne|jng|jnl|jno|jnp|jns|jnz|jpe|jpo|lar|lds|lea|les|lfs|lgs|lsl|lss|ltr|mov|mul|neg|nop|not|out|pop|por|rcl|rcr|rep|ret|rol|ror|rsm|sal|sar|sbb|shl|shr|stc|std|sti|str|sub|ud2|xor|arpl|call|cdqe|clgi|clts|cmps|cwde|dppd|dpps|emms|fabs|fadd|fbld|fchs|fcom|fcos|fdiv|feni|fild|fist|fld1|fldz|fmul|fnop|fsin|fstp|fsub|ftst|fxam|fxch|idiv|imul|insb|insd|insw|int1|int3|into|invd|iret|jcxz|jnae|jnbe|jnge|jnle|lahf|lgdt|lidt|lldt|lmsw|lock|lods|loop|movd|movq|movs|orpd|orps|outs|pand|popa|popd|popf|popq|popw|push|pxor|repe|repz|retd|retf|retn|retq|retw|sahf|salc|scas|seta|setb|setc|sete|setg|setl|seto|setp|sets|setz|sgdt|shld|shrd|sidt|sldt|smsw|stgi|stos|test|verr|verw|vpor|wait|xadd|xchg|xlat|addpd|addps|addsd|addss|andpd|andps|bound|bswap|cmova|cmovb|cmovc|cmove|cmovg|cmovl|cmovo|cmovp|cmovs|cmovz|cmppd|cmpps|cmpsb|cmpsd|cmpsq|cmpss|cmpsw|cpuid|crc32|divpd|divps|divsd|divss|enter|extrq|f2xm1|faddp|fbstp|fclex|fcomi|fcomp|fdisi|fdivp|fdivr|femms|ffree|fiadd|ficom|fidiv|fimul|finit|fistp|fisub|fldcw|fldpi|fmulp|fneni|fprem|fptan|fsave|fsqrt|fstcw|fstsw|fsubp|fsubr|fucom|fwait|fyl2x|icebp|iretd|iretq|iretw|jecxz|jrcxz|lddqu|leave|lodsb|lodsd|lodsq|lodsw|loopd|loope|loopq|loopw|loopz|lzcnt|maxpd|maxps|maxsd|maxss|minpd|minps|minsd|minss|movbe|movsb|movsd|movsq|movss|movsw|movsx|movzx|mulpd|mulps|mulsd|mulss|mwait|outsb|outsd|outsw|pabsb|pabsd|pabsw|paddb|paddd|paddq|paddw|pandn|pause|pavgb|pavgw|pf2id|pf2iw|pfacc|pfadd|pfmax|pfmin|pfmul|pfrcp|pfsub|pi2fd|pi2fw|popad|popaw|popfd|popfq|popfw|pslld|psllq|psllw|psrad|psraw|psrld|psrlq|psrlw|psubb|psubd|psubq|psubw|ptest|pusha|pushd|pushf|pushq|pushw|rcpps|rcpss|rdmsr|rdpmc|rdtsc|repne|repnz|retfd|retfq|retfw|retnd|retnq|retnw|scasb|scasd|scasq|scasw|setae|setbe|setge|setle|setna|setnb|setnc|setne|setng|setnl|setno|setnp|setns|setnz|setpe|setpo|stosb|stosd|stosq|stosw|subpd|subps|subsd|subss|vdppd|vdpps|vmovd|vmovq|vmrun|vmxon|vorpd|vorps|vpand|vpxor|wrmsr|xlatb|xorpd|xorps|xsave|aesdec|aesenc|aesimc|andnpd|andnps|cmovae|cmovbe|cmovge|cmovle|cmovna|cmovnb|cmovnc|cmovne|cmovng|cmovnl|cmovno|cmovnp|cmovns|cmovnz|cmovpe|cmovpo|comisd|comiss|fcmovb|fcmove|fcmovu|fcomip|fcompp|fdivrp|ffreep|ficomp|fidivr|fisttp|fisubr|fldenv|fldl2e|fldl2t|fldlg2|fldln2|fnclex|fndisi|fninit|fnsave|fnstcw|fnstsw|fpatan|fprem1|frstor|frstpm|fsaved|fsavew|fscale|fsetpm|fstenv|fsubrp|fucomi|fucomp|fxsave|getsec|haddpd|haddps|hsubpd|hsubps|invept|invlpg|lfence|looped|loopeq|loopew|loopne|loopnz|loopzd|loopzq|loopzw|mfence|movapd|movaps|movdqa|movdqu|movhpd|movhps|movlpd|movlps|movnti|movntq|movsxd|movupd|movups|paddsb|paddsw|pextrb|pextrd|pextrq|pextrw|pfnacc|pfsubr|phaddd|phaddw|phsubd|phsubw|pinsrb|pinsrd|pinsrq|pinsrw|pmaxsb|pmaxsd|pmaxsw|pmaxub|pmaxud|pmaxuw|pminsb|pminsd|pminsw|pminub|pminud|pminuw|pmuldq|pmulhw|pmulld|pmullw|popcnt|psadbw|pshufb|pshufd|pshufw|psignb|psignd|psignw|pslldq|psrldq|psubsb|psubsw|pswapd|pushad|pushaw|pushfd|pushfq|pushfw|rdmsrq|rdrand|rdtscp|setalc|setnae|setnbe|setnge|setnle|sfence|shufpd|shufps|skinit|sqrtpd|sqrtps|sqrtsd|sqrtss|swapgs|sysret|vaddpd|vaddps|vaddsd|vaddss|vandpd|vandps|vcmppd|vcmpps|vcmpsd|vcmpss|vdivpd|vdivps|vdivsd|vdivss|vlddqu|vmaxpd|vmaxps|vmaxsd|vmaxss|vmcall|vminpd|vminps|vminsd|vminss|vmload|vmovsd|vmovss|vmread|vmsave|vmulpd|vmulps|vmulsd|vmulss|vmxoff|vpabsb|vpabsd|vpabsw|vpaddb|vpaddd|vpaddq|vpaddw|vpandn|vpavgb|vpavgw|vpcmov|vpcomb|vpcomd|vpcomq|vpcomw|vpperm|vprotb|vprotd|vprotq|vprotw|vpshab|vpshad|vpshaq|vpshaw|vpshlb|vpshld|vpshlq|vpshlw|vpslld|vpsllq|vpsllw|vpsrad|vpsraw|vpsrld|vpsrlq|vpsrlw|vpsubb|vpsubd|vpsubq|vpsubw|vptest|vrcpps|vrcpss|vsubpd|vsubps|vsubsd|vsubss|vxorpd|vxorps|wbinvd|wrmsrq|xgetbv|xrstor|xsetbv|blendpd|blendps|clflush|cmovnae|cmovnbe|cmovnge|cmovnle|cmpeqpd|cmpeqps|cmpeqsd|cmpeqss|cmplepd|cmpleps|cmplesd|cmpless|cmpltpd|cmpltps|cmpltsd|cmpltss|cmpxchg|fcmovbe|fcmovnb|fcmovne|fcmovnu|fdecstp|fincstp|fldenvd|fldenvw|fnsaved|fnsavew|fnstenv|frndint|frstord|frstorw|fsincos|fstenvd|fstenvw|fucomip|fucompp|fxrstor|fxtract|fyl2xp1|insertq|invlpga|invvpid|ldmxcsr|loopned|loopneq|loopnew|loopnzd|loopnzq|loopnzw|monitor|movddup|movdq2q|movhlps|movlhps|movntdq|movntpd|movntps|movntsd|movntss|movq2dq|mpsadbw|paddusb|paddusw|palignr|pavgusb|pblendw|pcmpeqb|pcmpeqd|pcmpeqq|pcmpeqw|pcmpgtb|pcmpgtd|pcmpgtq|pcmpgtw|pfcmpeq|pfcmpge|pfcmpgt|pfpnacc|pfrsqrt|phaddsw|phsubsw|pmaddwd|pmulhrw|pmulhuw|pmuludq|pshufhw|pshuflw|psubusb|psubusw|roundpd|roundps|roundsd|roundss|rsqrtps|rsqrtss|stmxcsr|syscall|sysexit|sysretq|ucomisd|ucomiss|vaesdec|vaesenc|vaesimc|vandnpd|vandnps|vcomisd|vcomiss|vfrczpd|vfrczps|vfrczsd|vfrczss|vhaddpd|vhaddps|vhsubpd|vhsubps|vmclear|vmmcall|vmovapd|vmovaps|vmovdqa|vmovdqu|vmovhpd|vmovhps|vmovlpd|vmovlps|vmovupd|vmovups|vmptrld|vmptrst|vmwrite|vpaddsb|vpaddsw|vpcomub|vpcomud|vpcomuq|vpcomuw|vpextrb|vpextrd|vpextrq|vpextrw|vphaddd|vphaddw|vphsubd|vphsubw|vpinsrb|vpinsrd|vpinsrq|vpinsrw|vpmaxsb|vpmaxsd|vpmaxsw|vpmaxub|vpmaxud|vpmaxuw|vpminsb|vpminsd|vpminsw|vpminub|vpminud|vpminuw|vpmuldq|vpmulhw|vpmulld|vpmullw|vpsadbw|vpshufb|vpshufd|vpsignb|vpsignd|vpsignw|vpslldq|vpsrldq|vpsubsb|vpsubsw|vshufpd|vshufps|vsqrtpd|vsqrtps|vsqrtsd|vsqrtss|vtestpd|vtestps|addsubpd|addsubps|blendvpd|blendvps|cmpneqpd|cmpneqps|cmpneqsd|cmpneqss|cmpnlepd|cmpnleps|cmpnlesd|cmpnless|cmpnltpd|cmpnltps|cmpnltsd|cmpnltss|cmpordpd|cmpordps|cmpordsd|cmpordss|cvtdq2pd|cvtdq2ps|cvtpd2dq|cvtpd2pi|cvtpd2ps|cvtpi2pd|cvtpi2ps|cvtps2dq|cvtps2pd|cvtps2pi|cvtsd2si|cvtsd2ss|cvtsi2sd|cvtsi2ss|cvtss2sd|cvtss2si|fcmovnbe|fnstenvd|fnstenvw|insertps|maskmovq|movmskpd|movmskps|movntdqa|movshdup|movsldup|packssdw|packsswb|packusdw|packuswb|pblendvb|pfrcpit1|pfrcpit2|pfrsqit1|pmovmskb|pmovsxbd|pmovsxbq|pmovsxbw|pmovsxdq|pmovsxwd|pmovsxwq|pmovzxbd|pmovzxbq|pmovzxbw|pmovzxdq|pmovzxwd|pmovzxwq|pmulhrsw|prefetch|rdfsbase|rdgsbase|sysenter|sysexitq|unpckhpd|unpckhps|unpcklpd|unpcklps|vblendpd|vblendps|vcmpeqpd|vcmpeqps|vcmpeqsd|vcmpeqss|vcmpgepd|vcmpgeps|vcmpgesd|vcmpgess|vcmpgtpd|vcmpgtps|vcmpgtsd|vcmpgtss|vcmplepd|vcmpleps|vcmplesd|vcmpless|vcmpltpd|vcmpltps|vcmpltsd|vcmpltss|vfmaddpd|vfmaddps|vfmaddsd|vfmaddss|vfmsubpd|vfmsubps|vfmsubsd|vfmsubss|vldmxcsr|vmlaunch|vmovddup|vmovhlps|vmovlhps|vmovntdq|vmovntpd|vmovntps|vmpsadbw|vmresume|vpaddusb|vpaddusw|vpalignr|vpblendw|vpcmpeqb|vpcmpeqd|vpcmpeqq|vpcmpeqw|vpcmpgtb|vpcmpgtd|vpcmpgtq|vpcmpgtw|vpcomeqb|vpcomeqd|vpcomeqq|vpcomeqw|vpcomgeb|vpcomged|vpcomgeq|vpcomgew|vpcomgtb|vpcomgtd|vpcomgtq|vpcomgtw|vpcomleb|vpcomled|vpcomleq|vpcomlew|vpcomltb|vpcomltd|vpcomltq|vpcomltw|vphaddbd|vphaddbq|vphaddbw|vphadddq|vphaddsw|vphaddwd|vphaddwq|vphsubbw|vphsubdq|vphsubsw|vphsubwd|vpmacsdd|vpmacswd|vpmacsww|vpmaddwd|vpmulhuw|vpmuludq|vpshufhw|vpshuflw|vpsubusb|vpsubusw|vroundpd|vroundps|vroundsd|vroundss|vrsqrtps|vrsqrtss|vstmxcsr|vucomisd|vucomiss|vzeroall|wrfsbase|wrgsbase|xsaveopt|cmpxchg8b|cvttpd2dq|cvttpd2pi|cvttps2dq|cvttps2pi|cvttsd2si|cvttss2si|extractps|pclmulqdq|pcmpestri|pcmpestrm|pcmpistri|pcmpistrm|pmaddubsw|prefetchw|punpckhbw|punpckhdq|punpckhwd|punpcklbw|punpckldq|punpcklwd|vaddsubpd|vaddsubps|vblendvpd|vblendvps|vcmpneqpd|vcmpneqps|vcmpneqsd|vcmpneqss|vcmpngepd|vcmpngeps|vcmpngesd|vcmpngess|vcmpngtpd|vcmpngtps|vcmpngtsd|vcmpngtss|vcmpnlepd|vcmpnleps|vcmpnlesd|vcmpnless|vcmpnltpd|vcmpnltps|vcmpnltsd|vcmpnltss|vcmpordpd|vcmpordps|vcmpordsd|vcmpordss|vcvtdq2pd|vcvtdq2ps|vcvtpd2dq|vcvtpd2ps|vcvtph2ps|vcvtps2dq|vcvtps2pd|vcvtps2ph|vcvtsd2si|vcvtsd2ss|vcvtsi2sd|vcvtsi2ss|vcvtss2sd|vcvtss2si|vfnmaddpd|vfnmaddps|vfnmaddsd|vfnmaddss|vfnmsubpd|vfnmsubps|vfnmsubsd|vfnmsubss|vinsertps|vmovmskpd|vmovmskps|vmovntdqa|vmovshdup|vmovsldup|vpackssdw|vpacksswb|vpackusdw|vpackuswb|vpblendvb|vpcomequb|vpcomequd|vpcomequq|vpcomequw|vpcomgeub|vpcomgeud|vpcomgeuq|vpcomgeuw|vpcomgtub|vpcomgtud|vpcomgtuq|vpcomgtuw|vpcomleub|vpcomleud|vpcomleuq|vpcomleuw|vpcomltub|vpcomltud|vpcomltuq|vpcomltuw|vpcomneqb|vpcomneqd|vpcomneqq|vpcomneqw|vpermilpd|vpermilps|vphaddubd|vphaddubq|vphaddubw|vphaddudq|vphadduwd|vphadduwq|vpmacsdqh|vpmacsdql|vpmacssdd|vpmacsswd|vpmacssww|vpmadcswd|vpmovmskb|vpmovsxbd|vpmovsxbq|vpmovsxbw|vpmovsxdq|vpmovsxwd|vpmovsxwq|vpmovzxbd|vpmovzxbq|vpmovzxbw|vpmovzxdq|vpmovzxwd|vpmovzxwq|vpmulhrsw|vunpckhpd|vunpckhps|vunpcklpd|vunpcklps|aesdeclast|aesenclast|cmpunordpd|cmpunordps|cmpunordsd|cmpunordss|cmpxchg16b|loadall286|loadall386|maskmovdqu|phminposuw|prefetcht0|prefetcht1|prefetcht2|punpckhqdq|punpcklqdq|vcmptruepd|vcmptrueps|vcmptruesd|vcmptruess|vcvttpd2dq|vcvttps2dq|vcvttsd2si|vcvttss2si|vextractps|vmaskmovpd|vmaskmovps|vpclmulqdq|vpcmpestri|vpcmpestrm|vpcmpistri|vpcmpistrm|vpcomnequb|vpcomnequd|vpcomnequq|vpcomnequw|vpcomtrueb|vpcomtrued|vpcomtrueq|vpcomtruew|vperm2f128|vpermil2pd|vpermil2ps|vpmacssdqh|vpmacssdql|vpmadcsswd|vpmaddubsw|vpunpckhbw|vpunpckhdq|vpunpckhwd|vpunpcklbw|vpunpckldq|vpunpcklwd|vzeroupper|xsaveopt64|prefetchnta|vaesdeclast|vaesenclast|vcmpeq_ospd|vcmpeq_osps|vcmpeq_ossd|vcmpeq_osss|vcmpeq_uqpd|vcmpeq_uqps|vcmpeq_uqsd|vcmpeq_uqss|vcmpeq_uspd|vcmpeq_usps|vcmpeq_ussd|vcmpeq_usss|vcmpfalsepd|vcmpfalseps|vcmpfalsesd|vcmpfalsess|vcmpge_oqpd|vcmpge_oqps|vcmpge_oqsd|vcmpge_oqss|vcmpgt_oqpd|vcmpgt_oqps|vcmpgt_oqsd|vcmpgt_oqss|vcmple_oqpd|vcmple_oqps|vcmple_oqsd|vcmple_oqss|vcmplt_oqpd|vcmplt_oqps|vcmplt_oqsd|vcmplt_oqss|vcmpord_spd|vcmpord_sps|vcmpord_ssd|vcmpord_sss|vcmpunordpd|vcmpunordps|vcmpunordsd|vcmpunordss|vfmadd132pd|vfmadd132ps|vfmadd132sd|vfmadd132ss|vfmadd213pd|vfmadd213ps|vfmadd213sd|vfmadd213ss|vfmadd231pd|vfmadd231ps|vfmadd231sd|vfmadd231ss|vfmaddsubpd|vfmaddsubps|vfmsub132pd|vfmsub132ps|vfmsub132sd|vfmsub132ss|vfmsub213pd|vfmsub213ps|vfmsub213sd|vfmsub213ss|vfmsub231pd|vfmsub231ps|vfmsub231sd|vfmsub231ss|vfmsubaddpd|vfmsubaddps|vinsertf128|vmaskmovdqu|vpcomfalseb|vpcomfalsed|vpcomfalseq|vpcomfalsew|vpcomtrueub|vpcomtrueud|vpcomtrueuq|vpcomtrueuw|vphminposuw|vpunpckhqdq|vpunpcklqdq|vbroadcastsd|vbroadcastss|vcmpneq_oqpd|vcmpneq_oqps|vcmpneq_oqsd|vcmpneq_oqss|vcmpneq_ospd|vcmpneq_osps|vcmpneq_ossd|vcmpneq_osss|vcmpneq_uspd|vcmpneq_usps|vcmpneq_ussd|vcmpneq_usss|vcmpnge_uqpd|vcmpnge_uqps|vcmpnge_uqsd|vcmpnge_uqss|vcmpngt_uqpd|vcmpngt_uqps|vcmpngt_uqsd|vcmpngt_uqss|vcmpnle_uqpd|vcmpnle_uqps|vcmpnle_uqsd|vcmpnle_uqss|vcmpnlt_uqpd|vcmpnlt_uqps|vcmpnlt_uqsd|vcmpnlt_uqss|vextractf128|vfnmadd132pd|vfnmadd132ps|vfnmadd132sd|vfnmadd132ss|vfnmadd213pd|vfnmadd213ps|vfnmadd213sd|vfnmadd213ss|vfnmadd231pd|vfnmadd231ps|vfnmadd231sd|vfnmadd231ss|vfnmsub132pd|vfnmsub132ps|vfnmsub132sd|vfnmsub132ss|vfnmsub213pd|vfnmsub213ps|vfnmsub213sd|vfnmsub213ss|vfnmsub231pd|vfnmsub231ps|vfnmsub231sd|vfnmsub231ss|vpcomfalseub|vpcomfalseud|vpcomfalseuq|vpcomfalseuw|vpermilmo2pd|vpermilmo2ps|vpermilmz2pd|vpermilmz2ps|vpermiltd2pd|vpermiltd2ps|vcmptrue_uspd|vcmptrue_usps|vcmptrue_ussd|vcmptrue_usss|vcmpunord_spd|vcmpunord_sps|vcmpunord_ssd|vcmpunord_sss|vbroadcastf128|vcmpfalse_ospd|vcmpfalse_osps|vcmpfalse_ossd|vcmpfalse_osss|vfmaddsub132pd|vfmaddsub132ps|vfmaddsub213pd|vfmaddsub213ps|vfmaddsub231pd|vfmaddsub231ps|vfmsubadd132pd|vfmsubadd132ps|vfmsubadd213pd|vfmsubadd213ps|vfmsubadd231pd|vfmsubadd231ps|aeskeygenassist|vaeskeygenassist"""

# label? indent mnemonic spaces args comment?
_LINE_RE = re.compile(
    r'^(\s*[^;"\'\s$][^;"\'\s]*:)?(\s*)(' + _MNEMONICS + r')(\s+)([^;]*)(;.*)?$'
)

# Mnemonics that always take exactly one space before their arguments, like the
# 8-char-or-longer ones, regardless of their short length.
_LONG_MNEMONICS = {"rep", "repz", "repe", "repnz", "repne", "lock"}


def fail(name, lineno, msg):
    sys.stderr.write(f"Style error in {name}:{lineno}: {msg}\n")
    sys.exit(1)


def main():
    if len(sys.argv) > 1:
        name = sys.argv[1]
        with open(name, "rb") as f:
            raw_lines = f.readlines()
    else:
        name = ""
        raw_lines = sys.stdin.buffer.readlines()

    # None    -> not inside a line continuation
    # -1      -> continuation active, but the starting column is unknown
    # >= 0    -> continuation active; following lines must indent by this column
    continued = None

    for lineno, raw in enumerate(raw_lines, start=1):
        # Rule 6. All code and text files should be in UTF-8 without BOM.
        if lineno == 1 and raw[:3] == b"\xef\xbb\xbf":
            fail(name, lineno, "BOM is not allowed.")
        try:
            line = raw.decode("utf-8")
        except UnicodeDecodeError:
            fail(name, lineno, "the file must be in UTF-8.")

        # Rule 1. No tab characters allowed.
        if "\t" in line:
            fail(name, lineno, "no tab characters allowed.")

        if continued is not None:
            m = re.match(r'^(\s*)[^;]', line)
            if continued != -1 and m and len(m.group(1)) != continued:
                fail(name, lineno,
                     "arguments continued in the next line must start from the "
                     "same position as in the first line")
            if not re.search(r'\\\s*(;.*)?$', line):
                continued = None
            continue

        m = _LINE_RE.match(line)
        if m:
            label = m.group(1) or ""
            indent, mnem, spaces, args = m.group(2), m.group(3), m.group(4), m.group(5)

            # Rule 2. Any label must be on a separate line. It is not allowed to
            # have a label and anything except a comment on the same line.
            if label:
                fail(name, lineno, "any label must be on a separate line.")

            # Rule 3. Lines with commands must start with 8 spaces.
            # A mnemonic is short if its length is less than 8.
            # Arguments for short mnemonics must start in column 16.
            # Arguments for long mnemonics must be separated from the mnemonic by
            # exactly one space.
            if indent != "        ":
                fail(name, lineno, "indent for commands must be 8 spaces.")
            if len(mnem) >= 8 or mnem in _LONG_MNEMONICS:
                if args != "" and spaces != " ":
                    fail(name, lineno,
                         "arguments for long mnemonics must be separated from the "
                         "mnemonic by exactly one space.")
            else:
                if args != "" and spaces != " " * (8 - len(mnem)):
                    fail(name, lineno,
                         "arguments for short mnemonics must start in the column 16.")

            args = re.sub(r'\s*$', "", args)
            # Arguments must be separated with a comma and exactly one space after
            # a comma. Split off quoted substrings (indices 1, 2 mod 3) so commas
            # inside string literals are not checked.
            parts = re.split(r'(["\'])(.*?\1)', args)
            for i, segment in enumerate(parts):
                if i % 3 == 0 and segment is not None and \
                        re.search(r',(\s{2,})?([^\\\s]|$)', segment):
                    fail(name, lineno,
                         "arguments must be separated with a comma and exactly one "
                         "space after a comma.")

            if re.search(r'\\$', args):
                continued = len(indent) + len(mnem) + len(spaces)
        else:
            if re.search(r'\\\s*(;.*)?$', line):
                continued = -1

    sys.exit(0)


if __name__ == "__main__":
    main()
