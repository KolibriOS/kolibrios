/****************************  disasm.h   **********************************
* Author:        Agner Fog
* Date created:  2007-02-21
* Last modified: 2014-12-06
* Project:       objconv
* Module:        disasm.h
* Description:
* Header file for disassembler
*
* Copyright 2007-2014 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#ifndef DISASM_H
#define DISASM_H

// Define tabulator positions for output
#define AsmTab1  8                     // Column for opcode
#define AsmTab2  16                    // Column for first operand
#define AsmTab3  56                    // Column for comment

#define ReplaceIllegalChars 0          // 1 if you want to replace illegal characters in symbol names


// Structure for defining x86 opcode maps
struct SOpcodeDef {
   const char * Name;                  // opcode name
   uint32 InstructionSet;              // mmx, sse, 3dnow, x64, etc.
   uint32 AllowedPrefixes;             // prefixes allowed for this opcode
   uint16 InstructionFormat;           // opcode type, number of operands
   uint16 Destination;                 // type and size of destination operand
   uint16 Source1;                     // type and size of 1. source operand
   uint16 Source2;                     // type and size of 2. source operand
   uint16 Source3;                     // type and size of 3. source operand
   uint16 EVEX;                        // options for interpreting EVEX prefix, may be used for 4. source operand otherwise (unused)
   uint16 MVEX;                        // options for interpreting MVEX prefix: swizzle, convert, mask options
   uint16 TableLink;                   // this entry is a link to another map
   uint16 Options;                     // miscellaneous options
};

/****************     Constants for opcode definition     **********************
I have deliberately not assigned names to these constants because this would
make the tables in opcodes.cpp wery broad with many constant names OR'ed together.
It would be almost impossible to align the columns in a readable way.
Sorry that you have to look up the constants here.

The following tables define the possible values for each field in SOpcodeDef:

Name:
-----
Opcode mnemonic

InstructionSet:
(Some values can be OR'ed):
---------------------------
0:       8086
1:       80186
2:       80286
3:       80386
4:       80486, cpuid
5:       Pentium
6:       Pentium Pro, cmov, fcomi
7:       MMX
8:       Pentium II
0x11:    SSE
0x12:    SSE2
0x13:    SSE3
0x14:    Suppl. SSE3
0x15:    SSE4.1
0x16:    SSE4.2
0x17:    AES
0x18:    CLMUL
0x19:    AVX
0x1A:    FMA3
0x1C:    AVX2
0x1D:    BMI1, BMI2, ADX, RDRAND, RDSEED, INVPCID, SMAP, PRFCHW, F16C, Transactional Synchronization
0x20:    AVX512F,BW,DQ,VL
0x21:    AVX512PF,ER,CD
0x22:    SHA,TBD
0x23:    AVX512IFMA,VBMI
0x24:    AVX512_4FMAPS, ..

0x80:    MIC Knights Corner
0x100:   8087
0x101:   80387
0x800:   Privileged instruction
0x1001:  AMD 3DNow
0x1002:  AMD 3DNow extension
0x1004:  AMD SSE4a or AMD virtualization
0x1005:  AMD XOP
0x1006:  AMD FMA4
0x1007:  AMD TBM
0x2001;  VIA

0x4000:  Only available in 64 bit mode
0x8000:  Not  available in 64 bit mode
0x10000: Proposed instruction code, preliminary specification
0x20000: Proposed instruction code never implemented, preliminary specification later changed

AllowedPrefixes:
(Values can be OR'ed):
----------------------
0:       No prefix allowed other than possibly segment and address size prefixes if there is a mod/reg/rm byte
1:       Address size prefix allowed, even if no mod/reg/rm byte
2:       This is a stack operation. Address size prefix will truncate the stack pointer. Make warning if address size prefix or operand size prefix
4:       Segment prefix allowed, even if no mod/reg/rm byte
8:       Branch prediction hint prefix allowed (on Pentium 4) or BND prefix allowed
0x10:    LOCK prefix allowed
0x20:    REP prefix allowed
0x40:    REPE/REPNE prefix allowed
0x80:    This is a jump operation. 66 prefix will truncate EIP. Make warning if 66 prefix in 32 bit mode. 66 prefix not allowed in 64 bit mode.
0x100:   66 prefix determines integer operand size
0x200:   66 prefix allowed for other purpose. Typical meanings are:
         * indicates packed integer xmm vs. mmx,
         * indicates packed double precision xmm (pd) vs. packed single (ps)
         * always required
0x400:   F3 prefix allowed for other purpose. Typical = scalar single precision xmm (ss)
0x800:   F2 prefix allowed for other purpose. Typical = scalar double precision xmm (sd)
0xC40:   F2 and F3 prefix allowed for XACQUIRE and XRELEASE
0xE00:   none/66/F2/F3 prefix indicate ps/pd/sd/ss vector

0x1000:  REX.W prefix determines integer g.p. operand size or fp precision or swaps operands or other purpose
0x2000:  REX.W prefix allowed but unnecessary
0x3000:  REX.W prefix determines integer (vector) operand size d/q or ps/pd
0x4000:  VEX.W prefix determines integer (vector) operand size b/w
0x5000:  VEX.W and 66 prefix determines integer operand size b/w/d/q (mask instructions. B = 66W0, W = _W0, D = 66W1, Q = _W1)
0x7000:  REX.W prefix swaps last two operands (AMD)
0x8000:  Instruction not allowed without 66/F2/F3 prefix as specified by previous bits

0x10000: VEX or XOP prefix allowed
0x20000: VEX or EVEX or XOP prefix required
0x40000: VEX.L prefix allowed
0x80000: VEX.vvvv prefix allowed

0x100000:VEX.L prefix required
0x200000:VEX.L prefix allowed only if pp bits < 2
0x400000:MVEX prefix allowed
0x800000:EVEX prefix allowed

InstructionFormat:
(Values can be OR'ed):
----------------------
0:      Illegal opcode.
1:      No mod/reg/rm byte. Operands are implicit
2:      No mod/reg/rm byte. No operands (other than possibly immediate operand)
3:      No mod/reg/rm byte. Register operand indicated by bits 0-2
4:      Has VEX or EVEX prefix and no mod/reg/rm byte, Register operand, if any, indicated by VEX.v
0x10:   Has mod/reg/rm byte and possibly a SIB byte
0x11:   Has mod/reg/rm byte and one register/memory operand
0x12:   Has mod/reg/rm byte, a register destination operand and a register/memory source operand
0x13:   Has mod/reg/rm byte, a register/memory destination operand and a register source operand
0x14:   Has mod/reg/rm byte and AMD DREX byte. One destination and two source operands and possibly an immediate byte operand (AMD SSE5 instructions never implemened)
0x15:   Has mod/reg/rm byte and AMD DREX byte. One destination and three source operands. One of the source operands is equal to the destination operand (AMD SSE5 instructions never implemened)
0x18:   Has VEX or EVEX prefix and 2 operands. (NDD) Dest = VEX.v, src = rm, opcode extension in r bits. Src omitted if no VEX prefix.
0x19:   Has VEX or EVEX prefix and 3 operands. (NDS) Dest = r,  src1 = VEX.v, src2 = rm. Src1 omitted if no VEX prefix. May swap src1 and src2 if VEX.W = 0
0x1A:   Has VEX prefix and 3 operands. Dest = rm, src1 = VEX.v, src2 = r
0x1B:   Has VEX prefix and 3 operands. Dest = r,  src1 = rm, src2 = VEX.v.
0x1C:   Has VEX prefix and 4 operands. Dest = r,  src1 = VEX.v, src2 = rm, src3 = bits 4-7 of immediate byte. May swap src2 and src3 if VEX.W
0x1D:   Has VEX prefix and 4 operands. Dest = r,  src1 = bits 4-7 of immediate byte, src2 = rm, src3 = VEX.v. May swap src2 and src3 if VEX.W
0x1E:   Has VEX prefix VSIB and 2 or 3 operands. Dest = r or rm, src1 = rm or r, src2 = VEX.v or k register or none. VSIB byte required (rm operand & 0xF00 = index register size, rm operand & 0xFF = operand size)
0x20:   Has 2 bytes immediate operand (ret i) or 1 + 1 bytes (insrtq)
0x40:   Has 1 byte immediate operand or short jump
0x60:   Has 2 + 1 = 3 bytes immediate operand (enter)
0x80:   Has 2 or 4 bytes immediate operand or near jump
0x100:  Has a 2, 4 or 8 bytes immediate operand
0x200:  Has a 2+2 or 4+2 far direct jump operand
0x400:  Has a 2, 4 or 8 bytes direct memory operand
0x800:  Has a far indirect memory operand, dword, fword or tbyte
0x2000: Opcode reserved for future extensions
0x4000: Undocumented opcode or illegal (undocumented if name specified, otherwise illegal or unknown)
0x8000: This is a prefix, not an opcode
0x8001: This is a segment prefix

Destination and Source operand types,
used by SOpcodeDef::Destination, SOpcodeDef::Source, and CDisassembler::s.Operands[].
Many of the bit values can be OR'ed. If an instruction has two source operands, then
the values for these two operands are OR'ed (e.g. imul eax,ebx,9; shrd eax,ebx,cl).
-------------------------------------------------------------------------------------
0:      No explicit operand
1:      8  bit integer
2:      16 bit integer
3:      32 bit integer
4:      64 bit integer
5:      80 bit integer memory
6:      integer memory, other size
7:      48 bit memory
8:      16 or 32 bit integer, depending on 66 prefix
9:      16, 32 or 64 bit integer, depending on 66 or REX.W prefix. (8 bit in some cases as indicated by AllowedPrefixes)
0x0A:   16, 32 or 64 bit integer, default size = address size (REX.W not needed)
0x0B:   16, 32 or 64 bit near indirect pointer (jump)
0x0C:   16, 32 or 64 bit near indirect pointer (call)
0x0D:   16+16, 32+16 or 64+16 bits far indirect pointer (jump or call)

0x11:   8  bit constant, unsigned
0x12:   16 bit constant, unsigned
0x13:   32 bit constant, unsigned
0x18:   16 or 32 bit constant, unsigned
0x19:   16, 32 or 64 bit constant, unsigned
0x21:   8  bit constant, signed
0x22:   16 bit constant, signed
0x23:   32 bit constant, signed
0x28:   16 or 32 bit constant, signed
0x29:   16, 32 or 64 bit constant, signed
0x31:   8  bit constant, hexadecimal
0x32:   16 bit constant, hexadecimal
0x33:   32 bit constant, hexadecimal
0x34:   64 bit constant, hexadecimal
0x38:   16 or 32 bit constant, hexadecimal
0x39:   16, 32 or 64 bit constant, hexadecimal

0x40:   float x87, unknown size or register only
0x43:   32 bit float x87, single precision
0x44:   64 bit float x87, double precision
0x45:   80 bit float x87, long double precision
0x48:   float SSE, unknown size
0x4A:   16 bit float, half precision
0x4B:   32 bit float SSE,  single precision (ss) or packed (ps)
0x4C:   64 bit float SSE2, double precision (sd) or packed (pd)
0x4F:   XMM float. Size depends on prefix: none = ps, 66 = pd, F2 = sd, F3 = ss; or VEX.W bit = sd/pd
0x50:   Full vector, aligned
0x51:   Full vector, unaligned

0x81:   Short jump destination, 8 bits
0x82:   Near jump destination, 16 or 32 bits, depending on operand size
0x83:   Near call destination, 16 or 32 bits, depending on operand size
0x84:   Far jump destination, 16+16 or 32+16 bits, depending on operand size
0x85:   Far call destination, 16+16 or 32+16 bits, depending on operand size
0x91:   segment register
0x92:   control register
0x93:   debug register
0x94:   test register (obsolete or undocumented)
0x95:   k0 - k7 mask register. 16 bits if memory operand, 32-64 bits if register
0x96:   (reserved for future mask register > 16 bits)
0x98:   bnd0 - bnd3 bounds register

0xa1:   al
0xa2:   ax
0xa3:   eax
0xa4:   rax
0xa8:   ax or eax
0xa9:   ax, eax or rax
0xae:   xmm0
0xaf:   st(0)
0xb1:   1
0xb2:   dx
0xb3:   cl
0xc0:   [bx], [ebx] or [rbx]
0xc1:   [si], [esi] or [rsi]
0xc2:   es:[di], es:[edi] or [rdi]

// The following values can be added to specify vectors
0x100:  Vector MMX or XMM or YMM or ZMM, depending on 66 prefix and VEX.L prefix and EVEX.LL prefix
0x200:  Vector XMM, YMM or ZMM, depending on VEX.L prefix and EVEX.LL prefix
0x300:  Vector MMX (8  bytes)
0x400:  Vector XMM (16 bytes)
0x500:  Vector YMM (32 bytes)
0x600:  Vector ZMM (64 bytes)
0x700:  Future ??? (128 bytes)
0xF00:  Vector half the size defined by VEX.L prefix and EVEX.LL prefix. Minimum size = 8 bytes for memory, xmm for register

// The following values can be added to specify operand type
0x1000: Must be register, memory operand not allowed
0x2000: Must be memory, register operand not allowed

// The following bit values apply to CDisassembler::s.Operands[] only:
0x10000:    Direct memory operand without mod/reg/rm byte
0x20000:    Register operand indicated by last bits of opcode and B bit
0x30000:    Register or memory operand indicated by mod and rm bits of mod/reg/rm byte and B,X bits
0x40000:    Register operand indicated by reg bits of mod/reg/rm byte and R bit
0x50000:    Register operand indicated by dest bits of DREX byte
0x60000:    Register operand indicated by VEX.vvvv bits
0x70000:    Register operand indicated by bits 4-7 of immediate operand
0x80000:    (Register operand indicated by bits 0-3 of immediate operand. unused, reserved for future use)
0x100000:   Immediate operand using immediate field or first part of it
0x200000:   Immediate operand using second part of immediate field
0x1000000:  Is code
0x2000000:  Is supposed to be code, but dubious
0x4000000:  Is data

// The following bit values applies only to symbol types originating from object file
0x40000000: Gnu indirect function (CPU dispatcher)
0x80000000: Symbol is a segment (in COFF file symbol table)

EVEX:
--------
This field indicates the meaning of the z, L'L, b and aaa bits of an EVEX prefix. 
(The EVEX field may also be used in the future for indicating an extra operand
if it is not needed for its current purpose).

Bit 0-3 indicate meaning of L'L, b field:
  0x01  broadcast allowed for memory operand, LL indicate vector length
  0x02  SAE allowed for register operands, no rounding control, LL indicate vector length
  0x06  rounding control and SAE allowed for register operands  
  0x08  Scalar. LL ignored

Bit 4-7 indicate mask use in aaa/kkk field
  0x00  no masking. aaa must be zero
  0x10  allow masking, not zeroing
  0x20  allow masking  and zeroing
  0x50  allow masking, not zeroing. aaa must be nonzero
  0x80  mask is modified by instruction

Bit 12-15 indicate offset multiplier
  0x0000 Multiplier corresponds to memory operand size
  0x1000 Multiplier corresponds to vector element size
  0x2200 Multiplier corresponds to half the size of the largest vector operand
  0x2400 Multiplier corresponds to 1/4 of the size of the largest vector operand
  0x2600 Multiplier corresponds to 1/8 of the size of the largest vector operand


MVEX:
--------
This field indicates the meaning of the sss, e and kkk bits of an MVEX prefix. 
(The MVEX field may also be used in the future for indicating an extra operand
if it is not needed for its current purpose).
Bit 0-4 indicate meaning of sss field:
    0. none, sss must be 0
    1. sss ignored or used only for sae, offset multiplier defined, vector size defined
    2. sss ignored or used only for sae, offset multiplier defined, vector size not defined by sss
    3. reserved for future use
    4. Sf32. 32-bit float operand. permutation if register, broadcast or conversion if memory operand
    5. Sf64. 64-bit float operand. permutation if register, broadcast if memory operand
    6. Si32. 32-bit integer operand. permutation if register, broadcast or conversion if memory operand
    7. Si64. 64-bit integer operand. permutation if register, broadcast if memory operand
    8. Uf32. 32-bit float memory operand. Up conversion from smaller integer or float operand
    9. Uf64. 64-bit float memory operand. Currently no conversion supported
  0xA. Ui32. 32-bit integer memory operand. Up conversion from smaller integer operand
  0xB. Ui64. 64-bit integer memory operand. Currently no conversion supported
  0xC. Df32. 32-bit float memory operand. Down conversion to smaller integer or float operand
  0xD. Df64. 64-bit float memory operand. Currently no conversion supported
  0xE. Di32. 32-bit integer memory operand. Down conversion to smaller integer operand
  0xF. Di64. 64-bit integer memory operand. Currently no conversion supported
 0x10. Uf32, broadcast * 4, vbroadcastf32x4
 0x11. Uf64, broadcast * 4, vbroadcastf64x4
 0x12. Ui32, broadcast * 4, vbroadcasti32x4
 0x13. Ui64, broadcast * 4, vbroadcasti64x4
 0x14. Si32, half size, vcvtdq2pd, vcvtudq2pd
 0x15. Sf32, half size, vcvtps2pd
 0x16. Sf32, without register swizzle and limited broadcast, vfmadd233ps
Bit 6-7 indicate offset multiplier
  0x00  No broadcast. Multiplier corresponds to conversion
  0x40  Broadcast, gather and scatter instructions. Multiplier corresponds to element size before conversion
Bit 8-10 indicate alternative meaning of sss field for register operand when E bit is 1:
  0x000. E bit not allowed for register operand
  0x100. sss specifies rounding mode
  0x200. high s bit indicates suppress all exceptions {sae}
  0x300. sss specifies rounding mode and sae
  0x400. no rounding and no sae. sss bits ignored when E = 1
Bit 11  ignore E bit
  0x000. The E bit means cache eviction hint
  0x800. The E bit is ignored for memory operands or has a different meaning
Bit 12-13 indicate meaning of kkk field
  0x0000. kkk bits unused, must be 0
  0x1000. kkk bits specify register used for masked operation
  0x2000. kkk bits specify mask register as destination operand
  0x3000. kkk bits specify mask register used both for masked operation and as destination operand
The multiplier for single-byte address offsets is derived from the meaning of the sss field.

TableLink:
----------
Used for linking to another opcode table when more than one opcode begins 
with the same bytes or when different specifications are needed in different
cases. When TableLink is nonzero then InstructionSet is an index into 
OpcodeTables pointing to a subtable. The subtable is indexed according to
the criterion defined by TableLink.

0:      No link to other table
1:      Use following byte as index into next table (256 entries)
2:      Use reg field of mod/reg/rm byte as index into next table (8 entries)
3:      Use mod < 3 vs. mod == 3 as index (0: memory operand, 1: register operand)
4:      Use mod and reg fields of mod/reg/rm byte as index into next table,
        first 8 entries indexed by reg for mod < 3, next 8 entries indexed by reg for mod = 3.
5:      Use rm bits of mod/reg/rm byte as index into next table (8 entries)
6:      Use immediate byte after any operands as index into next table. Note: Instruction format must be specified
7:      Use mode as index into next table (0: 16 bits, 1: 32 bits, 2: 64 bits)
8:      Use operand size as index into next table (0: 16 bits, 1: 32 bits, 2: 64 bits)
9:      Use prefixes as index into next table (0: none, 1: 66, 2: F2, 3: F3)
0x0A:   Use address size as index into next table (0: 16 bits, 1: 32 bits, 2: 64 bits)
0x0B:   Use VEX prefix and VEX.L bits as index into next table (0: VEX absent, 1: VEX.L=0, 2: VEX.L=1, 3:MVEX or EVEX.LL=2, 4: EVEX.LL=3)
0x0C:   Use VEX.W bit as index into next table (0: VEX.W=0, 1: VEX.W=1)
0x0D:   Use vector size by VEX.L bits as index into next table (0: VEX.L=0, 1: VEX.L=1, 2:MVEX or EVEX.LL=2, 3: EVEX.LL=3)
0x0E:   Use VEX prefix type as index into next table. (0: 2- or 3-bytes VEX or none, 1: 4-bytes EVEX or MVEX)
0x0F:   Use MVEX.E bit as index into next table. (0: MVEX.E = 0 or no MVEX, 1: MVEX.E = 1)
0x10:   Use assembly language dialect as index into next table (0: MASM, 1: NASM/YASM, 2: GAS)
0x11:   Use VEX prefix type as index into next table. (0: none, 1: VEX prefix, 2: EVEX prefix, 3: MVEX prefix)

Options:
(Values can be OR'ed):
----------------------
1:      Append suffix for operand size or type to opcode name (prefix 0x100: b/w/d/q, 0xE00: ps/pd/ss/sd, 0x1000: s/d, 0x3000: d/q, 0x4000: b/w)
2:      Prepend 'v' to opcode name if VEX prefix present
4:      Does not change destination register
8:      Can change registers other than explicit destination register (includes call etc.)
0x10:   Unconditional jump. Next instruction will not be executed unless there is a jump to it.
0x20:   Code prefixes explicitly. Assembler cannot code prefixes on this instruction
0x40:   Instruction may be used as NOP or filler
0x80:   Shorter version of instruction exists for certain operand values
0x100:  Aligned. Memory operand must be aligned, even if VEX prefixed
0x200:  Unaligned. Unaligned memory operand always allowed.
0x400:  Opcode name differs if 64 bits
0x800:  Do not write size specifier on memory operand
0x1000: Append alternative suffix to opcode name (prefix 0x3000: "32"/"64")

*/

