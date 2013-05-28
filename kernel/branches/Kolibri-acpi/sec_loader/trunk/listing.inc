; Listing generator
; LocoDelAssembly 2007.06.01

INSTRUCTIONS equ bt in ja jb jc je jg jl jo jp js jz or                        \
        aaa     aad aam aas adc add and bsf bsr btc btr bts cbw cdq clc \
                cld cli cmc cmp cqo cwd daa das dec div fld fst hlt inc ins   \
                int jae jbe jge jle jmp jna jnb jnc jne jng jnl jno jnp jns   \
                jnz jpe jpo lar lds lea les lfs lgs lsl lss ltr mov mul neg   \
                nop not out pop por rcl rcr ret rol ror rsm sal sar sbb shl   \
                shr stc std sti str sub ud2 xor                               \
                arpl call cdqe clgi clts cmps cwde emms fabs fadd fbld fchs   \
                fcom fcos fdiv feni fild fist fld1 fldz fmul fnop fsin fstp   \
                fsub ftst fxam fxch idiv imul insb insd insw int1 int3 into   \
                invd iret jcxz jnae jnbe jnge jnle lahf lgdt lidt lldt lmsw   \
                lods loop movd movq movs orpd orps outs pand popa popd popf   \
                popq popw push pxor retd retf retn retq retw sahf salc scas   \
                seta setb setc sete setg setl seto setp sets setz sgdt shld   \
                shrd sidt sldt smsw stgi stos test verr verw wait xadd xchg   \
                xlat                                                          \
                addpd addps addsd addss andpd andps bound bswap cmova cmovb   \
                cmovc cmove cmovg cmovl cmovo cmovp cmovs cmovz cmppd cmpps   \
                cmpsb cmpsd cmpsq cmpss cmpsw cpuid divpd divps divsd divss   \
                enter f2xm1 faddp fbstp fclex fcomi fcomp fdisi fdivp fdivr   \
                femms ffree fiadd ficom fidiv fimul finit fistp fisub fldcw   \
                fldpi fmulp fneni fprem fptan fsave fsqrt fstcw fstsw fsubp   \
                fsubr fucom fwait fyl2x icebp iretd iretq iretw jecxz jrcxz   \
                lddqu leave lodsb lodsd lodsq lodsw loopd loope loopq loopw   \
                loopz maxpd maxps maxsd maxss minpd minps minsd minss movsb   \
                movsd movsq movss movsw movsx movzx mulpd mulps mulsd mulss   \
                mwait outsb outsd outsw pabsb pabsd pabsw paddb paddd paddq   \
                paddw pandn pause pavgb pavgw pf2id pf2iw pfacc pfadd pfmax   \
                pfmin pfmul pfrcp pfsub pi2fd pi2fw popad popaw popfd popfq   \
                popfw pslld psllq psllw psrad psraw psrld psrlq psrlw psubb   \
                psubd psubq psubw pusha pushd pushf pushq pushw rcpps rcpss   \
                rdmsr rdpmc rdtsc retfd retfq retfw retnd retnq retnw scasb   \
                scasd scasq scasw setae setbe setge setle setna setnb setnc   \
                setne setng setnl setno setnp setns setnz setpe setpo stosb   \
                stosd stosq stosw subpd subps subsd subss vmrun vmxon wrmsr   \
                xlatb xorpd xorps                                             \
                andnpd andnps cmovae cmovbe cmovge cmovle cmovna cmovnb cmovnc\
                cmovne cmovng cmovnl cmovno cmovnp cmovns cmovnz cmovpe cmovpo\
                comisd comiss fcmovb fcmove fcmovu fcomip fcompp fdivrp ffreep\
                ficomp fidivr fisttp fisubr fldenv fldl2e fldl2t fldlg2 fldln2\
                fnclex fndisi fninit fnsave fnstcw fnstsw fpatan fprem1 frstor\
                frstpm fscale fsetpm fstenv fsubrp fucomi fucomp fxsave haddpd\
                haddps hsubpd hsubps invlpg lfence looped loopeq loopew loopne\
                loopnz loopzd loopzq loopzw mfence movapd movaps movdqa movdqu\
                movhpd movhps movlpd movlps movnti movntq movsxd movupd movups\
                paddsb paddsw pextrw pfnacc pfsubr phaddd phaddw phsubd phsubw\
                pinsrw pmaxsw pmaxub pminsw pminub pmulhw pmullw psadbw pshufb\
                pshufd pshufw psignb psignd psignw pslldq psrldq psubsb psubsw\
                pswapd pushad pushaw pushfd pushfq pushfw rdmsrq rdtscp setalc\
                setnae setnbe setnge setnle sfence shufpd shufps skinit sqrtpd\
                sqrtps sqrtsd sqrtss swapgs sysret vmcall vmload vmread vmsave\
                vmxoff wbinvd wrmsrq                                          \
                clflush cmovnae cmovnbe cmovnge cmovnle cmpeqpd cmpeqps       \
                cmpeqsd cmpeqss cmplepd cmpleps cmplesd cmpless cmpltpd       \
                cmpltps cmpltsd cmpltss cmpxchg fcmovbe fcmovnb fcmovne       \
                fcmovnu fdecstp fincstp fnstenv frndint fsincos fucomip       \
                fucompp fxrstor fxtract fyl2xp1 invlpga ldmxcsr loopned       \
                loopneq loopnew loopnzd loopnzq loopnzw monitor movddup       \
                movdq2q movhlps movlhps movntdq movntpd movntps movq2dq       \
                paddusb paddusw palignr pavgusb pcmpeqb pcmpeqd pcmpeqw       \
                pcmpgtb pcmpgtd pcmpgtw pfcmpeq pfcmpge pfcmpgt pfpnacc       \
                pfrsqrt phaddsw phsubsw pmaddwd pmulhrw pmulhuw pmuludq       \
                pshufhw pshuflw psubusb psubusw rsqrtps rsqrtss stmxcsr       \
                syscall sysexit sysretq ucomisd ucomiss vmclear vmmcall       \
                vmptrld vmptrst vmwrite                                       \
                addsubpd addsubps cmpneqpd cmpneqps cmpneqsd cmpneqss cmpnlepd\
                cmpnleps cmpnlesd cmpnless cmpnltpd cmpnltps cmpnltsd cmpnltss\
                cmpordpd cmpordps cmpordsd cmpordss cvtdq2pd cvtdq2ps cvtpd2dq\
                cvtpd2pi cvtpd2ps cvtpi2pd cvtpi2ps cvtps2dq cvtps2pd cvtps2pi\
                cvtsd2si cvtsd2ss cvtsi2sd cvtsi2ss cvtss2sd cvtss2si fcmovnbe\
                maskmovq movmskpd movmskps movshdup movsldup packssdw packsswb\
                packuswb pfrcpit1 pfrcpit2 pfrsqit1 pmovmskb pmulhrsw prefetch\
                sysenter sysexitq unpckhpd unpckhps unpcklpd unpcklps vmlaunch\
                vmresume                                                      \
                cmpxchg8b cvttpd2dq cvttpd2pi cvttps2dq cvttps2pi cvttsd2si   \
                cvttss2si pmaddubsw prefetchw punpckhbw punpckhdq punpckhwd   \
                punpcklbw punpckldq punpcklwd                                 \
                cmpunordpd cmpunordps cmpunordsd cmpunordss cmpxchg16b        \
                loadall286 loadall386 maskmovdqu prefetcht0 prefetcht1        \
                prefetcht2 punpckhqdq punpcklqdq prefetchnta