// Structure for opcode swizzle table entries indicating meaning of EVEX.sss bits
struct SwizSpec {
    uint32 memop;       // memory operand type
    uint32 memopsize;   // memory operand size = byte offset multiplier = required alignment
    uint32 elementsize; // memory operand size for broadcast, gather and scatter instructions
    const char * name;  // name of permutation, conversion or rounding
};


// Define data structures and classes used by class CDisassembler:

// Structure for properties of a single opcode during disassembly
struct SOpcodeProp {
   SOpcodeDef const * OpcodeDef;                 // Points to entry in opcode map
   uint8  Prefixes[8];                           // Stores the last prefix encountered in each category
   uint8  Conflicts[8];                          // Counts prefix conflicts as different prefixes in the same category
   uint32 Warnings1;                             // Warnings about conditions that could be intentional and suboptimal code
   uint32 Warnings2;                             // Warnings about possible misinterpretation
   uint32 Errors;                                // Errors that will prevent execution or are unlikely to be intentional
   uint32 AddressSize;                           // Address size: 16, 32 or 64
   uint32 OperandSize;                           // Operand size: 16, 32 or 64
   uint32 MaxNumOperands;                        // Number of opcode table operands to check
   uint32 Mod;                                   // mod bits of mod/reg/rm byte
   uint32 Reg;                                   // reg bits of mod/reg/rm byte
   uint32 RM;                                    // r/m bits of mod/reg/rm byte
   uint32 MFlags;                                // Memory operand type: 1=has memory operand, 2=has mod/reg/rm byte, 4=has SIB byte, 8=has VEX or DREX byte, 0x100=is rip-relative
   uint32 BaseReg;                               // Base  register + 1. (0 if none)
   uint32 IndexReg;                              // Index register + 1. (0 if none)
   uint32 Scale;                                 // Scale factor = 2^Scale
   uint32 Vreg;                                  // ~VEX.vvvv or AMD DREX byte
   uint32 Kreg;                                  // EVEX.aaa = MVEX.kkk mask register
   uint32 Esss;                                  // EVEX.zLLb = MVEX.Esss option bits
   SwizSpec const * SwizRecord;                  // Selected entry in MVEX table for MVEX code
   uint32 OffsetMultiplier;                      // Multiplier for 1-byte offset calculated from EVEX or obtained from MVEX.sss and table lookup
   uint32 Operands[5];                           // Operand types for destination, source, immediate
   uint32 OpcodeStart1;                          // Index to first opcode byte, after prefixes
   uint32 OpcodeStart2;                          // Index to last opcode byte, after 0F, 0F 38, etc., before mod/reg/rm byte and operands
   uint32 AddressField;                          // Beginning of address/displacement field
   uint32 AddressFieldSize;                      // Size of address/displacement field
   uint32 AddressRelocation;                     // Relocation pointing to address field
   uint32 ImmediateField;                        // Beginning of immediate operand or jump address field
   uint32 ImmediateFieldSize;                    // Size of immediate operand or jump address field
   uint32 ImmediateRelocation;                   // Relocation pointing to immediate operand or jump address field
   const char * OpComment;                       // Additional comment for opcode
   void   Reset() {                              // Set everything to zero
      memset(this, 0, sizeof(*this));}
};
// The meaning of each bit in s.Warnings and s.Errors is given in 
// AsmErrorTexts and AsmWarningTexts in the beginning of disasm.cpp