PREFIXES equ     rep lock repe repz repne repnz

DATA_DEFINITORS  equ db dw du dd dp df dq dt file
DATA_RESERVERS   equ rb rw rd rp rf rq rt

CRLF              equ 13, 10  ; Remove 13 for Linux
MAX_BYTES         equ 13

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; MODE MACROSES ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
macro use16
{
use16
  _USE = 16
}

macro use32
{
use32
  _USE = 32
}

macro use64
{
use64
  _USE = 64
}

macro detect_mode
{
local aux

  _USE = 32

  virtual at 0
        xchg    eax, eax
    load aux byte from 0

    if aux = $66
      _USE = 16
    else if aux = $87
      _USE = 64
    end if
  end virtual
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;; DISPLAYING MACROSES ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

macro display_address address*
{
local aux, digit

  aux  = address

  repeat _USE / 4
    digit = aux shr (_USE - 4 * %) and $F

    display digit + '0' + ('9' - 'B') and (9 - digit) shr 4 and $F
  end repeat

  display ': '
}

macro display_bytes pointer
{
local aux, size, digit

  size = $ - pointer

  if size > MAX_BYTES
    size = MAX_BYTES
  end if

  repeat size
    load aux byte from pointer+%-1

      digit = aux shr 4
      display digit + '0' + ('9' - 'B') and (9 - digit) shr 4 and $F

      digit = aux and $F
      display digit + '0' + ('9' - 'B') and (9 - digit) shr 4 and $F


    display ' '
  end repeat

  repeat MAX_BYTES - size
    display '   '
  end repeat
}

; The macro below in some situations doesn't adds a space to separate things unfortunatelly, so for readability ensurance
; another one will be used instead...
;macro display_args [args]
;{
;common
;  aux = 1
;
;forward
;  if ~args eq
;    if aux
;      display ' '
;    else
;      display ', '
;    end if
;
;    aux = 0
;
;    match =ON, _RESOLVE_EQUATES
;    \{
;      match args, args
;      \\{
;        irps arg, args
;        \\\{
;          display \\\`arg
;        \\\}
;      \\}
;    \}
;    match =OFF, _RESOLVE_EQUATES
;    \{
;      irps arg, args
;      \\{
;        display \\`arg
;      \\}
;
;    \}
;  end if
;}

; This one separates everything with one space. A very ugly listing but at least you will not see things
; like "push ebxesiedi" nor "ret word0"

macro display_args [args]
{
common
  aux = 1

forward
  if ~args eq
    if ~aux
      display ','
    end if

    aux = 0

    match =ON, _RESOLVE_EQUATES
    \{
      match args, args
      \\{
        if ~args eqtype ""
          irps arg, args
          \\\{
              display ' ', \\\`arg
          \\\}
        else
          display " '", args, "'"
        end if
      \\}
    \}
    match =OFF, _RESOLVE_EQUATES
    \{
      if ~args eqtype ""
        irps arg, args
        \\{
            display ' ', \\`arg
        \\}
      else
        display " '", args, "'"
      end if
    \}
  end if
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;; INSTRUCTIONS & PREFIXES MACROSES ;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

macro prefix mnemonic
{
local aux

  macro mnemonic [args]
  \{
  \common
    match =1, _ON_VIRTUAL\\{_LISTING equ 0\\}
    match =equ, args\\{_LISTING equ 0\\}
    match =equ any, args\\{_LISTING equ 0\\}

    match =1, _LISTING
    \\{
      display_address $
      aux = $
    \\}

    mnemonic

    match =1, _LISTING
    \\{
      display_bytes aux

      display \`mnemonic
      display CRLF
    \\}

    match =1, _ON_VIRTUAL\\{restore _LISTING\\}
    match =equ, args\\{restore _LISTING\\}
    match =equ any, args\\{restore _LISTING\\}

    def_prefix mnemonic
    args
    purge mnemonic
  \}
}

macro def_prefix mnemonic
{
  macro def_prefix mnemonic
  \{
    prefix mnemonic
  \}
  def_prefix mnemonic
}

macro instruction mnemonic
{
local aux

  macro mnemonic [args]
  \{
  \common
    match =1, _ON_VIRTUAL\\{_LISTING equ 0\\}
    match =equ, args\\{_LISTING equ 0\\}
    match =equ any, args\\{_LISTING equ 0\\}

    match =1, _LISTING
    \\{
      display_address $
      aux = $
    \\}

    mnemonic args

    match =1, _LISTING
    \\{
      display_bytes aux

      display \`mnemonic

      virtual at 0
        db \`mnemonic
        repeat 11 - $
          display ' '
        end repeat
      end virtual

      display_args args
      display CRLF
    \\}

    match =1, _ON_VIRTUAL\\{restore _LISTING\\}
    match =equ, args\\{restore _LISTING\\}
    match =equ any, args\\{restore _LISTING\\}
  \}
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DATA MACROSES ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

macro data_define mnemonic
{
local aux
  macro mnemonic [args]
  \{
  \common
    match =1, _ON_VIRTUAL\\{_LISTING equ 0\\}
    match =equ, args\\{_LISTING equ 0\\}
    match =equ any, args\\{_LISTING equ 0\\}

    match =1, _LISTING
    \\{
      display_address $
       aux = $
    \\}

    mnemonic args

    match =1, _LISTING
    \\{
      display_bytes aux

      display \`mnemonic

      display_args args
      display CRLF

      aux = aux + MAX_BYTES

      repeat ($ - aux + MAX_BYTES - 1) / MAX_BYTES
        display_address aux
        display_bytes aux
        display CRLF

        aux = aux + MAX_BYTES
      end repeat
    \\}

    match =1, _ON_VIRTUAL\\{restore _LISTING\\}
    match =equ, args\\{restore _LISTING\\}
    match =equ any, args\\{restore _LISTING\\}
  \}

  struc mnemonic [args]
  \{
  \common
    match =1, _ON_VIRTUAL\\{_LISTING equ 0\\}
    match =equ, args\\{_LISTING equ 0\\}
    match =equ any, args\\{_LISTING equ 0\\}

    match =1, _LISTING
    \\{
      display_address $
      aux = $
    \\}

    . mnemonic args

    match =1, _LISTING
    \\{
      display_bytes aux

      display  \`., ' ', \`mnemonic

      display_args args
      display CRLF

      aux = aux + MAX_BYTES

      repeat ($ - aux + MAX_BYTES - 1) / MAX_BYTES
        display_address aux
        display_bytes aux
        display CRLF

        aux = aux + MAX_BYTES
      end repeat
    \\}

    match =1, _ON_VIRTUAL\\{restore _LISTING\\}
    match =equ, args\\{restore _LISTING\\}
    match =equ any, args\\{restore _LISTING\\}
  \}
}

macro data_reserve mnemonic
{
local aux
  macro mnemonic [args]
  \{
  \common
    match =1, _ON_VIRTUAL\\{_LISTING equ 0\\}
    match =equ, args\\{_LISTING equ 0\\}
    match =equ any, args\\{_LISTING equ 0\\}

    match =1, _LISTING
    \\{
      display_address $
       aux = $
    \\}

    mnemonic args

    match =1, _LISTING
    \\{
      times MAX_BYTES display '   '

      display \`mnemonic

      display_args args
      display CRLF
    \\}

    match =1, _ON_VIRTUAL\\{restore _LISTING\\}
    match =equ, args\\{restore _LISTING\\}
    match =equ any, args\\{restore _LISTING\\}
  \}

  struc mnemonic [args]
  \{
  \common
    match =1, _ON_VIRTUAL\\{_LISTING equ 0\\}
    match =equ, args\\{_LISTING equ 0\\}
    match =equ any, args\\{_LISTING equ 0\\}

    match =1, _LISTING
    \\{
      display_address $
      aux = $
    \\}

    . mnemonic args

    match =1, _LISTING
    \\{
      times MAX_BYTES display '   '

      display  \`., ' ', \`mnemonic

      display_args args
      display CRLF
    \\}

    match =1, _ON_VIRTUAL\\{restore _LISTING\\}
    match =equ, args\\{restore _LISTING\\}
    match =equ any, args\\{restore _LISTING\\}
  \}
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;; LISTING CONTROL MACROSES ;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

macro virtual [args]
{
common
  _ON_VIRTUAL equ 1

  virtual args
}

macro end [args]
{
common
  match =virtual, args\{restore _ON_VIRTUAL\}

  end args
}

macro enable_listing
{
  detect_mode

  match =0, _MACROSES_INSTALLED
  \{
    match instructions, INSTRUCTIONS
    \\{
      irps ins, instructions
      \\\{
        instruction ins
      \\\}
    \\}

    match prefixes, PREFIXES
    \\{
      irps prefix, prefixes
      \\\{
        def_prefix prefix
      \\\}
    \\}

    match data_definitors, DATA_DEFINITORS
    \\{
      irps def, data_definitors
      \\\{
        data_define def
      \\\}
    \\}

    match data_reservers, DATA_RESERVERS
    \\{
      irps def, data_reservers
      \\\{
        data_reserve def
      \\\}
    \\}
  \}

  _MACROSES_INSTALLED equ 1
  _LISTING            equ 1
}

macro disable_listing
{
  _LISTING equ 0
}

macro enable [feature*]
{
forward
  UNKNOWN equ 1

  match =resolve_equates, feature
  \{
    restore _RESOLVE_EQUATES
    _RESOLVE_EQUATES equ ON
    UNKNOWN equ 0
  \}

  match =listing, feature
  \{
    enable_listing
    UNKNOWN equ 0
  \}

  match =1, UNKNOWN
  \{
    display 'ERROR: Unknown "',`feature, '" feature', 13, 10
    err
  \}

  restore UNKNOWN
  restore UNKNOWN
}

macro disable [feature*]
{
  UNKNOWN equ 1

  match =resolve_equates, feature
  \{
    restore _RESOLVE_EQUATES
    _RESOLVE_EQUATES equ OFF
    UNKNOWN equ 0
  \}

  match =listing, feature
  \{
    disable_listing
    UNKNOWN equ 0
  \}

  match =1, UNKNOWN
  \{
    display 'ERROR: Unknown "',`feature, '" feature', 13, 10
    err
  \}

  restore UNKNOWN
  restore UNKNOWN
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; INITIALIZATION ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_MACROSES_INSTALLED equ 0
_ON_VIRTUAL         equ 0

disable resolve_equates