// Prefix categories used by s.Prefixes[category]
// 0: Segment prefix (26, 2E, 36, 3E, 64, 65)
// 1: Address size prefix (67)
// 2: Lock prefix (F0)
// 3: Repeat prefix (F2, F3) or VEX prefix (C4, C5) or EVEX, MVEX (62) or XOP (8F)
// 4: Operand size prefix (66, REX.W)
// 5: Operand type prefix (66, F2, F3)
// 6: VEX prefix: bit 5: VEX.L (vector length), bit 0-4: VEX.mmmmm
//    MVEX: bit 5 = 0, bit 6 = 1. EVEX: bit 5 = 1, bit 6 = 1
// 7: Rex prefix (40 - 4F), VEX.W,R,X,B, DREX.W,R,X,B
//    bit 0: B = extension of mod/rm or base or opcode
//    bit 1: X = extension of index register
//    bit 2: R = extension of reg bits
//    bit 3: W = 64 bit operand size, or swap operands or other use of VEX.W
//    bit 4: 2-bytes VEX prefix
//    bit 5: 3 or 4-bytes VEX prefix
//    bit 6: REX prefix
//    bit 7: XOP prefix or DREX byte (AMD only)
// Note that the 66 and REX.W prefixes belong to two categories. The interpretation
// is determined by AllowedPrefixes in SOpcodeDef

// Structure for tracing register values etc.
// See CDisassembler::UpdateTracer() in disasm.cpp for an explanation
struct SATracer {
   uint8  Regist[16];                            // Defines the type of information contained in each g.p. register
   uint32 Value[16];                             // Meaning depends on the value of Regist[i]
   void Reset() {                                // Set to zero
      *(uint64*)Regist = 0; *(uint64*)(Regist+8) = 0; 
   }
};

// Structure for defining section 
struct SASection {
   uint8 * Start;                                // Point to start of binary data
   uint32  SectionAddress;                       // Address of section (image relative)
   uint32  InitSize;                             // Size of initialized data in section
   uint32  TotalSize;                            // Size of initialized and uninitialized data in section
   uint32  Type;                                 // 0 = unknown, 1 = code, 
                                                 // 2 = data, 3 = uninitialized data only, 4 = constant data, 
                                                 // 0x10 = debug info, 0x11 = exception info. 
                                                 // 0x800 = segment group
                                                 // 0x1000 = communal section
   uint32  Align;                                // Alignment = 1 << Align
   uint32  WordSize;                             // Word size, 16, 32, 64
   uint32  Name;                                 // Name, as index into CDisassembler::NameBuffer
   int32   Group;                                // Group that the segment is member of. 0 = none, -2 = flat, > 0 = defined group
};

// Structure for defining relocation or cross-reference
struct SARelocation {
   int32   Section;                              // Section of relocation source
   uint32  Offset;                               // Offset of relocation source into section
   uint32  Type;                                 // Relocation types:
   // 0 = unknown, 1 = direct, 2 = self-relative, 4 = image-relative, 
   // 8 = segment relative, 0x10 = relative to arbitrary ref. point, 
   // 0x21 = direct, has already been relocated to image base (executable files only)
   // 0x41 = direct, make entry in procedure linkage table. Ignore addend (executable files only)
   // 0x81 = direct to Gnu indirect function PLT entry
   // 0x100 = segment address/descriptor, 0x200 = segment of symbol, 
   // 0x400 = segment:offset far
   // 0x1001 = reference to GOT entry relative to GOT. 0x1002 = self-relative reference to GOT or GOT-entry
   // 0x2002 = self-relative to PLT
   uint32  Size;                                 // 1 = byte, 2 = word, 4 = dword, 6 = fword, 8 = qword
   int32   Addend;                               // Addend to add to target address, 
                                                 // including distance from source to instruction pointer in self-relative addresses,
                                                 // not including inline addend.
   uint32  TargetOldIndex;                       // Old symbol table index of target
   uint32  RefOldIndex;                          // Old symbol table index of reference point if Type = 8, 0x10, 0x200
   int operator < (const SARelocation & y) const{// Operator for sorting relocation table by source address
      return Section < y.Section || (Section == y.Section && Offset < y.Offset);}
};

// Structure for indicating where a function begins and ends
struct SFunctionRecord {
   int32  Section;                               // Section containing function
   uint32 Start;                                 // Offset of function start
   uint32 End;                                   // Offset of function end
   uint32 Scope;                                 // Scope of function. 0 = inaccessible, 1 = function local, 2 = file local, 4 = public, 8 = weak public, 0x10 = communal, 0x20 = external
                                                 // 0x10000 means End not known, extend it when you pass End
   uint32 OldSymbolIndex;                        // Old symbol table index
   int operator < (const SFunctionRecord & y) const{// Operator for sorting function table by source address
      return Section < y.Section || (Section == y.Section && Start < y.Start);}
};

// Structure for defining symbol
struct SASymbol {
   int32   Section;                              // Section number. 0 = external, -1 = absolute symbol, -16 = section to be found from image-relative offset
   uint32  Offset;                               // Offset into section. (Value for absolute symbol)
   uint32  Size;                                 // Number of bytes used by symbol or function. 0 = unknown
   uint32  Type;                                 // Use values listed above for SOpcodeDef operands. 0 = unknown type
   uint32  Name;                                 // Name, as index into CDisassembler::SymbolNameBuffer. 0 = no name yet
   uint32  DLLName;                              // Name of DLL if symbol imported by dynamic linking
   uint32  Scope;                                // 0 = inaccessible, 1 = function local, 2 = file local, 4 = public, 8 = weak public, 0x10 = communal, 0x20 = external, 0x100 = has been written
   uint32  OldIndex;                             // Index in original symbol table. Used for tracking relocation entries
   void    Reset() {                             // Set everything to zero
      memset(this, 0, sizeof(*this));}
   int operator < (const SASymbol & y) const {   // Operator for sorting symbol table
      return Section < y.Section || (Section == y.Section && Offset < y.Offset);}
};

// Define class CSymbolTable
class CSymbolTable {
public:
   CSymbolTable();                               // Constructor
   uint32 AddSymbol(int32 Section, uint32 Offset,// Add a symbol from original file
      uint32 Size, uint32 Type, uint32 Scope, 
      uint32 OldIndex, const char * Name, const char * DLLName = 0);
   uint32 NewSymbol(int32 Section, uint32 Offset, uint32 Scope); // Add symbol to list
   uint32 NewSymbol(SASymbol & sym);             // Add symbol to list
   void AssignNames();                           // Assign names to symbols that do not have a name
   uint32 FindByAddress(int32 Section, uint32 Offset, uint32 * Last, uint32 * NextAfter = 0); // Find symbols by address
   uint32 FindByAddress(int32 Section, uint32 Offset); // Find symbols by address
   uint32 Old2NewIndex(uint32 OldIndex);         // Translate old symbol index to new index
   SASymbol & operator [](uint32 NewIndex) {     // Access symbol by new index
      return List[NewIndex];}
   const char * HasName(uint32 symo);            // Ask if symbol has a name, input = old index, output = name or 0
   const char * GetName(uint32 symi);            // Get symbol name by new index. (Assign a name if none)
   const char * GetNameO(uint32 symo);           // Get symbol name by old index. (Assign a name if none)
   const char * GetDLLName(uint32 symi);         // Get import DLL name
   void   AssignName(uint32 symi, const char *name); // Give symbol a specific name
   uint32 GetLimit() {return OldNum;}            // Get highest old symbol number + 1
   uint32 GetNumEntries() {return List.GetNumEntries();}// Get highest new symbol number + 1
protected:
   CSList<SASymbol> List;                        // List of symbols, sorted by address
   CMemoryBuffer    SymbolNameBuffer;            // String buffer for names of symbols
   CSList<uint32>   TranslateOldIndex;           // Table to translate old symbol index to new symbol index
   void UpdateIndex();                           // Update TranslateOldIndex
   uint32 OldNum;                                // = 1 + max OldIndex
   uint32 NewNum;                                // Number of entries in List
   uint32 UnnamedNum;                            // Number of unnamed symbols
public:
   const char * UnnamedSymbolsPrefix;            // Prefix for names of unnamed symbols
   const char * UnnamedSymFormat;                // Format string for giving names to unnamed symbols
   const char * ImportTablePrefix;               // Prefix for pointers in import table
};


// Define class CDisassembler

// Instructions for use:
// The calling program must first define the imagebase, if any, by calling
// Init. Define all sections by calls to AddSection.
// Then define all symbols and relocations or cross-references by calls to
// AddSymbol and AddRelocation.
// Then call Go().
// Go() and its subfunctions will sort Symbols and Relocations, add all
// nameless symbols to its symbol table and give them names, assign types
// to all symbols as good as possible from the available information, and 
// find where each function begins and ends. Then it will disassemble the 
// code and fill OutFile with the disassembly.

class CDisassembler {
public:
   CDisassembler();                              // Constructor. Initializes tables etc.
   void Go();                                    // Do the disassembly
   void Init(uint32 ExeType, int64 ImageBase);   // Define file type and imagebase if executable file
                                                 // ExeType: 0 = object, 1 = position independent shared object, 2 = executable file
                                                 // Set ExeType = 2 if addresses have been relocated to a nonzero image base and there is no base relocation table.
   void AddSection(                              // Define section to be disassembled
      uint8 * Buffer,                            // Buffer containing raw data
      uint32  InitSize,                          // Size of initialized data in section
      uint32  TotalSize,                         // Size of initialized and uninitialized data in section
      uint32  SectionAddress,                    // Start address of section (image relative)
      uint32  Type,                              // 0 = unknown, 1 = code, 2 = data, 3 = uninitialized data, 4 = constant data
      uint32  Align,                             // Alignment = 1 << Align
      uint32  WordSize,                          // Segment word size: 16, 32 or 64
      const char * Name,                         // Name of section
      uint32  NameLength = 0);                   // Length of name if not zero terminated
   uint32 AddSymbol(                             // Define symbol for disassembler
      int32   Section,                           // Section number (1-based). 0 = external, -1 = absolute, -16 = Offset contains image-relative address
      uint32  Offset,                            // Offset into section. (Value for absolute symbol)
      uint32  Size,                              // Number of bytes used by symbol or function. 0 = unknown
      uint32  Type,                              // Symbol type. Use values listed above for SOpcodeDef operands. 0 = unknown type
      uint32  Scope,                             // 1 = function local, 2 = file local, 4 = public, 8 = weak public, 0x10 = communal, 0x20 = external
      uint32  OldIndex,                          // Unique identifier used in relocation entries. Value must be > 0 and limited because an array is created with this as index. 
                                                 // A value will be assigned and returned if 0.
      const char * Name,                         // Name of symbol. Zero-terminated ASCII string. A name will be assigned if 0.
      const char * DLLName = 0);                 // Name of DLL if imported dynamically                    
   void AddRelocation(                           // Define relocation or cross-reference for disassembler
      int32   Section,                           // Section of relocation source:
                                                 // Sections (and groups) are numbered in the order they are defined, starting at 1
                                                 // 0 = none or external, -1 = absolute symbol
                                                 // -16 = Offset contains image-relative address
      uint32  Offset,                            // Offset of relocation source into section
      int32   Addend,                            // Addend to add to target address, 
                                                 // including distance from source to instruction pointer in self-relative addresses,
                                                 // not including inline addend.
      uint32  Type,                              // see above at SARelocation for definition of relocation types
      uint32  Size,                              // 1 = byte, 2 = word, 4 = dword, 8 = qword
      uint32  TargetIndex,                       // Symbol index of target
      uint32  ReferenceIndex = 0);               // Symbol index of reference point if Type 0x10, Segment index if Type = 8 or 0x200
   int32 AddSectionGroup(                        // Define section group (from OMF file)
      const char * Name,                         // Name of group
      int32 MemberSegment);                      // Group member. Repeat for multiple members. 0 if none.
   static void CountInstructions();              // Count total number of instructions defined in opcodes.cpp
   const char * CommentSeparator;                // "; " or "# " Start of comment string
   const char * HereOperator;                    // "$" or "." indicating current position
   CTextFileBuffer   OutFile;                    // Output file
protected:
   CSymbolTable Symbols;                         // Table of symbols
   CSList<SASection> Sections;                   // List of sections. First is 0
   CSList<SARelocation> Relocations;             // List of cross references. First is 0
   CMemoryBuffer NameBuffer;                     // String buffer for names of sections. First is 0.
   CSList<SFunctionRecord> FunctionList;         // List of functions 
   int64   ImageBase;                            // Image base for executable files
   uint32  ExeType;                              // File type: 0 = object, 1 = position independent shared object, 2 = executable
   uint32  RelocationsInSource;                  // Number of relocations in source file

   // Code parser: The following members are used for parsing 
   // an opcode and identifying its components
   uint8 * Buffer;                               // Point to start of binary data
   SOpcodeProp s;                                // Properties of current opcode
   SATracer t;                                   // Trace of register contents
   uint32  Pass;                                 // 1 = pass 1, 2-3 = pass 1 repeated, 0x10 = pass 2, 0x100 = repetition requested
   uint32  SectionEnd;                           // End of current section
   uint32  WordSize;                             // Segment word size: 16, 32, 64
   uint32  Section;                              // Current section/segment
   uint32  SectionAddress;                       // Address of beginning of this section
   uint32  SectionType;                          // 0 = unknown, 1 = code, 2 = data, 3 = uninitialized data, 4 = constant data
   uint32  CodeMode;                             // 1 if current position contains code, 2 if dubiuos, 4 if data
   uint32  IFunction;                            // Index into FunctionList
   uint32  FunctionEnd;                          // End address of current function (pass 2)
   uint32  LabelBegin;                           // Address of nearest preceding label
   uint32  LabelEnd;                             // Address of next label
   uint32  LabelInaccessible;                    // Address of inaccessible code
   uint32  IBegin;                               // Begin of current instruction
   uint32  IEnd;                                 // End of current instruction
   uint32  DataType;                             // Type of current data
   uint32  DataSize;                             // Size of current data
   uint32  FlagPrevious;                         // 1: previous instruction was a NOP. 
                                                 // 2: previous instruction was unconditional jump. 6: instruction was ud2
                                                 // 0x100: previous data aligned by 16
                                                 // 0x200: previous data aligned by 32
   uint8   InstructionSetMax;                    // Highest instruction set encountered
   uint8   InstructionSetAMDMAX;                 // Highest AMD-specific instruction set encountered
   uint16  InstructionSetOR;                     // Bitwise OR of all instruction sets encountered
   uint16  Opcodei;                              // Map number and index in opcodes.cpp
   uint16  OpcodeOptions;                        // Option flags for opcode
   uint16  PreviousOpcodei;                      // Opcode for previous instruction
   uint16  PreviousOpcodeOptions;                // Option flags for previous instruction
   uint32  CountErrors;                          // Number of errors since last label
   uint32  Syntax;                               // Assembly syntax dialect: 1: MASM/TASM, 2: NASM/YASM, 4: GAS
   uint32  MasmOptions;                          // Options needed for MASM: 1: dotname, 2: fs used, 4: gs used
                                                 // 0x100: 16 bit segments, 0x200: 32 bit segments, 0x400: 64 bit segments
   uint32  NamesChanged;                         // Symbol names containing invalid characters changed
   int32   Assumes[6];                           // Assumed value of segment register es, cs, ss, ds, fs, gs. See CDisassembler::WriteSectionName for values
   void    Pass1();                              // Pass 1: Find symbols types and unnamed symbols
   void    Pass2();                              // Pass 2: Write output file
   int     NextFunction2();                      // Loop through function blocks in pass 2. Return 0 if finished
   int     NextLabel();                          // Loop through labels. (Pass 2)
   int     NextInstruction1();                   // Go to next instruction. Return 0 if none. (Pass 1)
   int     NextInstruction2();                   // Go to next instruction. Return 0 if none. (Pass 2)
   void    ParseInstruction();                   // Parse one opcode
   void    ScanPrefixes();                       // Scan prefixes
   void    StorePrefix(uint32 Category, uint8 Byte);// Store prefix according to category
   void    FindMapEntry();                       // Find entry in opcode maps
   void    FindOperands();                       // Interpret mod/reg/rm and SIB bytes and find operand fields
   void    FindOperandTypes();                   // Determine the types of each operand
   void    FindBroadcast();                      // Find broadcast and offset multiplier for EVEX code
   void    SwizTableLookup();                    // Find swizzle table entry for MVEX code
   void    FindLabels();                         // Find any labels at current position and next
   void    CheckForMisplacedLabel();             // Remove any label placed inside function
   void    FindRelocations();                    // Find any relocation sources in this instruction
   void    FindWarnings();                       // Find any reasons for warnings in code
   void    FindErrors();                         // Find any errors in code
   void    FindInstructionSet();                 // Update instruction set
   void    CheckForNops();                       // Check if warnings are caused by multi-byte NOP
   void    UpdateSymbols();                      // Find unnamed symbols, determine symbol types, update symbol list, call CheckJumpTarget if jump/call
   void    UpdateTracer();                       // Trace register values
   void    MarkCodeAsDubious();                  // Remember that this may be data in a code segment
   void    CheckRelocationTarget(uint32 IRel, uint32 TargetType, uint32 TargetSize);// Update relocation record and its target
   void    CheckJumpTarget(uint32 symi);         // Extend range of current function to jump target, if needed
   void    FollowJumpTable(uint32 symi, uint32 RelType);// Check jump/call table and its targets
   uint32  MakeMissingRelocation(int32 Section, uint32 Offset, uint32 RelType, uint32 TargetType, uint32 TargetScope, uint32 SourceSize = 0, uint32 RefPoint = 0); // Make a relocation and its target symbol from inline address
   void    CheckImportSymbol(uint32 symi);       // Check for indirect jump to import table entry
   void    CheckForFunctionBegin();              // Check if function begins at current position
   void    CheckForFunctionEnd();                // Check if function ends at current position
   void    CheckLabel();                         // Check if a label is needed before instruction
   void    InitialErrorCheck();                  // Check for illegal relocations table entries
   void    FinalErrorCheck();                    // Check for illegal entries in symbol table and relocations table
   void    CheckNamesValid();                    // Fix invalid characters in symbol and section names
   void    FixRelocationTargetAddresses();       // Find missing relocation target addresses
   int     TranslateAbsAddress(int64 Addr, int32 &Sect, uint32 &Offset); // Translate absolute virtual address to section and offset
   void    WriteFileBegin();                     // Write begin of file
   void    WriteFileBeginMASM();                 // Write MASM-specific file init
   void    WriteFileBeginYASM();                 // Write YASM-specific file init
   void    WriteFileBeginGASM();                 // Write  GAS-specific file init
   void    WriteFileEnd();                       // Write end of file
   void    WriteSegmentBegin();                  // Write begin of segment
   void    WriteSegmentBeginMASM();              // Write begin of segment, MASM syntax
   void    WriteSegmentBeginYASM();              // Write begin of segment, YASM syntax
   void    WriteSegmentBeginGASM();              // Write begin of segment, GAS  syntax
   void    WriteSegmentEnd();                    // Write end of segment
   void    WritePublicsAndExternalsMASM();       // Write public and external symbol definitions, MASM syntax
   void    WritePublicsAndExternalsYASMGASM();   // Write public and external symbol definitions, YASM and GAS syntax
   void    WriteFunctionBegin();                 // Write begin of function
   void    WriteFunctionBeginMASM(uint32 symi, uint32 scope);// Write begin of function, MASM syntax
   void    WriteFunctionBeginYASM(uint32 symi, uint32 scope);// Write begin of function, YASM syntax
   void    WriteFunctionBeginGASM(uint32 symi, uint32 scope);// Write begin of function, GAS  syntax
   void    WriteFunctionEnd();                   // Write end of function
   void    WriteFunctionEndMASM(uint32 symi);    // Write end of function, MASM syntax
   void    WriteFunctionEndYASM(uint32 symi);    // Write end of function, YASM syntax
   void    WriteFunctionEndGASM(uint32 symi);    // Write end of function, GAS  syntax
   void    WriteCodeLabel(uint32 symi);          // Write private or public code label
   void    WriteCodeLabelMASM(uint32 symi, uint32 scope);// Write private or public code label, MASM syntax
   void    WriteCodeLabelYASM(uint32 symi, uint32 scope);// Write private or public code label, MASM syntax
   void    WriteCodeLabelGASM(uint32 symi, uint32 scope);// Write private or public code label, MASM syntax
   int     WriteFillers();                       // Check if code is a series of NOPs or other fillers. If so then write it as such
   void    WriteAlign(uint32 a);                 // Write alignment directive
   void    WriteErrorsAndWarnings();             // Write errors and warnings, if any
   void    WriteAssume();                        // Write assume directive for segment register
   void    WriteInstruction();                   // Write instruction and operands
   void    WriteCodeComment();                   // Write hex listing of instruction as comment after instruction
   void    WriteStringInstruction();             // Write string instruction or xlat instruction
   void    WriteShortRegOperand(uint32 Type);    // Write register operand from lower 3 bits of opcode byte to OutFile
   void    WriteRegOperand(uint32 Type);         // Write register operand from reg bits to OutFile
   void    WriteRMOperand(uint32 Type);          // Write memory or register operand from mod/rm bits of mod/reg/rm byte and possibly SIB byte to OutFile
   void    WriteDREXOperand(uint32 Type);        // Write register operand from dest bits of DREX byte
   void    WriteVEXOperand(uint32 Type, int i);  // Write register operand from VEX.vvvv bits or immediate bits
   void    WriteOperandAttributeEVEX(int i, int isMem);// Write operand attributes and instruction attributes from EVEX z, LL, b and aaa bits
   void    WriteOperandAttributeMVEX(int i, int isMem);// Write operand attributes and instruction attributes from MVEX sss, e and kkk bits
   void    WriteImmediateOperand(uint32 Type);   // Write immediate operand or direct jump/call address
   void    WriteOtherOperand(uint32 Type);       // Write other type of operand
   void    WriteRegisterName(uint32 Value, uint32 Type); // Write name of register to OutFile
   void    WriteSectionName(int32 SegIndex);     // Write section name from section index
   void    WriteSymbolName(uint32 symi);         // Write symbol name
   void    WriteRelocationTarget(uint32 irel, uint32 Context, int64 Addend);// Write cross reference
   void    WriteOperandType(uint32 type);        // Write type override before operand, e.g. "dword ptr"
   void    WriteOperandTypeMASM(uint32 type);    // Write type override before operand, e.g. "dword ptr", MASM syntax
   void    WriteOperandTypeYASM(uint32 type);    // Write type override before operand, e.g. "dword", YASM syntax
   void    WriteOperandTypeGASM(uint32 type);    // Write type override before operand, e.g. "dword ptr", GAS syntax
   void    WriteDataItems();                     // Write data items
   void    WriteDataLabelMASM(const char * name, uint32 sym, int line); // Write label before data item, MASM syntax
   void    WriteDataLabelYASM(const char * name, uint32 sym, int line); // Write label before data item, YASM syntax
   void    WriteDataLabelGASM(const char * name, uint32 sym, int line); // Write label before data item, GAS  syntax
   void    WriteUninitDataItemsMASM(uint32 size, uint32 count);// Write uninitialized (BSS) data, MASM syntax
   void    WriteUninitDataItemsYASM(uint32 size, uint32 count);// Write uninitialized (BSS) data, YASM syntax
   void    WriteUninitDataItemsGASM(uint32 size, uint32 count);// Write uninitialized (BSS) data, GAS  syntax
   void    WriteDataDirectiveMASM(uint32 size);  // Write DB, etc., MASM syntax
   void    WriteDataDirectiveYASM(uint32 size);  // Write DB, etc., MASM syntax
   void    WriteDataDirectiveGASM(uint32 size);  // Write DB, etc., MASM syntax
   void    WriteDataComment(uint32 ElementSize, uint32 LinePos, uint32 Pos, uint32 irel);// Write comment after data item
   uint32  GetDataItemSize(uint32 Type);         // Get size of data item with specified type
   uint32  GetDataElementSize(uint32 Type);      // Get size of vector element in data item with specified type
   int32   GetSegmentRegisterFromPrefix();       // Translate segment prefix to segment register

   template <class TX> TX & Get(uint32 Offset) { // Get object of arbitrary type from buffer
      return *(TX*)(Buffer + Offset);}
};


// Declare tables in opcodes.cpp:
extern SOpcodeDef OpcodeMap0[256];               // First opcode map

extern uint32 OpcodeStartPageVEX[];              // Entries to opcode maps, indexed by VEX.mmmm bits
extern SOpcodeDef const * OpcodeStartPageXOP[];  // Entries to opcode maps, indexed by XOP.mmmm bits

extern const uint32 NumOpcodeStartPageVEX;       // Number of entries in OpcodeStartPage
extern const uint32 NumOpcodeStartPageXOP;       // Number of entries in OpcodeStartPageXOP

extern const SOpcodeDef * const OpcodeTables[];  // Pointers to all opcode tables

extern const uint32 OpcodeTableLength[];         // Size of each table pointed to by OpcodeTables[]

extern const uint32 NumOpcodeTables1, NumOpcodeTables2;// Number of entries in OpcodeTables[] and OpcodeTableLength[]

extern const char * RegisterNames8[8];           // Names of 8 bit registers
extern const char * RegisterNames8x[16];         // Names of 8 bit registers with REX prefix
extern const char * RegisterNames16[16];         // Names of 16 bit registers
extern const char * RegisterNames32[16];         // Names of 32 bit registers
extern const char * RegisterNames64[16];         // Names of 64 bit registers
extern const char * RegisterNamesSeg[8];         // Names of segment registers
extern const char * RegisterNamesCR[16];         // Names of control registers

extern SwizSpec const * SwizTables[][2];         // Pointers to swizzle tables
extern SwizSpec const * SwizRoundTables[][2];    // Pointers to swizzle round tables
extern const char * EVEXRoundingNames[5];        // Tables of rounding mode names for EVEX


// Define constants for special section/segment/group values
#define ASM_SEGMENT_UNKNOWN    0   // Unknown segment for external symbols
#define ASM_SEGMENT_ABSOLUTE  -1   // No segment for absolute public symbols
#define ASM_SEGMENT_FLAT      -2   // Flat segment group for non-segmented code
#define ASM_SEGMENT_NOTHING   -3   // Segment register assumed to nothing by assume directive
#define ASM_SEGMENT_ERROR     -4   // Segment register assumed to error (don't use) by assume directive
#define ASM_SEGMENT_IMGREL   -16   // Offset is relative to image base or file base,
                                   // ..leave it to the disassembler to find which section contains this address.
// Values > 0 are indices into the Sections buffer representing a named section, segment or group

#endif // #ifndef DISASM_H
