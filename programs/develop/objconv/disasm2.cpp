/****************************  disasm2.cpp   ********************************
* Author:        Agner Fog
* Date created:  2007-02-25
* Last modified: 2016-11-27
* Project:       objconv
* Module:        disasm2.cpp
* Description:
* Module for disassembler containing file output functions
*
* Changes that relate to assembly language syntax should be done in this file only.
*
* Copyright 2007-2016 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"

/**********************  Warning and error texts   ***************************
These texts are inserted in disassembled code in case of warnings or errors.

The occurrence of an error makes the disassembler mark the code block between
the nearest known code labels as dubious. This means that the byte sequence
might be data in the code segment or the disassembler might be out of phase
with instruction boundaries. Dubious code will be shown both as code and as
data.

A warning will be shown as 'Note:' before the instruction it applies to.
This might indicate suboptimal coding or a possible cause for concern.

The criteria for distinguishing between warnings and errors is not the 
severity of consequences, but whether the condition is likely to be caused
by common programming errors or by data in the code segment.

Some of the warning messages are quite benign, e.g. an unnecessary prefix.
Other warning messages can have severe consequences, e.g. a function missing
a return statement.

Still other warnings are no case for concern, but a condition requiring 
attention. For example the message: "Multi-byte NOP. Replace with ALIGN",
might actually indicate a well optimized code. But it requires attention
because the assembler cannot re-create the multi-byte NOP if the code
is assembled again. The programmer needs to decide what level of alignment
is optimal and replace the NOP with an align statement.

*****************************************************************************/

// Define error texts. 
SIntTxt AsmErrorTexts[] = {
    {1,         "Instruction longer than 15 bytes"},
    {2,         "Lock prefix not allowed for this opcode"},
    {4,         "Illegal opcode"},
    {8,         "Illegal operands for this opcode"},
    {0x10,      "Instruction extends beyond end of code block"},
    {0x20,      "Prefix after REX prefix not allowed"},
    {0x40,      "This instruction is not allowed in 64 bit mode"},
    {0x80,      "Instruction out of phase with next label"},
    {0x100,     "Attempt to use R13 as base register without displacement"},
    {0x200,     "Register 8 - 15 only allowed in 64 bit mode (Ignored)."},
    {0x400,     "REX prefix not allowed on instruction with DREX byte"},
    {0x800,     "VEX has X bit but no SIB byte (Probably ignored)"},
    {0x1000,    "Relocation source does not match address or operand field"},
    {0x2000,    "Overlapping relocations"},
    {0x4000,    "This is unlikely to be code"}, // Consecutive bytes of 0 found
    {0x8000,    "VEX.L bit not allowed here"},
    {0x10000,   "VEX.mmmm bits out of range"},
    {0x80000,   "Internal error in opcode table in opcodes.cpp"}
};

// Warning texts 1: Warnings about conditions that could be intentional and suboptimal code
SIntTxt AsmWarningTexts1[] = {
    {1,          "Immediate operand could be made smaller by sign extension"},
    {2,          "Immediate operand could be made smaller by zero extension"},
    {4,          "Zero displacement could be omitted"},
    {8,          "Displacement could be made smaller by sign extension"},
    {0x10,       "SIB byte unnecessary here"},
    {0x20,       "A shorter instruction exists for register operand"},
    {0x40,       "Length-changing prefix causes delay on Intel processors"},
    {0x80,       "Address size prefix should be avoided"},
    {0x100,      "Same prefix occurs more than once"},
    {0x200,      "Prefix valid but unnecessary"},
    {0x400,      "Prefix bit or byte has no meaning in this context"},
    {0x800,      "Contradicting prefixes"},
    {0x1000,     "Required prefix missing"},
    {0x2000,     "Address has scale factor but no index register"},
    {0x4000,     "Address is not rip-relative"},
    {0x8000,     "Absolute memory address without relocation"},
    {0x10000,    "Unusual relocation type for this operand"},
    {0x20000,    "Instruction pointer truncated by operand size prefix"},
    {0x40000,    "Stack pointer truncated by address size prefix"},
    {0x80000,    "Jump or call to data segment not allowed"},
    {0x100000,   "Undocumented opcode"},
    {0x200000,   "Unknown opcode reserved for future extensions"},
    {0x400000,   "Memory operand is misaligned. Performance penalty"},
    {0x800000,   "Alignment fault. Memory operand must be aligned"},
    {0x1000000,  "Multi-byte NOP. Replace with ALIGN"},
    {0x2000000,  "Bogus length-changing prefix causes delay on Intel processors here"},
    {0x4000000,  "Non-default size for stack operation"},
    {0x8000000,  "Function does not end with ret or jmp"},
    {0x10000000, "No jump seems to point here"},
    {0x20000000, "Full 64-bit address"},
    {0x40000000, "VEX prefix bits not allowed here"}
};

// Warning texts 2: Warnings about possible misinterpretation; serious warnings
SIntTxt AsmWarningTexts2[] = {
    {1,          "Label out of phase with instruction. Possibly spurious"},
    {2,          "Planned future instruction, according to preliminary specification"},
    {4,          "This instruction has been planned but never implemented because plans were changed. Will not work"},
    {0x10,       "EVEX prefix not allowed for this instruction"},
    {0x20,       "MVEX prefix not allowed for this instruction"},
    {0x40,       "EVEX prefix option bits not allowed here"},
    {0x80,       "MVEX prefix option bits not allowed here"},
    {0x100,      "Mask register must be nonzero"},
    {0x200,      "Broadcasting to scalar not allowd"},
};


// Indication of relocation types in comments:
SIntTxt RelocationTypeNames[] = {
    {0x001,  "(d)" },                   // Direct address in flat address space
    {0x002,  "(rel)" },                 // Self-relative
    {0x004,  "(imgrel)" },              // Image-relative
    {0x008,  "(segrel)" },              // Segment-relative
    {0x010,  "(refpoint)" },            // Relative to arbitrary point (position-independent code in Mach-O)
    {0x021,  "(d)" },                   // Direct (adjust by image base)
    {0x041,  "(d)" },                   // Direct (make procecure linkage table entry)
    {0x081,  "(indirect)" },            // Gnu indirect function dispatcher (make procecure linkage table entry?)
    {0x100,  "(seg)" },                 // Segment address or descriptor
    {0x200,  "(sseg)" },                // Segment of symbol
    {0x400,  "(far)" },                 // Far segment:offset address
    {0x1001, "(GOT)" },                 // GOT entry
    {0x1002, "(GOT r)" },               // self-relative to GOT entry
    {0x2002, "(PLT r)" }                // self-relative to PLT entry
};

// Instruction set names
const char * InstructionSetNames[] = {
    "8086", "80186", "80286", "80386",               // 0 - 3
    "80486", "Pentium", "Pentium Pro", "MMX",        // 4 - 7
    "Pentium II", "", "", "",                        // 8 - B
    "", "", "", "",                                  // C - F
    "", "SSE", "SSE2", "SSE3",                       // 10 - 13
    "Supplementary SSE3", "SSE4.1", "SSE4.2", "AES", // 14 - 17
    "CLMUL", "AVX", "FMA3", "?",                     // 18 - 1B
    "AVX2", "BMI etc.", "?", "?",                    // 1C - 1F
    "AVX-512", "AVX512PF/ER/CD", "MPX,SHA,TBD", "AVX512IFMA/VBMI", // 20 - 23
    "AVX512_4FMAPS", "?", "?", "?",                                // 24 - 27
    "?", "?", "?", "?",                              // 28 - 2B
    "?", "?", "?", "?",                              // 2C - 2F
    "?", "?", "?", "?",                              // 30 - 33
    "?", "?", "?", "?",                              // 34 - 37
    "?", "?", "?", "?",                              // 38 - 3B
    "?", "?", "?", "?",                              // 3C - 3F
    "?", "?", "?", "?",                              // 40 - 43
    "?", "?", "?", "?",                              // 44 - 47
    "?", "?", "?", "?",                              // 48 - 4B
    "?", "?", "?", "?",                              // 4C - 4F
    "?", "?", "?", "?",                              // 50 - 53
    "?", "?", "?", "?",                              // 54 - 57
    "?", "?", "?", "?",                              // 58 - 5B
    "?", "?", "?", "?",                              // 5C - 5F
    "?", "?", "?", "?",                              // 60 - 63
    "?", "?", "?", "?",                              // 64 - 67
    "?", "?", "?", "?",                              // 68 - 6B
    "?", "?", "?", "?",                              // 6C - 6F
    "?", "?", "?", "?",                              // 70 - 73
    "?", "?", "?", "?",                              // 74 - 77
    "?", "?", "?", "?",                              // 78 - 7B
    "?", "?", "?", "?",                              // 7C - 7F
    "Knights Corner", "?", "?", "?",                 // 80 - 83
    "?", "?", "?", "?"                               // 84 - 87
};

const int InstructionSetNamesLen = TableSize(InstructionSetNames);


/**************************  class CDisassembler  *****************************
Most member functions of CDisassembler are defined in disasm1.cpp

Only the functions that produce output are defined here:
******************************************************************************/

void CDisassembler::WriteShortRegOperand(uint32 Type) {
    // Write register operand from lower 3 bits of opcode byte to OutFile
    uint32 rnum = Get<uint8>(s.OpcodeStart2) & 7;
    // Check REX.B prefix
    if (s.Prefixes[7] & 1) rnum |= 8;             // Add 8 if REX.B prefix
    // Write register name
    WriteRegisterName(rnum, Type);
}

void CDisassembler::WriteRegOperand(uint32 Type) {
    // Write register operand from reg bits
    uint32 Num = s.Reg;                           // Register number

    // Write register name
    WriteRegisterName(Num, Type);
}

void CDisassembler::WriteRMOperand(uint32 Type) {
    // Write memory or register operand from mod/rm bits of mod/reg/rm byte 
    // and possibly SIB byte or direct memory operand to OutFile.
    // Also used for writing direct memory operand

    if ((Type & 0xFF) == 0) {
        // No explicit operand
        return;
    }

    uint32 Components = 0;                        // Count number of addends inside []
    int64  Addend = 0;                            // Inline displacement or addend
    int AddressingMode = 0;                       // 0: 16- or 32 bit addressing mode
    // 1: 64-bit pointer
    // 2: 32-bit absolute in 64-bit mode
    // 4: 64-bit rip-relative
    // 8: 64-bit absolute
    // Check if register or memory
    if (s.Mod == 3) {
        // Register operand
        WriteRegisterName(s.RM, Type);
        return;
    }

    // Find addend, if any
    switch (s.AddressFieldSize) {
    case 1:  // 1 byte displacement
        Addend = Get<int8>(s.AddressField);
        break;
    case 2:  // 2 bytes displacement
        Addend = Get<int16>(s.AddressField);
        break;
    case 4:  // 4 bytes displacement
        Addend = Get<int32>(s.AddressField);
        if ((s.MFlags & 0x100) && !s.AddressRelocation) {
            // rip-relative
            Addend += ImageBase + uint64(SectionAddress + IEnd);
        }
        break;
    case 8:  // 8 bytes address
        Addend = Get<int64>(s.AddressField);
        break;
    }
    // Get AddressingMode
    if (s.AddressSize > 32) {
        if (s.MFlags & 0x100) {
            AddressingMode = 4;                     // 64-bit rip-relative
        }
        else if (s.AddressFieldSize == 8) {
            AddressingMode = 8;                     // 64-bit absolute
        }
        else if (s.AddressRelocation || (s.BaseReg==0 && s.IndexReg==0)) {
            AddressingMode = 2;                     // 32-bit absolute in 64-bit mode
        }
        else {
            AddressingMode = 1;                     // 64-bit pointer
        }
    }

    // Make exception for LEA with no type
    if (Opcodei == 0x8D) {
        Type = 0;
    }
    // Write type override
    if ((s.OpcodeDef->InstructionFormat & 0x1F) == 0x1E) {    
        WriteOperandType(Type & 0xFF);    // has vsib address: write element type rather than vector type
    }
    else if (!(s.OpcodeDef->Options & 0x800)) {
        WriteOperandType(Type);           // write operand type
    }

    if (Syntax != SUBTYPE_MASM) {
        // Write "[" around memory operands, before segment
        OutFile.Put("[");
    }

    // Write segment prefix, if any
    if (s.Prefixes[0]) {
        OutFile.Put(RegisterNamesSeg[GetSegmentRegisterFromPrefix()]);
        OutFile.Put(":");
    }
    else if (!s.BaseReg && !s.IndexReg && (!s.AddressRelocation || (s.Warnings1 & 0x10000)) && Syntax != SUBTYPE_YASM) { 
        // No pointer register and no memory reference or wrong type of memory reference.
        // Write segment register to indicate that we have a memory operand
        OutFile.Put("DS:");
    }

    if (Syntax == SUBTYPE_MASM) {
        // Write "[" around memory operands, after segment
        OutFile.Put("[");
    }

    if (Syntax == SUBTYPE_YASM && (AddressingMode & 0x0E)) {
        // Specify absolute or relative addressing mode
        switch (AddressingMode) {
        case 2: OutFile.Put("abs ");  break;
        case 4: OutFile.Put("rel ");  break;
        case 8: OutFile.Put("abs qword ");  break;
        }
    }

    // Write relocation target, if any
    if (s.AddressRelocation) {
        // Write cross reference
        WriteRelocationTarget(s.AddressRelocation, 4 | (s.MFlags & 0x100), Addend);
        // Addend has been written, don't write it again
        Addend = 0;
        // Remember that something has been written
        Components++;
    }

    // Check address size for pointer registers
    //const char * * PointerRegisterNames;
    uint32 RegisterType = 0;
    switch (s.AddressSize) {
    case 16:
        RegisterType = 2;  break;
    case 32:
        RegisterType = 3;  break;
    case 64:
        RegisterType = 4;  break;
    }

    // Write base register, if any
    if (s.BaseReg) {
        if (Components++) OutFile.Put("+");      // Put "+" if anything before
        WriteRegisterName(s.BaseReg - 1, RegisterType); 
    }

    // Write index register, if any
    if (s.IndexReg) {
        if (Components++) OutFile.Put("+");        // Put "+" if anything before
        if ((s.OpcodeDef->InstructionFormat & 0x1F) != 0x1E) {
            // normal index register
            WriteRegisterName(s.IndexReg - 1, RegisterType); 
        }
        else {
            // VSIB byte specifies vector index register
            WriteRegisterName(s.IndexReg - 1, Type & 0xF00); 
        }
        // Write scale factor, if any
        if (s.Scale) {
            OutFile.Put("*");
            OutFile.PutDecimal(1 << s.Scale);
        }
    }

    // Write +/- before addend
    if (Components && Addend) {
        // Displacement comes after base/index registers
        if (Addend >= 0 || s.AddressFieldSize == 8) {
            // Positive. Write +
            OutFile.Put("+");
        }
        else {
            // Negative. Write -
            OutFile.Put("-");
            Addend = -Addend;
        }
    }

    if (Addend || Components == 0) {
        // Find minimum number of digits needed
        uint32 AddendSize = s.AddressFieldSize;
        if ((uint64)Addend < 0x100 && AddendSize > 1) AddendSize = 1;
        else if ((uint64)Addend < 0x10000 && AddendSize > 2) AddendSize = 2;

        // Write address or addend as hexadecimal
        OutFile.PutHex((uint64)Addend, 2);

        // Check if offset multiplier needed
        if (s.OffsetMultiplier && s.AddressFieldSize == 1 && Addend) {
            OutFile.Put("*");
            OutFile.PutHex(s.OffsetMultiplier, 2);
        }
    }

    if (Syntax == SUBTYPE_GASM && (AddressingMode == 4)) {
        // Need to specify rip-relative address
        OutFile.Put("+rip");
    }

    // End with "]"
    OutFile.Put("]");
}


void CDisassembler::WriteOperandType(uint32 type) {
    switch (Syntax) {
    case SUBTYPE_MASM:
        WriteOperandTypeMASM(type);  break;
    case SUBTYPE_YASM:
        WriteOperandTypeYASM(type);  break;
    case SUBTYPE_GASM:
        WriteOperandTypeGASM(type);  break;
    }
}

void CDisassembler::WriteOperandTypeMASM(uint32 type) {
    // Write type override before operand, e.g. "dword ", MASM syntax
    if (type & 0xF00) {
        type &= 0xF00;                             // Ignore element type for vectors
    }
    else {
        type &= 0xFF;                              // Use operand type only
    }

    switch (type) {
    case 1:  // 8 bits
        OutFile.Put("byte ");  break;
    case 2:  // 16 bits
        OutFile.Put("word ");  break;
    case 3:  // 32 bits
        OutFile.Put("dword ");  break;
    case 4:  // 64 bits
        OutFile.Put("qword ");  break;
    case 5:  // 80 bits
        if ((s.OpcodeDef->Destination & 0xFF) == 0xD) {
            // 64+16 bit far pointer. Not supported by MASM
            OutFile.Put("fword ");
            s.OpComment = "64+16 bit. Need REX.W prefix";
        }
        else { 
            OutFile.Put("tbyte ");}
        break;
    case 6: case 0x40: case 0x48: case 0:
        // Other size. Write nothing
        break;
    case 7: case 0x0D: // 48 bits or far
        OutFile.Put("fword ");  
        if ((s.OpcodeDef->Destination & 0xFF) == 0xD && WordSize == 64) {
            // All assemblers I have tried forget the REX.W prefix here. Make a notice
            s.OpComment = "32+16 bit. Possibly forgot REX.W prefix"; 
        }      
        break;
    case 0x4A: // 16 bits float
        OutFile.Put("word ");  break;
    case 0x43: // 32 bits float (x87)
    case 0x4B: // 32 bits float (SSE2)
        OutFile.Put("dword ");  break;
    case 0x44: // 64 bits float
    case 0x4C: // 64 bits float (SSE2)
        OutFile.Put("qword ");  break;
    case 0x45: // 80 bits float
        OutFile.Put("tbyte ");  break;
    case 0x84: case 0x85: // far call
        OutFile.Put("far ");  break;
    case 0x95: // 16 bits mask register
        OutFile.Put("word ");  break;
    case 0x300:  // MMX
        OutFile.Put("qword ");  break;
    case 0x400:  // XMM
        OutFile.Put("xmmword ");  break;
    case 0x500:  // YMM
        OutFile.Put("ymmword ");  break;
    case 0x600:  // ZMM
        OutFile.Put("zmmword ");  break;
    case 0x700:  // future 1024 bit
        OutFile.Put("?mmword ");  break;
    }
    OutFile.Put("ptr ");
}

void CDisassembler::WriteOperandTypeYASM(uint32 type) {
    // Write type override before operand, e.g. "dword", NASM/YASM syntax
    if (type & 0xF00) {
        type &= 0xF00;                             // Ignore element type for vectors
    }
    else {
        type &= 0xFF;                              // Use operand type only
    }
    uint32 Dest = s.OpcodeDef->Destination & 0xFF;// Destination operand
    if (Dest >= 0xB && Dest < 0x10) {
        // This is a pointer
        if (Dest < 0x0D) {
            OutFile.Put("near ");                   // Near indirect jump/call
        }
        else {
            // Far pointer
            if ((WordSize == 16 && type == 3) || (WordSize == 32 && type == 7)) {
                OutFile.Put("far ");
            }
            else {
                // Size currently not supported by YASM
                switch (type) {
                case 3: OutFile.Put("far ");
                    s.OpComment = "16+16 bit. Needs 66H prefix";
                    break;
                case 7: OutFile.Put("far ");  
                    s.OpComment = "32+16 bit. Possibly forgot REX.W prefix";
                    break;
                case 5: OutFile.Put("far ");  
                    s.OpComment = "64+16 bit. Needs REX.W prefix";
                    break;
                }
            }
        }
        return;
    }
    switch (type) {
    case 1:  // 8 bits
        OutFile.Put("byte ");  break;
    case 2:  // 16 bits
        OutFile.Put("word ");  break;
    case 3:  // 32 bits
        OutFile.Put("dword ");  break;
    case 4:  // 64 bits
        OutFile.Put("qword ");  break;
    case 5:  // 80 bits
        OutFile.Put("tbyte ");  break;
    case 7:  // 48 bits
        OutFile.Put("fword ");  break;
    case 0x4A: // 16 bits float
        OutFile.Put("word ");  break;
    case 0x43: // 32 bits float (x87)
    case 0x4B: // 32 bits float (SSE2)
        OutFile.Put("dword ");  break;
    case 0x44: // 64 bits float
    case 0x4C: // 64 bits float (SSE2)
        OutFile.Put("qword ");  break;
    case 0x45: // 80 bits float
        OutFile.Put("tbyte ");  break;
    case 0x84: case 0x85: // far call
        OutFile.Put("far ");  break;
    case 0x95: // 16 bits mask register
        OutFile.Put("word ");  break;
    case 0x300:  // MMX
        OutFile.Put("qword ");  break;
    case 0x400:  // XMM
        OutFile.Put("oword ");  break;
    case 0x500:  // YMM
        OutFile.Put("yword ");  break;
    case 0x600:  // ZMM
        OutFile.Put("zword ");  break;
    case 0x700:  // Future 128 bytes
        OutFile.Put("?word ");  break;
    default:; // Anything else: write nothing
    }
}

void CDisassembler::WriteOperandTypeGASM(uint32 type) {
    // Write type override before operand, e.g. "dword ", GAS syntax
    if (type & 0xF00) {
        type &= 0xF00;                             // Ignore element type for vectors
    }
    else {
        type &= 0xFF;                              // Use operand type only
    }

    switch (type) {
    case 1:  // 8 bits
        OutFile.Put("byte ");  break;
    case 2:  // 16 bits
        OutFile.Put("word ");  break;
    case 3:  // 32 bits
        OutFile.Put("dword ");  break;
    case 4:  // 64 bits
        OutFile.Put("qword ");  break;
    case 5:  // 80 bits
        if ((s.OpcodeDef->Destination & 0xFF) == 0xD) {
            // 64+16 bit far pointer. Not supported by Gas
            OutFile.Put("fword ");
            s.OpComment = "64+16 bit. Needs REX.W prefix";
        }
        else { 
            OutFile.Put("tbyte ");}
        break;
    case 6: case 0x40: case 0x48: case 0:
        // Other size. Write nothing
        break;
    case 7:    // 48 bits
        OutFile.Put("fword ");  
        if ((s.OpcodeDef->Destination & 0xFF) == 0xD && WordSize == 64) {
            // All assemblers I have tried forget the REX.W prefix here. Make a notice
            s.OpComment = "32+16 bit. Possibly forgot REX.W prefix"; 
        }      
        break;
    case 0x4A: // 16 bits float
        OutFile.Put("word ");  break;
    case 0x43: // 32 bits float (x87)
    case 0x4B: // 32 bits float (SSE2)
        OutFile.Put("dword ");  break;
    case 0x44: // 64 bits float
    case 0x4C: // 64 bits float (SSE2)
        OutFile.Put("qword ");  break;
    case 0x45: // 80 bits float
        OutFile.Put("tbyte ");  break;
    case 0x84: case 0x85: // far call
        OutFile.Put("far ");  break;
    case 0x95: // 16 bits mask register
        OutFile.Put("word ");  break;
    case 0x300:  // MMX
        OutFile.Put("qword ");  break;
    case 0x400:  // XMM
        OutFile.Put("xmmword ");  break;
    case 0x500:  // YMM
        OutFile.Put("ymmword ");  break;
    case 0x600:  // ZMM
        OutFile.Put("zmmword ");  break;
    case 0x700:  // future 1024 bit
        OutFile.Put("?mmword ");  break;
    }
}


void CDisassembler::WriteDREXOperand(uint32 Type) {
    // Write register operand from dest bits of DREX byte (AMD only)
    uint32 Num = s.Vreg >> 4;                    // Register number
    // Write register name
    WriteRegisterName(Num, Type);
}

void CDisassembler::WriteVEXOperand(uint32 Type, int i) {
    // Write register operand from VEX.vvvv bits or immediate bits
    uint32 Num;                                   // Register number
    switch (i) {
    case 0:  // Use VEX.vvvv bits
        Num = s.Vreg & 0x1F;  break;
    case 1:  // Use immediate bits 4-7
        Num = Get<uint8>(s.ImmediateField) >> 4;  break;
    case 2:  // Use immediate bits 0-3 (Unused. For possible future use)
        Num = Get<uint8>(s.ImmediateField) & 0x0F;  break;
    default:
        Num = 0;
    }
    // Write register name
    WriteRegisterName(Num, Type);
}


void CDisassembler::WriteOperandAttributeEVEX(int i, int isMem) {
    // Write operand attributes and instruction attributes from EVEX z, LL, b and aaa bits
    // i = operand number (0 = destination, 1 = first source, 2 = second source, 
    // 98 = after last SIMD operand, 99 = after last operand)
    // isMem: true if memory operand, false if register operand
    uint32 swiz = s.OpcodeDef->EVEX;   // indicates meaning of EVEX attribute bits

    if ((swiz & 0x30) && (i == 0 || (s.OpcodeDef->Destination == 0 && i == 1))) {  // first operand
        // write mask
        if (s.Kreg || (swiz & 0xC0)) {
            OutFile.Put(" {k");
            OutFile.PutDecimal(s.Kreg);
            OutFile.Put("}");
            if ((swiz & 0x20) && (s.Esss & 8)) {
                // zeroing
                OutFile.Put("{z}");
            }
        }
    }
    if (swiz & 0x07) {
        // broadcast, rounding or sae allowed
        if (isMem && i < 8) {
            // memory operand
            if ((swiz & 0x01) && (s.Esss & 1)) {            
                // write memory broadcast
                // calculate broadcast factor
                uint32 op = s.Operands[i];  // operand
                uint32 elementsize = GetDataElementSize(op); // element size
                uint32 opv = s.Operands[0];  // any vector operand
                if (!(opv & 0xF00)) opv = s.Operands[1]; // first operand is not a vector, use next
                uint32 vectorsize  = GetDataItemSize(opv); // vector size
                if (vectorsize > elementsize) { // avoid broadcasting to scalar
                    if (elementsize) { // avoid division by zero
                        OutFile.Put(" {1to");
                        OutFile.PutDecimal(vectorsize/elementsize);
                        OutFile.Put("}");
                    }
                    else {
                        OutFile.Put("{unknown broadcast}");
                    }
                }
            }
        }
        if (i == 98 && s.Mod == 3) {   // after last SIMD operand. no memory operand
            // NASM has rounding mode and sae decoration after last SIMD operand with a comma.
            // No spec. for other assemblers available yet (2014). 
            // use i == 99 if it should be placed after last operand.
            // Perhaps the comma should be removed for other assemblers?
            if ((swiz & 0x4) && (s.Esss & 1)) {
                // write rounding mode
                uint32 rounding = (s.Esss >> 1) & 3;
                OutFile.Put(", {");
                OutFile.Put(EVEXRoundingNames[rounding]);
                OutFile.Put("}");
            }
            else if ((swiz & 0x2) && (s.Esss & 1)) {
                // no rounding mode. write sae
                OutFile.Put(", {");
                OutFile.Put(EVEXRoundingNames[4]);
                OutFile.Put("}");
            }
        }
    }
}


void CDisassembler::WriteOperandAttributeMVEX(int i, int isMem) {
    // Write operand attributes and instruction attributes from MVEX sss, e and kkk bits.
    // i = operand number (0 = destination, 1 = first source, 2 = second source, 99 = after last operand)
    // isMem: true if memory operand, false if register operand
    uint32 swiz = s.OpcodeDef->MVEX;   // indicates meaning of MVEX attribute bits
    const int R_sae_syntax = 0;   // syntax alternatives for rounding mode + sae
                                  // 0: {rn-sae}, 1: {rn}{sae}
    const char * text = 0;        // temporary text pointer

    if ((swiz & 0x1000) && (i == 0 || (s.OpcodeDef->Destination == 0 && i == 1))) {  // first operand
        // write mask
        if (s.Kreg || (swiz & 0x2000)) {
            OutFile.Put(" {k");
            OutFile.PutDecimal(s.Kreg);
            OutFile.Put("}");
        }
    }
    if (swiz & 0x1F) {
        // swizzle allowed    
        if (isMem && i < 90) {
            // write memory broadcast/up/down conversion
            text = s.SwizRecord->name;
            if (text && *text) {
                OutFile.Put(" {");  OutFile.Put(text);  OutFile.Put("}");
            }
        }
        //if (i == 2 || ((s.OpcodeDef->Source2 & 0xF0F00) == 0 && i == 1)) {
        if (i == 98) {   // after last SIMD operand
            // last register or memory operand
            if (s.Mod == 3 && !((swiz & 0x700) && (s.Esss & 8))) { // skip alternative meaning of sss field for register operand when E=1
                // write register swizzle
                text = s.SwizRecord->name;
                if (text && *text) {
                    OutFile.Put(" {");  OutFile.Put(text);  OutFile.Put("}");
                }
            }
        }
        if (i == 99) {   // after last operand
            if (s.Mod == 3 && (swiz & 0x300) && (s.Esss & 8)) {            
                // alternative meaning of sss field for register operand when E=1
                switch (swiz & 0x300) {
                case 0x100:  // rounding mode and not sae
                    text = SwizRoundTables[0][0][s.Esss & 3].name;
                    break;
                case 0x200:  // suppress all exceptions
                    if ((s.Esss & 4) && !(swiz & 0x800)) text = "sae";
                    break;
                case 0x300:  // rounding mode and sae
                    text = SwizRoundTables[0][R_sae_syntax][s.Esss & 7].name;
                    break;
                }
            }
            if (text && *text) {
                OutFile.Put(", {");  OutFile.Put(text);  OutFile.Put("}");
            }
        }
    }
    if (isMem && (s.Esss & 8) && !(swiz & 0x800)) {
        // cache eviction hint after memory operand
        OutFile.Put(" {eh}");
    }
}

void CDisassembler::WriteRegisterName(uint32 Value, uint32 Type) {
    // Write name of register to OutFile
    if (Type & 0xF00) {
        // vector register
        Type &= 0xF00;
    }
    else {
        // Other register
        Type &= 0xFF;                              // Remove irrelevant bits
    }

    // Check fixed registers (do not depend on Value)
    switch (Type) {
    case 0xA1:  // al
        Type = 1;  Value = 0;
        break;

    case 0xA2:  // ax
        Type = 2;  Value = 0;
        break;

    case 0xA3:  // eax
        Type = 3;  Value = 0;
        break;

    case 0xA4:  // rax
        Type = 4;  Value = 0;
        break;

    case 0xAE:  // xmm0
        Type = 0x400;  Value = 0;
        break;

    case 0xAF:  // st(0)
        Type = 0x40;  Value = 0;
        break;

    case 0xB2:  // dx
        Type = 2;  Value = 2;
        break;

    case 0xB3:  // cl
        Type = 1;  Value = 1;
        break;
    }

    // Get register number limit
    uint32 RegNumLimit = 7;    // largest register number
    if (WordSize >= 64) {
        RegNumLimit = 15;
        if ((s.Prefixes[6] & 0x40) && (Type & 0xF40)) {
            // EVEX or MVEX prefix and vector
            RegNumLimit = 31;
        }
    }

    switch (Type) {
    case 0x91:     // segment register
        RegNumLimit = 5;
        break;
    case 0x300:  // mmx
    case 0x40:   // st register         
    case 0x95:   // k mask register
        RegNumLimit = 7;
        break;
    case 0x98:   // bounds register
        RegNumLimit = 3;
        break;
    }
    if (Value > RegNumLimit) {
        // register number out of range
        OutFile.Put("unknown register ");
        switch (Type) {
        case 1:
            OutFile.Put("(8 bit) ");  break;
        case 2:
            OutFile.Put("(16 bit) ");  break;
        case 3:
            OutFile.Put("(32 bit) ");  break;
        case 4:
            OutFile.Put("(64 bit) ");  break;
        case 0x40:   // st register
            OutFile.Put("st");  break;
        case 0x91:  // Segment register
            OutFile.Put("seg");  break;
        case 0x92:  // Control register
            OutFile.Put("cr");  break;
        case 0x95:  // k mask register
            OutFile.Put("k");  break;
        case 0x300:  // mmx register
            OutFile.Put("mm");  break;
        case 0x400:  // xmm register
            OutFile.Put("xmm");  break;
        case 0x500:  // ymm register
            OutFile.Put("ymm");  break;
        case 0x600:  // zmm register
            OutFile.Put("zmm");  break;
        case 0x700:  // future 1024 bit register
            OutFile.Put("?mm");  break;
        }
        OutFile.PutDecimal(Value);
    }
    else {
        // Write register name depending on type
        switch (Type) {
        case 1:  // 8 bit register. Depends on any REX prefix
            OutFile.Put(s.Prefixes[7] ? RegisterNames8x[Value] : RegisterNames8[Value & 7]);
            break;

        case 2:  // 16 bit register
            OutFile.Put(RegisterNames16[Value]);
            break;

        case 3:  // 32 bit register
            OutFile.Put(RegisterNames32[Value]);
            break;

        case 4:  // 64 bit register
            OutFile.Put(RegisterNames64[Value]);
            break;

        case 0x300:  // mmx register
            OutFile.Put("mm");
            OutFile.PutDecimal(Value);
            break;

        case 0x400:  // xmm register (packed integer or float)
        case 0x48: case 0x4B: case 0x4C: // xmm register (scalar float)
            OutFile.Put("xmm");
            OutFile.PutDecimal(Value);
            break;

        case 0x500:  // ymm register (packed)
            OutFile.Put("ymm");
            OutFile.PutDecimal(Value);
            break;

        case 0x600:  // zmm register (packed)
            OutFile.Put("zmm");
            OutFile.PutDecimal(Value);
            break;

        case 0x700:  // future 1024 bit register
            OutFile.Put("?mm");
            OutFile.PutDecimal(Value);
            break;

        case 0x40:  // st register
            if (Syntax == SUBTYPE_YASM) {
                // NASM, YASM and GAS-AT&T use st0
                OutFile.Put("st");
                OutFile.PutDecimal(Value);
            }
            else {
                // MASM and GAS-Intel use st(0), 
                OutFile.Put("st(");
                OutFile.PutDecimal(Value);
                OutFile.Put(")");
            }
            break;

        case 0x91:  // Segment register
            OutFile.Put(RegisterNamesSeg[Value & 7]);
            break;

        case 0x92:  // Control register
            OutFile.Put(RegisterNamesCR[Value]);
            break;

        case 0x93:  // Debug register
            OutFile.Put("dr");
            OutFile.PutDecimal(Value);
            break;

        case 0x94:  // Test register (obsolete)
            OutFile.Put("tr");
            OutFile.PutDecimal(Value);
            break;

        case 0x95:  // k mask register
            OutFile.Put("k");
            OutFile.PutDecimal(Value);
            break;

        case 0x98:  // bounds register
            OutFile.Put("bnd");
            OutFile.PutDecimal(Value);
            break;

        case 0xB1:  // 1
            OutFile.Put("1");
            break;

        default:    // Unexpected
            OutFile.Put("UNKNOWN REGISTER TYPE ");
            OutFile.PutDecimal(Value);
            break;
        }
    }
}


void CDisassembler::WriteImmediateOperand(uint32 Type) {
    // Write immediate operand or direct jump/call address
    int    WriteFormat;                 // 0: unsigned, 1: signed, 2: hexadecimal
    int    Components = 0;              // Number of components in immediate operand       
    uint32 OSize;                       // Operand size
    uint32 FieldPointer;                // Pointer to field containing value
    uint32 FieldSize;                   // Size of field containing value
    int64  Value = 0;                   // Value of immediate operand

    // Check if far
    if ((Type & 0xFE) == 0x84) {
        // Write far 
        WriteOperandType(Type);
    }

    // Check if type override needed
    if ((s.OpcodeDef->AllowedPrefixes & 2) && s.Prefixes[4] == 0x66
        && (Opcodei == 0x68 || Opcodei == 0x6A)) {
            // Push immediate with non-default operand size needs type override
            WriteOperandType(s.OperandSize == 16 ? 2 : 3);
    }

    FieldPointer = s.ImmediateField;
    FieldSize    = s.ImmediateFieldSize;

    if (Syntax == SUBTYPE_YASM && (Type & 0x0F) == 4 && FieldSize == 8) {
        // Write type override to make sure we get 8 bytes address in case there is a relocation here
        WriteOperandType(4);
    }

    if (Type & 0x200000) {
        if (FieldSize > 1) {
            // Uses second part of field. Single byte only
            FieldPointer += FieldSize-1;
            FieldSize = 1;
        }
        else {
            // Uses half a byte
            FieldSize = 0;
        }
    }

    // Get inline value
    switch (FieldSize) {
    case 0:  // 4 bits
        Value = Get<uint8>(FieldPointer) & 0x0F;
        break;

    case 1:  // 8 bits
        Value = Get<int8>(FieldPointer);  
        break;

    case 2:  // 16 bits
        Value = Get<int16>(FieldPointer);  break;

    case 6:  // 48 bits
        Value  = Get<int32>(FieldPointer);  
        Value += (uint64)Get<uint16>(FieldPointer + 4) << 32;  
        break;

    case 4:  // 32 bits
        Value = Get<int32>(FieldPointer);  break;

    case 8:  // 64 bits
        Value = Get<int64>(FieldPointer);  break;

    case 3:  // 16+8 bits ("Enter" instruction)
        if ((Type & 0xFF) == 0x12) {
            // First 16 bits
            FieldSize = 2; Value = Get<int16>(FieldPointer);  break;
        }
        // else continue in default case to get error message

    default:  // Other sizes should not occur
        err.submit(3000);  Value = -1;
    }

    // Check if relocation
    if (s.ImmediateRelocation) {
        // Write relocation target name
        uint32 Context = 2;
        if ((Type & 0xFC) == 0x80) Context = 8;     // Near jump/call destination
        if ((Type & 0xFC) == 0x84) Context = 0x10;  // Far jump/call destination

        // Write cross reference
        WriteRelocationTarget(s.ImmediateRelocation, Context, Value);

        // Remember that Value has been written
        Value = 0;
        Components++;
    }
    // Check if AAM or AAD
    if (Value == 10 && (Opcodei & 0xFE) == 0xD4) {
        // Don't write operand for AAM or AAD if = 10
        return;
    }

    // Write as unsigned, signed or hexadecimal:
    if ((Type & 0xF0) == 0x30 || (Type & 0xF0) == 0x80) {
        // Hexadecimal
        WriteFormat = 2;
    }
    else if (s.ImmediateFieldSize == 8) {
        // 64 bit constant
        if (Value == (int32)Value) {
            // Signed
            WriteFormat = 1;
        }
        else {
            // Hexadecimal
            WriteFormat = 2;
        }
    }
    else if ((Type & 0xF0) == 0x20) {
        // Signed
        WriteFormat = 1;
    }
    else {
        // Unsigned
        WriteFormat = 0;
    }

    if ((Type & 0xFC) == 0x80 && !s.ImmediateRelocation) {
        // Self-relative jump or call without relocation. Adjust immediate value
        Value += IEnd;                             // Get absolute address of target

        // Look for symbol at target address
        uint32 ISymbol = Symbols.FindByAddress(Section, (uint32)Value);
        if (ISymbol && (Symbols[ISymbol].Name || CodeMode == 1)) {
            // Symbol found. Write its name
            OutFile.Put(Symbols.GetName(ISymbol));
            // No offset to write
            return;
        }
        // Target address has no name
        Type |= 0x4000;                            // Write target as hexadecimal
    }

    // Operand size
    if ((s.Operands[0] & 0xFFF) <= 0xA || (s.Operands[0] & 0xF0) == 0xA0) {
        // Destination is general purpose register
        OSize = s.OperandSize;
    }
    else {
        // Constant probably unrelated to destination size
        OSize = 8;
    }
    // Check if destination is 8 bit operand
    //if ((s.Operands[0] & 0xFF) == 1 || (s.Operands[0] & 0xFF) == 0xA1) OSize = 8;

    // Check if sign extended
    if (OSize > s.ImmediateFieldSize * 8) {
        if (WriteFormat == 2 && Value >= 0) {
            // Hexadecimal sign extended, not negative:
            // Does not need full length
            OSize = s.ImmediateFieldSize * 8;
        }
        else if (WriteFormat == 0) {
            // Unsigned and sign extended, change to signed
            WriteFormat = 1;
        }
    }

    if (Components) {
        // There was a relocated name
        if (Value) {
            // Addend to relocation is not zero
            if (Value > 0 || WriteFormat != 1) {
                OutFile.Put("+");                  // Put "+" between name and addend
            }
            else {
                OutFile.Put("-");                  // Put "-" between name and addend
                Value = - Value;                  // Change sign to avoid another "-"
            }
        }
        else {
            // No addend to relocated name
            return;
        }
    }
    // Write value
    if (WriteFormat == 2) {
        // Write with hexadecimal number appropriate size
        switch (OSize) {
        case 8:  // 8 bits
            OutFile.PutHex((uint8)Value, 1);  break;
        case 16:  // 16 bits
            if ((Type & 0xFC) == 0x84) {
                // Segment of far call
                OutFile.PutHex((uint16)(Value >> 16), 1);
                OutFile.Put(':');
            }
            OutFile.PutHex((uint16)Value, 2);  break;
        case 32:  // 32 bits
        default:  // Should not occur
            if ((Type & 0xFC) == 0x84) {
                // Segment of far call
                OutFile.PutHex((uint16)(Value >> 32), 1);
                OutFile.Put(':');
            }
            OutFile.PutHex((uint32)Value, 2);  break;
        case 64:  // 64 bits
            OutFile.PutHex((uint64)Value, 2);  break;
        }
    }
    else {
        // Write as signed or unsigned decimal
        if (WriteFormat == 0) { // unsigned
            switch (OSize) {
            case 8:  // 8 bits
                Value &= 0x00FF;  break;
            case 16:  // 16 bits
                Value &= 0xFFFF;  break;
            }
        }
        OutFile.PutDecimal((int32)Value, WriteFormat);  // Write value. Signed or usigned decimal
    }
}


void CDisassembler::WriteOtherOperand(uint32 Type) {
    // Write other type of operand
    const char * * OpRegisterNames;               // Pointer to list of register names
    uint32 RegI = 0;                              // Index into list of register names

    switch (Type & 0x8FF) {
    case 0xA1:  // AL
        OpRegisterNames = RegisterNames8;
        break;
    case 0xA2:  // AX
        OpRegisterNames = RegisterNames16;
        break;
    case 0xA3:  // EAX
        OpRegisterNames = RegisterNames32;
        break;
    case 0xA4:  // RAX
        OpRegisterNames = RegisterNames64;
        break;
    case 0xAE:  // xmm0
        OutFile.Put("xmm0");
        return;
    case 0xAF:  // ST(0)
        OutFile.Put("st(0)");
        return;
    case 0xB1:  // 1
        OutFile.Put("1");
        return;
    case 0xB2:  // DX
        OpRegisterNames = RegisterNames16;
        RegI = 2;
        break;
    case 0xB3: // CL
        OpRegisterNames = RegisterNames8;
        RegI = 1;
        break;
    default:
        OutFile.Put("unknown operand");
        err.submit(3000);
        return;
    }
    // Write register name
    OutFile.Put(OpRegisterNames[RegI]);
}


void CDisassembler::WriteErrorsAndWarnings() {
    // Write errors, warnings and comments, if any
    uint32 n;                                     // Error bit
    if (s.Errors) {
        // There are errors
        // Loop through all bits in s.Errors
        for (n = 1; n; n <<= 1) {
            if (s.Errors & n) {
                if (OutFile.GetColumn()) OutFile.NewLine(); 
                OutFile.Put(CommentSeparator);       // Write "\n; "
                OutFile.Put("Error: ");              // Write "Error: "
                OutFile.Put(Lookup(AsmErrorTexts,n));// Write error text
                OutFile.NewLine(); 
            }
        }
    }

    if (s.Warnings1) {
        // There are warnings 1
        // Loop through all bits in s.Warnings1
        for (n = 1; n; n <<= 1) {
            if (s.Warnings1 & n) {
                if (OutFile.GetColumn()) OutFile.NewLine(); 
                OutFile.Put(CommentSeparator);       // Write "; "
                OutFile.Put("Note: ");               // Write "Note: "
                OutFile.Put(Lookup(AsmWarningTexts1, n));// Write warning text
                OutFile.NewLine(); 
            }
        }
    }
    if (s.Warnings2) {
        // There are warnings 2
        // Loop through all bits in s.Warnings2
        for (n = 1; n; n <<= 1) {
            if (s.Warnings2 & n) {
                if (OutFile.GetColumn()) OutFile.NewLine(); 
                OutFile.Put(CommentSeparator);            // Write "; "
                OutFile.Put("Warning: ");                 // Write "Warning: "
                OutFile.Put(Lookup(AsmWarningTexts2, n)); // Write warning text
                OutFile.NewLine(); 
            }
        }
        if (s.Warnings2 & 1) {
            // Write spurious label
            uint32 sym1 = Symbols.FindByAddress(Section, LabelEnd);
            if (sym1) {
                const char * name = Symbols.GetName(sym1);
                OutFile.Put(CommentSeparator);
                OutFile.Put(name);
                OutFile.Put("; Misplaced symbol at address ");
                OutFile.PutHex(Symbols[sym1].Offset);
                OutFile.NewLine();
            }
        }
    }

    if (s.OpcodeDef && (s.OpcodeDef->AllowedPrefixes & 8) && !s.Warnings1) {
        if (s.Prefixes[0]) {
            // Branch hint prefix. Write comment
            OutFile.Put(CommentSeparator);             // Write "; "
            switch (s.Prefixes[0]) {
            case 0x2E:
                OutFile.Put("Branch hint prefix for Pentium 4: Predict no jump");   
                break;
            case 0x3E:
                OutFile.Put("Branch hint prefix for Pentium 4: Predict jump");   
                break;
            case 0x64:
                OutFile.Put("Branch hint prefix for Pentium 4: Predict alternate");   
                break;
            default:
                OutFile.Put("Note: Unrecognized branch hint prefix");   
            }
            OutFile.NewLine(); 
        }
    }
}

void CDisassembler::WriteSymbolName(uint32 symi) {
    // Write symbol name. symi = new symbol index
    OutFile.Put(Symbols.GetName(symi));
}

void CDisassembler::WriteSectionName(int32 SegIndex) {
    // Write name of section, segment or group from section index
    const char * Name = 0;
    // Check for special index values
    switch (SegIndex) {
    case ASM_SEGMENT_UNKNOWN:   // Unknown segment. Typical for external symbols
        Name = "Unknown";  break;
    case ASM_SEGMENT_ABSOLUTE:  // No segment. Used for absolute symbols
        Name = "Absolute";  break;
    case ASM_SEGMENT_FLAT:      // Flat segment group
        Name = "flat";  break;
    case ASM_SEGMENT_NOTHING:   // No segment
        Name = "Nothing";  break;
    case ASM_SEGMENT_ERROR:     // Segment register assumed to error
        Name = "Error";  break;
    case ASM_SEGMENT_IMGREL:    // Segment unknown. Offset relative to image base or file base
        Name = "ImageBased";  break;
    default:                    // > 0 means normal segment index
        if ((uint32)SegIndex >= Sections.GetNumEntries()) {
            // Out of range
            Name = "IndexOutOfRange";
       }
       else {
           // Get index into NameBuffer
           uint32 NameIndex = Sections[SegIndex].Name;
           // Check if valid
           if (NameIndex == 0 || NameIndex >= NameBuffer.GetDataSize()) {
               Name = "ErrorNameMissing";
           }
           else {
               // Normal valid name of segment, section or group
               Name = NameBuffer.Buf() + NameIndex;
           }
       }
       break;
    }
    if (Syntax == SUBTYPE_YASM && Name[0] == '_') {
        // Change leading underscore to dot
        OutFile.Put('.');
        OutFile.Put(Name+1); // Write rest of name
    }
    else {
        // Write name
        OutFile.Put(Name);
    }
}

void CDisassembler::WriteDataItems() {
    // Write data items to output file

    int LineState;  // 0: Start of new line, write label
    // 1: Label written if any, write data directive
    // 2: Data directive written, write data
    // 3: First data item written, write comma and more data
    // 4: Last data item written, write comment
    // 5: Comment written if any, start new line
    uint32 Pos = IBegin;                          // Current position
    uint32 LinePos = IBegin;                      // Position for beginning of output line
    uint32 BytesPerLine;                          // Number of bytes to write per line
    uint32 LineEnd;                               // Data position for end of line
    uint32 DataEnd;                               // End of data
    uint32 ElementSize, OldElementSize;           // Size of each data element
    uint32 RelOffset;                             // Offset of relocation
    uint32 irel, Oldirel;                         // Relocation index
    int64  Value;                                 // Inline value or addend
    const char * Symname;                         // Symbol name
    int    SeparateLine;                          // Label is on separate line

    SARelocation Rel;                             // Dummy relocation record

    // Check if size is valid
    if (DataSize == 0) DataSize = 1;
    if (DataSize > 32) DataSize = 32;  

    // Expected end position
    if (CodeMode & 3) {
        // Writing data for dubious code. Make same length as code instruction
        DataEnd = IEnd;
    }
    else {
        // Regular data. End at next label
        DataEnd = LabelEnd;
        if (DataEnd > FunctionEnd) DataEnd = FunctionEnd;
        if (DataEnd <= Pos) DataEnd = Pos + DataSize;
        if (DataEnd > Sections[Section].InitSize && Pos < Sections[Section].InitSize) {
            DataEnd = Sections[Section].InitSize;
        }
    }

    // Size of each data element
    ElementSize = DataSize;

    // Check if packed type
    if (DataType & 0xF00) {
        // This is a packed vector type. Get element size
        ElementSize = GetDataElementSize(DataType);
    }

    // Avoid sizes that are not powers of 2
    if (ElementSize == 6 || ElementSize == 10) ElementSize = 2;

    // Set maximum element size to 8
    if (ElementSize > 8)  ElementSize = 8;

    // Set minimum element size to 1
    if (ElementSize < 1)  ElementSize = 1;

    if (Pos + ElementSize > DataEnd) {
        // Make sure we end at DataEnd
        ElementSize = 1;  BytesPerLine = 8;
        LineEnd = DataEnd;
    }

    // Set number of bytes per line
    BytesPerLine = (DataSize == 10) ? 10 : 8;

    if (!(CodeMode & 3)) {
        // Begin new line for each data item (except in code segment)
        OutFile.NewLine();
    }
    LineState = 0; irel = 0;

    // Check if alignment required
    if (DataSize >= 16 && (DataType & 0xC00) && (DataType & 0xFF) != 0x51 
        && (FlagPrevious & 0x100) < (DataSize << 4) && !(IBegin & (DataSize-1))) {
            // Write align directive
            WriteAlign(DataSize);
            // Remember that data is aligned
            FlagPrevious |= (DataSize << 4);
    }

    // Get symbol name for label
    uint32 sym;                                   // Current symbol index
    uint32 sym1, sym2 = 0;                        // First and last symbol at current address

    sym1 = Symbols.FindByAddress(Section, Pos, &sym2);

    // Loop for one or more symbols at this address
    for (sym = sym1; sym <= sym2; sym++) {

        if (sym && Symbols[sym].Scope && !(Symbols[sym].Scope & 0x100) && !(Symbols[sym].Type & 0x80000000)) {

            // Prepare for writing symbol label
            Symname = Symbols.GetName(sym);         // Symbol name
            // Check if label needs a separate line
            SeparateLine = (ElementSize != DataSize 
                || Symbols[sym].Size != DataSize 
                || strlen(Symname) > AsmTab1
                || sym < sym2 
                // || (Sections[Section].Type & 0xFF) == 3
                || ((Symbols[sym].Type+1) & 0xFE) == 0x0C);

            // Write symbol label
            switch (Syntax) {
            case SUBTYPE_MASM:
                WriteDataLabelMASM(Symname, sym, SeparateLine);  break;
            case SUBTYPE_YASM:
                WriteDataLabelYASM(Symname, sym, SeparateLine);  break;
            case SUBTYPE_GASM:
                WriteDataLabelGASM(Symname, sym, SeparateLine);  break;
            }
            LineState = 1;                          // Label written
            if (SeparateLine) {
                LineState = 0;
            }
        }
    }

    if ((Sections[Section].Type & 0xFF) == 3 || Pos >= Sections[Section].InitSize) {
        // This is an unitialized data (BSS) section
        // Data repeat count
        uint32 DataCount = (DataEnd - Pos) / ElementSize;
        if (DataCount) {
            OutFile.Tabulate(AsmTab1);
            // Write data directives
            switch (Syntax) {
            case SUBTYPE_MASM:
                WriteUninitDataItemsMASM(ElementSize, DataCount);  break;
            case SUBTYPE_YASM:
                WriteUninitDataItemsYASM(ElementSize, DataCount);  break;
            case SUBTYPE_GASM:
                WriteUninitDataItemsGASM(ElementSize, DataCount);  break;
            }
            // Write comment
            WriteDataComment(ElementSize, Pos, Pos, 0);
            OutFile.NewLine();
            LineState = 0;
        }
        // Update data position
        Pos += DataCount * ElementSize;

        if (Pos < DataEnd) {
            // Some odd data remain. Write as bytes
            DataCount = DataEnd - Pos;
            ElementSize = 1;
            OutFile.Tabulate(AsmTab1);
            switch (Syntax) {
            case SUBTYPE_MASM:
                WriteUninitDataItemsMASM(ElementSize, DataCount);  break;
            case SUBTYPE_YASM:
                WriteUninitDataItemsYASM(ElementSize, DataCount);  break;
            case SUBTYPE_GASM:
                WriteUninitDataItemsGASM(ElementSize, DataCount);  break;
            }
            // Write comment
            WriteDataComment(ElementSize, Pos, Pos, 0);
            OutFile.NewLine();
            Pos = DataEnd;
            LineState = 0;
        }
    }
    else {
        // Not a BSS section
        // Label has been written, write data

        // Loop for one or more elements
        LinePos = Pos;
        while (Pos < DataEnd) {

            // Find end of line position
            LineEnd = LinePos + BytesPerLine;

            // Remember element size and relocation
            OldElementSize = ElementSize;
            Oldirel = irel;

            // Check if relocation
            Rel.Section = Section;
            Rel.Offset  = Pos;
            uint32 irel = Relocations.FindFirst(Rel);
            if (irel >= Relocations.GetNumEntries() || Relocations[irel].Section != (int32)Section) {
                // No relevant relocation
                irel = 0;
            }
            if (irel) {
                // A relocation is found
                // Check relocation source
                RelOffset = Relocations[irel].Offset;
                if (RelOffset == Pos) {
                    // Relocation source is here
                    // Make sure the size fits and begin new line
                    ElementSize = Relocations[irel].Size;  BytesPerLine = 8;
                    if (ElementSize < 1) ElementSize = WordSize / 8;
                    if (ElementSize < 1) ElementSize = 4;
                    LineEnd = Pos + ElementSize;
                    if (LineState > 2) LineState = 4; // Make sure we begin at new line
                }
                else if (RelOffset < Pos + ElementSize) {
                    // Relocation source begins before end of element with current ElementSize
                    // Change ElementSize to make sure a new element begins at relocation source
                    ElementSize = 1;  BytesPerLine = 8;
                    LineEnd = RelOffset;
                    if (LineState > 2) LineState = 4; // Make sure we begin at new line
                    irel = 0;
                }
                else {
                    // Relocation is after this element
                    irel = 0;
                }
                // Check for overlapping relocations
                if (irel && irel+1 < Relocations.GetNumEntries() 
                    && Relocations[irel+1].Section == (int32)Section
                    && Relocations[irel+1].Offset < RelOffset + ElementSize) {
                        // Overlapping relocations
                        s.Errors |= 0x2000;
                        WriteErrorsAndWarnings();
                        LineEnd = Relocations[irel+1].Offset;
                        if (LineState > 2) LineState = 4; // Make sure we begin at new line
                }
                // Drop alignment
                FlagPrevious &= ~0xF00;
            }
            if (irel == 0) {
                // No relocation here
                // Check if DataEnd would be exceeded
                if (Pos + ElementSize > DataEnd) {
                    // Make sure we end at DataEnd unless there is a relocation source here
                    ElementSize = 1;  BytesPerLine = 8;
                    LineEnd = DataEnd;
                    if (LineState > 2) LineState = 4; // Make sure we begin at new line
                    FlagPrevious &= ~0xF00;           // Drop alignment
                }
            }
            // Check if new line needed
            if (LineState == 4) {
                // Finish this line
                if (!(CodeMode & 3)) {
                    WriteDataComment(OldElementSize, LinePos, Pos, Oldirel);
                }
                // Start new line
                OutFile.NewLine();
                LineState = 0;
                LinePos = Pos;
                continue;
            }

            // Tabulate
            OutFile.Tabulate(AsmTab1);

            if (LineState < 2) {
                // Write data definition directive for appropriate size
                switch (Syntax) {
                case SUBTYPE_MASM:
                    WriteDataDirectiveMASM(ElementSize);  break;
                case SUBTYPE_YASM:
                    WriteDataDirectiveYASM(ElementSize);  break;
                case SUBTYPE_GASM:
                    WriteDataDirectiveGASM(ElementSize);  break;
                }
                LineState = 2;
            }
            else if (LineState == 3) {
                // Not the first element, write comma
                OutFile.Put(", ");
            }
            // Get inline value
            switch (ElementSize) {
            case 1:  Value = Get<int8>(Pos);  break;
            case 2:  Value = Get<int16>(Pos);  break;
            case 4:  Value = Get<int32>(Pos);  break;
            case 6:  Value = Get<uint32>(Pos) + ((uint64)Get<uint16>(Pos+4) << 32); break;
            case 8:  Value = Get<int64>(Pos);  break;
            case 10: Value = Get<int64>(Pos); break;
            default: Value = 0; // should not occur
            }
            if (irel) {
                // There is a relocation here. Write the name etc.
                WriteRelocationTarget(irel, 1, Value);
            }
            else {
                // Write value
                switch (ElementSize) {
                case 1:
                    OutFile.PutHex((uint8)Value, 1);  
                    break;
                case 2:
                    OutFile.PutHex((uint16)Value, 1);  
                    break;
                case 4:
                    OutFile.PutHex((uint32)Value, 1);  
                    break;
                case 6:
                    OutFile.PutHex((uint16)(Value >> 32), 1);  
                    OutFile.Put(":");
                    OutFile.PutHex((uint32)Value, 1);  
                    break;
                case 8:
                    OutFile.PutHex((uint64)Value, 1);  
                    break;
                case 10:
                    OutFile.Put("??");
                    break;
                }
            }
            LineState = 3;
            // Increment position
            Pos += ElementSize;

            // Check if end of line
            if (Pos >= LineEnd || Pos >= DataEnd) LineState = 4;

            if (LineState == 4) {
                // End of line
                if (!(CodeMode & 3)) {
                    // Write comment
                    WriteDataComment(ElementSize, LinePos, Pos, irel);
                }
                OutFile.NewLine();
                LinePos = Pos;
                LineState = 0;
            }
        }
    }

    // Indicate end
    if (IEnd < Pos) IEnd = Pos;
    if (IEnd > LabelEnd) IEnd = LabelEnd;
    if (IEnd > FunctionEnd && FunctionEnd) IEnd = FunctionEnd;

    // Reset FlagPrevious if not aligned
    if (DataSize < 16 || (DataType & 0xFF) == 0x28) FlagPrevious = 0;
}


void CDisassembler::WriteDataLabelMASM(const char * name, uint32 sym, int line) {
    // Write label before data item, MASM syntax
    // name = name of data item(s)
    // sym  = symbol index
    // line = 1 if label is on separate line, 0 if data follows on same line
    // Write name
    OutFile.Put(name);
    // At least one space
    OutFile.Put(" ");
    // Tabulate
    OutFile.Tabulate(AsmTab1);

    if (line) {
        // Write label and type on seperate line
        // Get size
        uint32 Symsize = Symbols[sym].Size;
        if (Symsize == 0) Symsize = DataSize;
        OutFile.Put("label ");
        // Write type
        switch(Symsize) {
        case 1: default:
            OutFile.Put("byte");  break;
        case 2:
            OutFile.Put("word");  break;
        case 4:
            OutFile.Put("dword");  break;
        case 6:
            OutFile.Put("fword");  break;
        case 8:
            OutFile.Put("qword");  break;
        case 10:
            OutFile.Put("tbyte");  break;
        case 16:
            OutFile.Put("xmmword");  break;
        case 32:
            OutFile.Put("ymmword");  break;
        }
        // Check if jump table or call table
        if (((Symbols[sym].Type+1) & 0xFE) == 0x0C) {
            OutFile.Tabulate(AsmTab3);
            OutFile.Put(CommentSeparator);
            if (Symbols[sym].DLLName) {
                // DLL import
                OutFile.Put("import from ");
                OutFile.Put(Symbols.GetDLLName(sym));
            }
            else if (Symbols[sym].Type & 1) {
                OutFile.Put("switch/case jump table");
            }
            else {
                OutFile.Put("virtual table or function pointer");
            }
        }
        // New line
        OutFile.NewLine();
    }
}

void CDisassembler::WriteDataLabelYASM(const char * name, uint32 sym, int line) {
    // Write label before data item, YASM syntax
    // name = name of data item(s)
    // sym  = symbol index
    // line = 1 if label is on separate line, 0 if data follows on same line
    // Write name and colon
    OutFile.Put(name);
    OutFile.Put(": ");
    // Tabulate
    OutFile.Tabulate(AsmTab1);

    if (line) {
        // Write label on seperate line
        // Write comment
        OutFile.Tabulate(AsmTab3);
        OutFile.Put(CommentSeparator);
        // Check if jump table or call table
        if (((Symbols[sym].Type+1) & 0xFE) == 0x0C) {
            if (Symbols[sym].DLLName) {
                // DLL import
                OutFile.Put("import from ");
                OutFile.Put(Symbols.GetDLLName(sym));
            }
            else if (Symbols[sym].Type & 1) {
                OutFile.Put("switch/case jump table");
            }
            else {
                OutFile.Put("virtual table or function pointer");
            }
        }
        else {
            // Write size
            uint32 Symsize = Symbols[sym].Size;
            if (Symsize == 0) Symsize = DataSize;
            switch(Symsize) {
            case 1: default:
                OutFile.Put("byte");  break;
            case 2:
                OutFile.Put("word");  break;
            case 4:
                OutFile.Put("dword");  break;
            case 6:
                OutFile.Put("fword");  break;
            case 8:
                OutFile.Put("qword");  break;
            case 10:
                OutFile.Put("tbyte");  break;
            case 16:
                OutFile.Put("oword");  break;
            case 32:
                OutFile.Put("yword");  break;
            case 64:
                OutFile.Put("zword");  break;
            }
        }
        // New line
        OutFile.NewLine();
    }
}

void CDisassembler::WriteDataLabelGASM(const char * name, uint32 sym, int line) {
    // Write label before data item, GAS syntax
    // name = name of data item(s)
    // sym  = symbol index
    // line = 1 if label is on separate line, 0 if data follows on same line
    // Write name and colon
    OutFile.Put(name);
    OutFile.Put(": ");
    // Tabulate
    OutFile.Tabulate(AsmTab1);

    if (line) {
        // Write label on seperate line
        // Write comment
        OutFile.Tabulate(AsmTab3);
        OutFile.Put(CommentSeparator);
        // Check if jump table or call table
        if (((Symbols[sym].Type+1) & 0xFE) == 0x0C) {
            if (Symbols[sym].DLLName) {
                // DLL import
                OutFile.Put("import from ");
                OutFile.Put(Symbols.GetDLLName(sym));
            }
            else if (Symbols[sym].Type & 1) {
                OutFile.Put("switch/case jump table");
            }
            else {
                OutFile.Put("virtual table or function pointer");
            }
        }
        else {
            // Write size
            uint32 Symsize = Symbols[sym].Size;
            if (Symsize == 0) Symsize = DataSize;
            switch(Symsize) {
            case 1: default:
                OutFile.Put("byte");  break;
            case 2:
                OutFile.Put("word");  break;
            case 4:
                OutFile.Put("int");  break;
            case 6:
                OutFile.Put("farword");  break;
            case 8:
                OutFile.Put("qword");  break;
            case 10:
                OutFile.Put("tfloat");  break;
            case 16:
                OutFile.Put("xmmword");  break;
            case 32:
                OutFile.Put("ymmword");  break;
            }
        }
        // New line
        OutFile.NewLine();
    }
}

void CDisassembler::WriteUninitDataItemsMASM(uint32 size, uint32 count) {
    // Write uninitialized (BSS) data, MASM syntax
    // size = size of each data element
    // count = number of data elements on each line

    // Write data definition directive for appropriate size
    switch (size) {
    case 1:
        OutFile.Put("db ");  break;
    case 2:
        OutFile.Put("dw ");  break;
    case 4:
        OutFile.Put("dd ");  break;
    case 6:
        OutFile.Put("df ");  break;
    case 8:
        OutFile.Put("dq ");  break;
    case 10:
        OutFile.Put("dt ");  break;
    }
    OutFile.Tabulate(AsmTab2);
    if (count > 1) {
        // Write duplication operator
        OutFile.PutDecimal(count);
        OutFile.Put(" dup (?)");
    }
    else {
        // DataCount == 1
        OutFile.Put("?");
    }
}

void CDisassembler::WriteUninitDataItemsYASM(uint32 size, uint32 count) {
    // Write uninitialized (BSS) data, YASM syntax
    // Write data definition directive for appropriate size
    switch (size) {
    case 1:
        OutFile.Put("resb ");  break;
    case 2:
        OutFile.Put("resw ");  break;
    case 4:
        OutFile.Put("resd ");  break;
    case 6:
        OutFile.Put("resw ");  count *= 3;  break;
    case 8:
        OutFile.Put("resq ");  break;
    case 10:
        OutFile.Put("rest ");  break;
    }
    OutFile.Tabulate(AsmTab2);
    OutFile.PutDecimal(count);
}

void CDisassembler::WriteUninitDataItemsGASM(uint32 size, uint32 count) {
    // Write uninitialized (BSS) data, GAS  syntax
    OutFile.Put(".zero");
    OutFile.Tabulate(AsmTab2);
    if (count != 1) {
        OutFile.PutDecimal(count);  OutFile.Put(" * ");
    }
    OutFile.PutDecimal(size);
}

void CDisassembler::WriteDataDirectiveMASM(uint32 size) {
    // Write DB, etc., MASM syntax
    // Write data definition directive for appropriate size
    switch (size) {
    case 1:  OutFile.Put("db ");  break;
    case 2:  OutFile.Put("dw ");  break;
    case 4:  OutFile.Put("dd ");  break;
    case 6:  OutFile.Put("df ");  break;
    case 8:  OutFile.Put("dq ");  break;
    case 10: OutFile.Put("dt ");  break;
    case 16: OutFile.Put("xmmword ");  break;
    case 32: OutFile.Put("ymmword ");  break;
    default: OutFile.Put("Error ");  break;
    }
}

void CDisassembler::WriteDataDirectiveYASM(uint32 size) {
    // Write DB, etc., YASM syntax
    // Write data definition directive for appropriate size
    switch (size) {
    case 1:  OutFile.Put("db ");  break;
    case 2:  OutFile.Put("dw ");  break;
    case 4:  OutFile.Put("dd ");  break;
    case 6:  OutFile.Put("df ");  break;
    case 8:  OutFile.Put("dq ");  break;
    case 10: OutFile.Put("dt ");  break;
    case 16: OutFile.Put("ddq ");  break;
    default: OutFile.Put("Error ");  break;
    }
}

void CDisassembler::WriteDataDirectiveGASM(uint32 size) {
    // Write DB, etc., GAS syntax
    // Write data definition directive for appropriate size
    switch (size) {
    case 1:  OutFile.Put(".byte  ");  break;
    case 2:  OutFile.Put(".short ");  break;
    case 4:  OutFile.Put(".int   ");  break;
    case 8:  OutFile.Put(".quad  ");  break;
    case 10: OutFile.Put(".tfloat ");  break;
    default: OutFile.Put("Error ");  break;
    }
}


void CDisassembler::WriteDataComment(uint32 ElementSize, uint32 LinePos, uint32 Pos, uint32 irel) {
    // Write comment after data item
    uint32 pos1;                            // Position of data for comment
    uint32 RelType = 0;                     // Relocation type
    char TextBuffer[64];                    // Buffer for writing floating point number

    OutFile.Tabulate(AsmTab3);              // Tabulate to comment field
    OutFile.Put(CommentSeparator);          // Start comment

    // Write address
    if (SectionEnd + SectionAddress + (uint32)ImageBase > 0xFFFF) {
        // Write 32 bit address
        OutFile.PutHex(LinePos + SectionAddress + (uint32)ImageBase);
    }
    else {
        // Write 16 bit address
        OutFile.PutHex((uint16)(LinePos + SectionAddress));
    }

    if ((Sections[Section].Type & 0xFF) == 3 || Pos > Sections[Section].InitSize) {
        // Unitialized data. Write no data
        return;
    }

    if (irel && irel < Relocations.GetNumEntries() && Relocations[irel].Offset == LinePos) {
        // Value is relocated, get relocation type
        RelType = Relocations[irel].Type;
    }

    // Space after address
    OutFile.Put(" _ ");

    // Comment type depends on ElementSize and DataType
    switch (ElementSize) {
    case 1:
        // Bytes. Write ASCII characters
        for (pos1 = LinePos; pos1 < Pos; pos1++) {
            // Get character
            int8 c = Get<int8>(pos1);
            // Avoid non-printable characters
            if (c < ' ' || c == 0x7F) c = '.';
            // Print ASCII character
            OutFile.Put(c);
        }
        break;
    case 2:
        // Words. Write as decimal
        for (pos1 = LinePos; pos1 < Pos; pos1 += 2) {
            if (RelType) {
                OutFile.PutHex(Get<uint16>(pos1), 1); // Write as hexadecimal
            }
            else {
                OutFile.PutDecimal(Get<int16>(pos1), 1);// Write as signed decimal
            }
            OutFile.Put(' ');
        }
        break;
    case 4:
        // Dwords      
        for (pos1 = LinePos; pos1 < Pos; pos1 += 4) {
            if ((DataType & 0x47) == 0x43) {
                // Write as float
                sprintf(TextBuffer, "%.8G", Get<float>(pos1));
                OutFile.Put(TextBuffer);
                // Make sure the number has a . or E to indicate a floating point number
                if (!strchr(TextBuffer,'.') && !strchr(TextBuffer,'E')) OutFile.Put(".0");
            }
            else if (((DataType + 1) & 0xFF) == 0x0C || RelType) {
                // jump/call address or offset. Write as hexadecimal
                OutFile.PutHex(Get<uint32>(pos1));
            }
            else {
                // Other. Write as decimal
                OutFile.PutDecimal(Get<int32>(pos1), 1);
            }
            OutFile.Put(' ');
        }
        break;                 
    case 8:
        // Qwords
        for (pos1 = LinePos; pos1 < Pos; pos1 += 8) {
            if ((DataType & 0x47) == 0x44) {
                // Write as double
                sprintf(TextBuffer, "%.16G", Get<double>(pos1));
                OutFile.Put(TextBuffer);
                // Make sure the number has a . or E to indicate a floating point number
                if (!strchr(TextBuffer,'.') && !strchr(TextBuffer,'E')) OutFile.Put(".0");
            }
            else {
                // Write as hexadecimal
                OutFile.PutHex(Get<uint64>(pos1));
            }
            OutFile.Put(' ');
        }
        break;
    case 10:
        // tbyte. Many compilers do not support long doubles in sprintf. Write as bytes
        for (pos1 = LinePos; pos1 < Pos; pos1++) {
            OutFile.PutHex(Get<uint8>(pos1), 1);
        }
        break;
    }
    if (RelType) {
        // Indicate relocation type
        OutFile.Put(Lookup(RelocationTypeNames, RelType));
    }
}


void CDisassembler::WriteRelocationTarget(uint32 irel, uint32 Context, int64 Addend) {
    // Write cross reference, including addend, but not including segment override and []
    // irel = index into Relocations
    // Context:
    // 1      = Data definition
    // 2      = Immediate data field in instruction
    // 4      = Data address in instruction
    // 8      = Near jump/call destination
    // 0x10   = Far  jump/call destination
    // 0x100  = Self-relative address expected
    // Addend:  inline addend
    // Implicit parameters:
    // IBegin:  value of '$' operator
    // IEnd:    reference point for self-relative addressing
    // BaseReg, IndexReg

    uint32 RefFrame;                    // Target segment
    int32  Addend2 = 0;                 // Difference between '$' and reference point

    // Get relocation type
    uint32 RelType = Relocations[irel].Type;

    if (RelType & 0x60) {
        // Inline addend is already relocated. 
        // Ignore addend and treat as direct relocation
        RelType = 1;
        Addend = 0;
    }

    // Get relocation size
    uint32 RelSize = Relocations[irel].Size;

    // Get relocation addend
    Addend += Relocations[irel].Addend;

    // Get relocation target
    uint32 Target = Relocations[irel].TargetOldIndex;

    // Is offset operand needed?
    if (Syntax != SUBTYPE_YASM && (
        ((RelType & 0xB) && (Context & 2)) 
        || ((RelType & 8) && (Context & 0x108)))) {
            // offset operator needed to convert memory operand to immediate address
            OutFile.Put("offset ");
    }

    // Is seg operand needed?
    if (RelType & 0x200) {
        // seg operator needed to convert memory operand to its segment
        OutFile.Put("seg ");
    }

    // Is explicit segment or frame needed?
    if ((RelType & 0x408) && (Context & 0x11B)) {
        // Write name of segment/group frame
        RefFrame = Relocations[irel].RefOldIndex;
        if (!RefFrame) {
            // No frame. Use segment of symbol
            RefFrame = Symbols[Symbols.Old2NewIndex(Target)].Section;
        }
        if (RefFrame && RefFrame < Sections.GetNumEntries()) {
            // Write segment or group name
            const char * SecName = NameBuffer.Buf()+Sections[RefFrame].Name;
            OutFile.Put(SecName);
            OutFile.Put(":");
        }
    }

    // Is imagerel operator needed?
    if (RelType & 4) {
        // imagerel operator needed to get image-relative address
        OutFile.Put("imagerel(");
    }

    // Adjust addend
    // Adjust offset if self-relative relocation expected and found
    if ((RelType & 2) && (Context & 0x108)) {
        // Self-relative relocation expected and found
        // Adjust by size of address field and immediate field
        Addend += IEnd - Relocations[irel].Offset;
    }
    // Subtract self-reference if unexpected self-relative relocation
    if ((RelType & 2) && !(Context & 0x108)) {
        // Self-relative relocation found but not expected
        // Fix difference between '$' and reference point
        Addend2 = Relocations[irel].Offset - IBegin;
        Addend -= Addend2;
    }
    // Add self-reference if self-relative relocation expected but not found
    if (!(RelType & 2) && (Context & 0x108)) {
        // Self-relative relocation expected but not found
        // Fix difference between '$' and reference point
        Addend += IEnd - IBegin;
    }

    if (RelType & 0x100) {
        // Target is a segment
        RefFrame = Symbols[Symbols.Old2NewIndex(Target)].Section;
        if (RefFrame && RefFrame < Sections.GetNumEntries()) {
            const char * SecName = NameBuffer.Buf()+Sections[RefFrame].Name;
            OutFile.Put(SecName);
        }
        else {
            OutFile.Put("undefined segment");
        }
    }
    else {
        // Target is a symbol

        // Find target symbol
        uint32 TargetSym = Symbols.Old2NewIndex(Target);

        // Check if Target is appropriate
        if (((Symbols[TargetSym].Type & 0x80000000) || (int32)Addend)
            && !(CodeMode == 1 && s.BaseReg)) {
                // Symbol is a start-of-section entry in symbol table, or has an addend
                // Look for a more appropriate symbol, except if code with base register
                uint32 sym, sym1, sym2 = 0;
                sym1 = Symbols.FindByAddress(Symbols[TargetSym].Section, Symbols[TargetSym].Offset + (int32)Addend, &sym2);
                for (sym = sym1; sym && sym <= sym2; sym++) {
                    if (Symbols[sym].Scope && !(Symbols[sym].Type & 0x80000000)) {
                        // Found a better symbol name for target address
                        TargetSym = sym;
                        Addend = Addend2;
                    }
                }
        }
        // Write name of target symbol
        OutFile.Put(Symbols.GetName(TargetSym));

        if (Syntax == SUBTYPE_GASM && (
            RelType == 0x41 || RelType == 0x81 || RelType == 0x2002)) {
                // make PLT entry
                OutFile.Put("@PLT");
        }
    }

    // End parenthesis if we started one
    if (RelType & 4) {
        OutFile.Put(")");
    }

    // Subtract reference point, if any
    if (RelType & 0x10) {
        OutFile.Put("-");
        // Write name of segment/group frame
        uint32 RefPoint = Relocations[irel].RefOldIndex;
        if (RefPoint) {
            // Reference point name valid
            OutFile.Put(Symbols.GetNameO(RefPoint));
        }
        else {
            OutFile.Put("Reference_Point_Missing");
        }
    }

    // Subtract self-reference if unexpected self-relative relocation
    if ((RelType & 2) && !(Context & 0x108)) {
        // Self-relative relocation found but not expected
        OutFile.Put("-"); OutFile.Put(HereOperator);
    }

    // Add self-reference if self-relative relocation expected but not found
    if (!(RelType & 2) && (Context & 0x108)) {
        // Self-relative relocation expected but not found
        OutFile.Put("+"); OutFile.Put(HereOperator);
    }

    // Write addend, if not zero
    if (Addend) {
        if (Addend < 0) {
            // Negative, write "-"
            OutFile.Put("-");
            Addend = -Addend;
        }
        else {
            // Positive, write "+"
            OutFile.Put("+");
        }

        // Write value as hexadecimal
        switch (RelSize) {
        case 1:
            OutFile.PutHex((uint8)Addend, 1);
            break;
        case 2:
            OutFile.PutHex((uint16)Addend, 2);
            break;
        case 4:
            OutFile.PutHex((uint32)Addend, 2);
            break;
        case 6:
            OutFile.PutHex((uint16)(Addend >> 32), 1);
            OutFile.Put(":");
            OutFile.PutHex((uint32)Addend, 1);
            break;
        case 8:
            OutFile.PutHex((uint64)Addend, 2);
            break;
        default:
            OutFile.Put("??"); // Should not occur
            break;
        }
    }
}


int CDisassembler::WriteFillers() {
    // Check if code is a series of NOPs or other fillers. 
    // If so then write it as filler and return 1.
    // If not, then return 0.

    // Check if code is filler
    if (!(OpcodeOptions & 0x40)) {
        // This instruction can not be used as filler
        return 0;
    }
    uint32 FillerType;                            // Type of filler
    const char * FillerName = s.OpcodeDef->Name;  // Name of filler
    uint32 IFillerBegin = IBegin;                 // Start of filling space
    uint32 IFillerEnd;                            // End of filling space

    // check for CC = int 3 breakpoint, 3C00 = 90 NOP, 11F = multibyte NOP
    if (Opcodei == 0xCC || (Opcodei & 0xFFFE) == 0x3C00 || Opcodei == 0x11F) {
        // Instruction is a NOP or int 3 breakpoint
        FillerType = Opcodei;
    }
    else if (s.Warnings1 & 0x1000000) {
        // Instruction is a LEA, MOV, etc. with same source and destination
        // used as a multi-byte NOP
        FillerType = 0xFFFFFFFF;
    }
    else {
        // This instruction does something. Not a filler
        return 0;
    }
    // Save beginning position
    IFillerEnd = IEnd = IBegin;

    // Loop through instructions to find all consecutive fillers
    while (NextInstruction2()) {

        // Parse instruction
        ParseInstruction();

        // Check if code is filler
        if (!(OpcodeOptions & 0x40)) {
            // This instruction can not be a filler
            // Save position of this instruction
            IFillerEnd = IBegin;
            break;
        }
        if (Opcodei != 0xCC && (Opcodei & 0xFFFE) != 0x3C00 && Opcodei != 0x11F
            && !(s.Warnings1 & 0x1000000)) {
                // Not a filler
                // Save position of this instruction
                IFillerEnd = IBegin;
                break;
        }
        // If loop exits here then fillers end at end of this instruction
        IFillerEnd = IEnd;
    }
    // Safety check
    if (IFillerEnd <= IFillerBegin) return 0;

    // Size of fillers
    uint32 FillerSize = IFillerEnd - IFillerBegin;

    // Write size of filling space
    OutFile.Put(CommentSeparator);
    OutFile.Put("Filling space: ");
    OutFile.PutHex(FillerSize, 2);
    OutFile.NewLine();
    // Write filler type
    OutFile.Put(CommentSeparator);
    OutFile.Put("Filler type: ");
    switch (FillerType) {
    case 0xCC:
        FillerName = "INT 3 Debug breakpoint"; break;
    case 0x3C00:
        FillerName = "NOP"; break;
    case 0x3C01:
        FillerName = "NOP with prefixes"; break;
    case 0x011F:
        FillerName = "Multi-byte NOP";break;
    }
    OutFile.Put(FillerName);
    if (FillerType == 0xFFFFFFFF) {
        OutFile.Put(" with same source and destination");
    }

    // Write as bytes
    uint32 Pos;
    for (Pos = IFillerBegin; Pos < IFillerEnd; Pos++) {
        if (((Pos - IFillerBegin) & 7) == 0) {
            // Start new line
            OutFile.NewLine();
            OutFile.Put(CommentSeparator);
            OutFile.Tabulate(AsmTab1);
            OutFile.Put(Syntax == SUBTYPE_GASM ? ".byte " : "db ");
        }
        else {
            // Continue on same line
            OutFile.Put(", ");
        }
        // Write byte value
        OutFile.PutHex(Get<uint8>(Pos), 1);
    }
    // Blank line
    OutFile.NewLine(); OutFile.NewLine();

    // Find alignment
    uint32 Alignment = 4;                         // Limit to 2^4 = 16

    // Check if first non-filler is aligned by this value
    while (Alignment && (IFillerEnd & ((1 << Alignment) - 1))) {
        // Not aligned by 2^Alignment
        Alignment--;
    }
    if (Alignment) {

        // Check if smaller alignment would do
        if (Alignment > 3 && FillerSize < 1u << (Alignment-1)) {
            // End is aligned by 16, but there are less than 8 filler bytes.
            // Change to align 8
            Alignment--;
        }
        // Write align directive
        WriteAlign(1 << Alignment);
        // Prevent writing ALIGN again
        FlagPrevious &= ~1;
    }

    // Restore IBegin and IEnd to beginning of first non-filler instruction
    IBegin = IEnd = IFillerEnd;

    if (LabelInaccessible == IFillerBegin && IFillerEnd < LabelEnd) {
        // Mark first instruction after filler as inaccessible
        LabelInaccessible = IFillerEnd;
    }

    // Return success. Fillers have been written. Don't write as normal instructions
    return 1;
}

void CDisassembler::WriteAlign(uint32 a) {
    // Write alignment directive
    OutFile.Put(Syntax == SUBTYPE_GASM ? ".ALIGN" : "ALIGN");
    OutFile.Tabulate(AsmTab1);
    OutFile.PutDecimal(a);
    OutFile.NewLine();
}

void CDisassembler::WriteFileBegin() {
    // Write begin of file

    OutFile.SetFileType(FILETYPE_ASM);

    // Initial comment
    OutFile.Put(CommentSeparator);
    OutFile.Put("Disassembly of file: ");
    OutFile.Put(cmd.InputFile);
    OutFile.NewLine();
    // Date and time. 
    // Note: will fail after year 2038 on computers that use 32-bit time_t
    time_t time1 = time(0);
    char * timestring = ctime(&time1);
    if (timestring) {
        // Remove terminating '\n' in timestring
        for (char *c = timestring; *c; c++) {
            if (*c < ' ') *c = 0;
        }
        // Write date and time as comment
        OutFile.Put(CommentSeparator);
        OutFile.Put(timestring);
        OutFile.NewLine();
    }

    // Write mode
    OutFile.Put(CommentSeparator);
    OutFile.Put("Mode: ");
    OutFile.PutDecimal(WordSize);
    OutFile.Put(" bits");
    OutFile.NewLine();

    // Write syntax dialect
    OutFile.Put(CommentSeparator);
    OutFile.Put("Syntax: ");
    switch (Syntax) {
    case SUBTYPE_MASM:
        OutFile.Put(WordSize < 64 ? "MASM/ML" : "MASM/ML64");  break;
    case SUBTYPE_YASM:
        OutFile.Put("YASM/NASM");  break;
    case SUBTYPE_GASM:
        OutFile.Put("GAS(Intel)");  break;
    }
    OutFile.NewLine();

    // Write instruction set as comment
    // Instruction set is at least .386 if 32 bit mode
    if (InstructionSetMax < 3 && (MasmOptions & 0x200)) InstructionSetMax = 3;

    // Get name of basic instruction set
    const char * set0 = "";
    if (InstructionSetMax < InstructionSetNamesLen) {
        set0 = InstructionSetNames[InstructionSetMax];
    }

    // Write as comment
    OutFile.Put(CommentSeparator);
    OutFile.Put("Instruction set: ");
    OutFile.Put(set0);

    if (InstructionSetAMDMAX) {
        // Get name of any AMD-specific instruction set
        const char * setA = "";
        switch (InstructionSetAMDMAX) {
        case 1:  setA = "AMD 3DNow";   break;
        case 2:  setA = "AMD 3DNowE";  break;
        case 4:  setA = "AMD SSE4a";   break;
        case 5:  setA = "AMD XOP";     break;
        case 6:  setA = "AMD FMA4";    break;
        case 7:  setA = "AMD TBM";   break;
        }
        if (*setA) {
            OutFile.Put(", ");
            OutFile.Put(setA);
        }
    }
    // VIA instruction set:
    if (InstructionSetOR & 0x2000) OutFile.Put(", VIA");

    // Additional instruction sets:
    if (WordSize > 32) OutFile.Put(", x64");
    if (InstructionSetOR & 0x100) OutFile.Put(", 80x87");
    if (InstructionSetOR & 0x800) OutFile.Put(", privileged instructions");
    OutFile.NewLine();

    if (NamesChanged) {
        // Tell that symbol names have been changed
        OutFile.NewLine();
        OutFile.Put(CommentSeparator);
        OutFile.Put("Error: symbol names contain illegal characters,");
        OutFile.NewLine(); OutFile.Put(CommentSeparator);
        OutFile.PutDecimal(NamesChanged);
#if ReplaceIllegalChars
        OutFile.Put(" Symbol names changed");
#else
        OutFile.Put(" Symbol names not changed");
#endif
        OutFile.NewLine();
    }

    // Write syntax-specific initializations
    switch (Syntax) {
    case SUBTYPE_MASM:
        WriteFileBeginMASM();  
        WritePublicsAndExternalsMASM();
        break;
    case SUBTYPE_YASM:
        WriteFileBeginYASM();  
        WritePublicsAndExternalsYASMGASM();
        break;
    case SUBTYPE_GASM:
        WriteFileBeginGASM();
        WritePublicsAndExternalsYASMGASM();
        break;
    }
}


void CDisassembler::WriteFileBeginMASM() {
    // Write MASM-specific file init
    if (WordSize < 64) {
        // Write instruction set directive, except for 64 bit assembler
        const char * set1 = "";
        switch (InstructionSetMax) {
        case 0:  set1 = ".8086";  break;
        case 1:  set1 = ".186";  break;
        case 2:  set1 = ".286";  break;
        case 3:  set1 = ".386";  break;
        case 4:  set1 = ".486";  break;
        case 5:  set1 = ".586";  break;
        case 6: default:
            set1 = ".686";  break;
        }
        // Write basic instruction set
        OutFile.NewLine();
        OutFile.Put(set1);
        if (InstructionSetOR & 0x800) {
            // Privileged. Add "p"
            OutFile.Put("p");
        }
        OutFile.NewLine();
        // Write extended instruction set
        if (InstructionSetOR & 0x100) {
            // Floating point
            if (InstructionSetMax < 3) {
                OutFile.Put(".8087");  OutFile.NewLine();
            }
            else if (InstructionSetMax < 5) {
                OutFile.Put(".387");  OutFile.NewLine();
            }
        }
        if (InstructionSetMax >= 0x11) {
            // .xmm directive. Not differentiated between SSE, SSE2, etc.
            OutFile.Put(".xmm");  OutFile.NewLine();
        }
        else if (InstructionSetMax >= 7) {
            // .mmx directive
            OutFile.Put(".mmx");  OutFile.NewLine();
        }
    }
    if (MasmOptions & 1) {
        // Need dotname option
        OutFile.Put("option dotname");  OutFile.NewLine();
    }
    if (WordSize == 32) {
        // Write .model flat if 32 bit mode
        OutFile.Put(".model flat");  OutFile.NewLine();
    }
    // Initialize Assumes for segment registers
    if (!(MasmOptions & 0x100)) {
        // No 16-bit segments. Assume CS=DS=ES=SS=flat
        Assumes[0]=Assumes[1]=Assumes[2]=Assumes[3] = ASM_SEGMENT_FLAT;
    }
    else {
        // 16-bit segmented model. Segment register values unknown
        Assumes[0]=Assumes[1]=Assumes[2]=Assumes[3] = ASM_SEGMENT_UNKNOWN;
    }
    // FS and GS assumed to ERROR
    Assumes[4] = Assumes[5] = ASM_SEGMENT_ERROR;

    // Write assume if FS or GS used
    // This is superfluous because an assume directive will be written at first use of FS/GS
    if (MasmOptions & 2) {
        OutFile.Put("assume fs:nothing");  OutFile.NewLine();
    }
    if (MasmOptions & 4) {
        OutFile.Put("assume gs:nothing");  OutFile.NewLine();
    }
    OutFile.NewLine();                            // Blank line
}

void CDisassembler::WriteFileBeginYASM() {
    // Write YASM-specific file init
    OutFile.NewLine();
    if (WordSize == 64) {
        OutFile.Put("default rel"); OutFile.NewLine();
    }
    //if (InstructionSetMax >= 0x11) {OutFile.Put("%define xmmword  oword");  OutFile.NewLine();}
    //if (InstructionSetMax >= 0x19) {OutFile.Put("%define ymmword");  OutFile.NewLine();}
    OutFile.NewLine();
}

void CDisassembler::WriteFileBeginGASM() {
    // Write  GAS-specific file init
    OutFile.NewLine();
    OutFile.Put(CommentSeparator);
    OutFile.Put("Note: Uses Intel syntax with destination operand first. Remember to");
    OutFile.NewLine();
    OutFile.Put(CommentSeparator);
    OutFile.Put("put syntax directives in the beginning and end of inline assembly:");
    OutFile.NewLine();
    OutFile.Put(".intel_syntax noprefix ");
    OutFile.NewLine(); OutFile.NewLine();
}

void CDisassembler::WritePublicsAndExternalsMASM() {
    // Write public and external symbol definitions
    uint32 i;                                     // Loop counter
    uint32 LinesWritten = 0;                      // Count lines written
    const char * XName;                           // Name of external symbols

    // Loop through public symbols
    for (i = 0; i < Symbols.GetNumEntries(); i++) {
        if (Symbols[i].Scope & 0x1C) {
            // Symbol is public
            OutFile.Put("public ");
            // Write name
            OutFile.Put(Symbols.GetName(i));
            // Check if weak or communal
            if (Symbols[i].Scope & 0x18) {
                // Scope is weak or communal
                OutFile.Tabulate(AsmTab3);
                OutFile.Put(CommentSeparator);
                if (Symbols[i].Scope & 8) OutFile.Put("Note: Weak. Not supported by MASM ");
                if (Symbols[i].Scope & 0x10) OutFile.Put("Note: Communal. Not supported by MASM");
            }
            OutFile.NewLine();  LinesWritten++;
        }
    }
    // Blank line if anything written
    if (LinesWritten) {
        OutFile.NewLine();
        LinesWritten = 0;
    }
    // Loop through external symbols
    for (i = 0; i < Symbols.GetNumEntries(); i++) {

        if (Symbols[i].Scope & 0x20) {
            // Symbol is external
            OutFile.Put("extern ");
            // Get name
            XName = Symbols.GetName(i);
            // Check for dynamic import
            if (Symbols[i].DLLName && strncmp(XName, Symbols.ImportTablePrefix, (uint32)strlen(Symbols.ImportTablePrefix)) == 0) {
                // Remove "_imp" prefix from name
                XName += (uint32)strlen(Symbols.ImportTablePrefix);
            }

            // Write name
            OutFile.Put(XName);
            OutFile.Put(": ");

            // Write type
            if ((Symbols[i].Type & 0xFE) == 0x84) {
                // Far
                OutFile.Put("far");
            }
            else if ((Symbols[i].Type & 0xF0) == 0x80 || Symbols[i].DLLName) {
                // Near
                OutFile.Put("near");
            }
            else {
                // Data. Write size
                switch (GetDataItemSize(Symbols[i].Type)) {
                case 1: default: OutFile.Put("byte");  break;
                case 2: OutFile.Put("word");  break;
                case 4: OutFile.Put("dword");  break;
                case 6: OutFile.Put("fword");  break;
                case 8: OutFile.Put("qword");  break;
                case 10: OutFile.Put("tbyte");  break;
                case 16: OutFile.Put("xmmword");  break;
                case 32: OutFile.Put("ymmword");  break;
                }
            }
            // Add comment if DLL import
            if (Symbols[i].DLLName) {
                OutFile.Tabulate(AsmTab3);
                OutFile.Put(CommentSeparator);
                OutFile.Put(Symbols.GetDLLName(i));
            }
            // Finished line
            OutFile.NewLine();  LinesWritten++;
        }
    }
    // Blank line if anything written
    if (LinesWritten) {
        OutFile.NewLine();
        LinesWritten = 0;
    }
    // Write the value of any constants
    // Loop through symbols
    for (i = 0; i < Symbols.GetNumEntries(); i++) {
        // Local symbols included because there might be a rip-relative address to a named constant = 0
        if (Symbols[i].Section == ASM_SEGMENT_ABSOLUTE /*&& (Symbols[i].Scope & 0x1C)*/) {
            // Symbol is constant
            // Write name
            OutFile.Put(Symbols.GetName(i));
            OutFile.Put(" equ ");
            // Write value as hexadecimal
            OutFile.PutHex(Symbols[i].Offset, 1);
            // Write decimal value as comment
            OutFile.Tabulate(AsmTab3);
            OutFile.Put(CommentSeparator);
            OutFile.PutDecimal(Symbols[i].Offset, 1);
            OutFile.NewLine();  LinesWritten++;
        }
    }
    // Blank line if anything written
    if (LinesWritten) {
        OutFile.NewLine();
        LinesWritten = 0;
    }
    // Write any group definitions
    int32 GroupId, SegmentId;
    // Loop through sections to search for group definitions
    for (GroupId = 1; GroupId < (int32)Sections.GetNumEntries(); GroupId++) {

        // Get section type
        uint32 SectionType = Sections[GroupId].Type;
        if (SectionType & 0x800) {
            // This is a segment group definition
            // Count number of members
            uint32 NumMembers = 0;
            // Write group name
            WriteSectionName(GroupId);
            // Write "group"
            OutFile.Put(" ");  OutFile.Tabulate(AsmTab1);  OutFile.Put("GROUP ");
            // Search for group members
            for (SegmentId = 1; SegmentId < (int32)Sections.GetNumEntries(); SegmentId++) {
                if (Sections[SegmentId].Group == GroupId && !(Sections[SegmentId].Type & 0x800)) {
                    // is this first member?
                    if (NumMembers++) {
                        // Not first member. Write comma
                        OutFile.Put(", ");
                    }
                    // Write group member
                    WriteSectionName(SegmentId);
                }
            }
            // End line
            OutFile.NewLine();  LinesWritten++;
        }
    }
    // Blank line if anything written
    if (LinesWritten) {
        OutFile.NewLine();
        LinesWritten = 0;
    }
}


void CDisassembler::WritePublicsAndExternalsYASMGASM() {
    // Write public and external symbol definitions, YASM and GAS syntax
    uint32 i;                                     // Loop counter
    uint32 LinesWritten = 0;                      // Count lines written
    const char * XName;                           // Name of external symbols

    // Loop through public symbols
    for (i = 0; i < Symbols.GetNumEntries(); i++) {
        if (Symbols[i].Scope & 0x1C) {
            // Symbol is public
            if (Syntax == SUBTYPE_GASM) OutFile.Put(".");
            OutFile.Put("global ");
            // Write name
            OutFile.Put(Symbols.GetName(i));

            // Write type
            if ((Symbols[i].Type & 0xF0) == 0x80) {         
                // Symbol is a function
                if (Syntax == SUBTYPE_YASM) { 
                    OutFile.Put(": function");
                }
                else if (Syntax == SUBTYPE_GASM) {
                    OutFile.NewLine();
                    OutFile.Put(".type ");
                    OutFile.Put(Symbols.GetName(i));
                    OutFile.Put(", @function");
                }
            }

            // Check if weak or communal
            if (Symbols[i].Scope & 0x18) {
                // Scope is weak or communal
                OutFile.Tabulate(AsmTab3);
                OutFile.Put(CommentSeparator);
                if (Symbols[i].Scope & 8) OutFile.Put("Note: Weak.");
                if (Symbols[i].Scope & 0x10) OutFile.Put("Note: Communal.");
            }
            OutFile.NewLine();  LinesWritten++;
        }
    }
    // Blank line if anything written
    if (LinesWritten) {
        OutFile.NewLine();
        LinesWritten = 0;
    }
    // Loop through external symbols
    for (i = 0; i < Symbols.GetNumEntries(); i++) {

        if (Symbols[i].Scope & 0x20) {
            // Symbol is external
            if (Syntax == SUBTYPE_GASM) OutFile.Put(".");
            OutFile.Put("extern ");
            // Get name
            XName = Symbols.GetName(i);
            // Check for dynamic import
            if (Symbols[i].DLLName && strncmp(XName, Symbols.ImportTablePrefix, (uint32)strlen(Symbols.ImportTablePrefix)) == 0) {
                // Remove "_imp" prefix from name
                XName += (uint32)strlen(Symbols.ImportTablePrefix);
            }
            // Write name
            OutFile.Put(XName);
            OutFile.Put(" ");
            OutFile.Tabulate(AsmTab3);
            OutFile.Put(CommentSeparator);

            // Write type
            if ((Symbols[i].Type & 0xFE) == 0x84) {
                // Far
                OutFile.Put("far");
            }
            else if ((Symbols[i].Type & 0xF0) == 0x80 || Symbols[i].DLLName) {
                // Near
                OutFile.Put("near");
            }
            else {
                // Data. Write size
                switch (GetDataItemSize(Symbols[i].Type)) {
                case 1: default: OutFile.Put("byte");  break;
                case 2: OutFile.Put("word");  break;
                case 4: OutFile.Put("dword");  break;
                case 6: OutFile.Put("fword");  break;
                case 8: OutFile.Put("qword");  break;
                case 10: OutFile.Put("tbyte");  break;
                case 16: OutFile.Put("xmmword");  break;
                case 32: OutFile.Put("ymmword");  break;
                }
            }
            // Add comment if DLL import
            if (Symbols[i].DLLName) {
                OutFile.Tabulate(AsmTab3);
                OutFile.Put(CommentSeparator);
                OutFile.Put(Symbols.GetDLLName(i));
            }
            // Finished line
            OutFile.NewLine();  LinesWritten++;
        }
    }
    // Blank line if anything written
    if (LinesWritten) {
        OutFile.NewLine();  LinesWritten = 0;
    }
    // Write the value of any constants
    // Loop through symbols
    for (i = 0; i < Symbols.GetNumEntries(); i++) {
        if (Symbols[i].Section == ASM_SEGMENT_ABSOLUTE /*&& (Symbols[i].Scope & 0x1C)*/) {
            // Symbol is constant
            if (Syntax == SUBTYPE_YASM) {
                // Write name equ value
                OutFile.Put(Symbols.GetName(i));
                OutFile.Put(" equ ");
            }
            else {
                // Gas: write .equ name, value
                OutFile.Put(".equ ");
                OutFile.Tabulate(AsmTab1);
                OutFile.Put(Symbols.GetName(i));
                OutFile.Put(", ");
            }
            // Write value as hexadecimal
            OutFile.PutHex(Symbols[i].Offset, 1);
            // Write decimal value as comment
            OutFile.Tabulate(AsmTab3);
            OutFile.Put(CommentSeparator);
            OutFile.PutDecimal(Symbols[i].Offset, 1);
            OutFile.NewLine();  LinesWritten++;
        }
    }
    // Blank line if anything written
    if (LinesWritten) {
        OutFile.NewLine();
        LinesWritten = 0;
    }
    // Write any group definitions
    int32 GroupId, SegmentId;
    // Loop through sections to search for group definitions
    for (GroupId = 1; GroupId < (int32)Sections.GetNumEntries(); GroupId++) {
        // Get section type
        uint32 SectionType = Sections[GroupId].Type;
        if (SectionType & 0x800) {
            // This is a segment group definition
            // Count number of members
            uint32 NumMembers = 0;
            // Write group name
            WriteSectionName(GroupId);
            // Write "group"
            OutFile.Put(" ");  OutFile.Tabulate(AsmTab1);  OutFile.Put("GROUP ");
            // Search for group members
            for (SegmentId = 1; SegmentId < (int32)Sections.GetNumEntries(); SegmentId++) {
                if (Sections[SegmentId].Group == GroupId && !(Sections[SegmentId].Type & 0x800)) {
                    // is this first member?
                    if (NumMembers++) {
                        // Not first member. Write comma
                        OutFile.Put(", ");
                    }
                    // Write group member
                    WriteSectionName(SegmentId);
                }
            }
            // End line
            OutFile.NewLine();  LinesWritten++;
        }
    }
    // Blank line if anything written
    if (LinesWritten) {
        OutFile.NewLine();
        LinesWritten = 0;
    }
}


void CDisassembler::WriteFileEnd() {
    // Write end of file
    OutFile.NewLine();
    switch(Syntax) {
    case SUBTYPE_MASM:
        OutFile.Put("END");  break;
    case SUBTYPE_GASM:
        OutFile.Put(CommentSeparator);
        OutFile.Put("Return to AT&T syntax with destination operand last:");
        OutFile.NewLine();
        OutFile.Put(".att_syntax prefix ");
        OutFile.NewLine();
        break;
    case SUBTYPE_YASM:
        break;
    }
}


void CDisassembler::WriteSegmentBegin() {
    // Write begin of segment
    // Choose dialect
    switch (Syntax) {
    case SUBTYPE_MASM:
        WriteSegmentBeginMASM();  break;
    case SUBTYPE_YASM:
        WriteSegmentBeginYASM();  break;
    case SUBTYPE_GASM:
        WriteSegmentBeginGASM();  break;
    }
}


void CDisassembler::WriteSegmentBeginMASM() {
    // Write begin of segment
    OutFile.NewLine();                            // Blank line

    // Check if Section is valid
    if (Section == 0 || Section >= Sections.GetNumEntries()) {
        // Illegal segment entry
        OutFile.Put("UNKNOWN SEGMENT");  OutFile.NewLine(); 
        return;
    }

    // Write segment name
    WriteSectionName(Section);
    // Tabulate
    OutFile.Put(" "); OutFile.Tabulate(AsmTab1);
    // Write "segment"
    OutFile.Put("SEGMENT ");

    // Write alignment
    switch (Sections[Section].Align) {
    case 0:  // 1
        OutFile.Put("BYTE ");  break;
    case 1:  // 2
        OutFile.Put("WORD ");  break;
    case 2:  // 4
        OutFile.Put("DWORD ");  break;
    case 4:  // 16
        OutFile.Put("PARA ");  break;
        //case 8:  // 256 or 4096. Definition is ambiguous!
        //   OutFile.Put("PAGE ");  break;
    default:
        // Non-standard alignment
        OutFile.Put("ALIGN("); 
        OutFile.PutDecimal(1 << Sections[Section].Align); 
        OutFile.Put(") "); 
        break;
    }
    if (WordSize != 64) {
        // "PUBLIC" not supported by ml64 assembler
        OutFile.Put("PUBLIC ");
        // Write segment word size if necessary
        if (MasmOptions & 0x100) {
            // There is at least one 16-bit segment. Write segment word size
            OutFile.Put("USE"); 
            OutFile.PutDecimal(Sections[Section].WordSize);
            OutFile.Put(" "); 
        }
    }
    // Write segment class
    switch (Sections[Section].Type & 0xFF) {
    case 1:
        OutFile.Put("'CODE'");   break;
    case 2:
        OutFile.Put("'DATA'");   break;
    case 3:
        OutFile.Put("'BSS'");    break;
    case 4:
        OutFile.Put("'CONST'");  break;
    default:;
        // Unknown class. Write nothing
    }

    // Tabulate to comment
    OutFile.Put(" ");  OutFile.Tabulate(AsmTab3);
    OutFile.Put(CommentSeparator);
    // Write section number
    OutFile.Put("section number ");  
    OutFile.PutDecimal(Section);

    // New line
    OutFile.NewLine();

    if (Sections[Section].Type & 0x1000) {
        // Communal
        OutFile.Put(CommentSeparator);
        OutFile.Put(" Communal section not supported by MASM");
        OutFile.NewLine();
    }

    if (WordSize == 16 && Sections[Section].Type == 1) {
        // 16 bit code segment. Write ASSUME CS: SEGMENTNAME
        OutFile.Put("ASSUME ");
        OutFile.Tabulate(AsmTab1);
        OutFile.Put("CS:");
        if (Sections[Section].Group) {
            // Group name takes precedence over segment name
            WriteSectionName(Sections[Section].Group);
        }
        else {
            WriteSectionName(Section);
        }
        OutFile.NewLine();
        Assumes[1] = Section;
    }
}

void CDisassembler::WriteSegmentBeginYASM() {
    // Write begin of segment
    OutFile.NewLine();                            // Blank line

    // Check if Section is valid
    if (Section == 0 || Section >= Sections.GetNumEntries()) {
        // Illegal segment entry
        OutFile.Put("UNKNOWN SEGMENT");  OutFile.NewLine(); 
        return;
    }

    // Write SECTION directive
    OutFile.Put("SECTION ");   
    // Write segment name
    WriteSectionName(Section);
    // Tabulate
    OutFile.Put(" ");  OutFile.Tabulate(AsmTab2);
    OutFile.Put("align=");
    OutFile.PutDecimal(1 << Sections[Section].Align);
    if (Sections[Section].WordSize != WordSize) {
        OutFile.Put(" use");
        OutFile.PutDecimal(Sections[Section].WordSize);
    }
    if ((Sections[Section].Type & 0xFF) == 1) {
        OutFile.Put(" execute");
    }
    else {
        OutFile.Put(" noexecute");
    }

    // Tabulate to comment
    OutFile.Put(" ");  OutFile.Tabulate(AsmTab3);
    OutFile.Put(CommentSeparator);
    // Write section number
    OutFile.Put("section number ");  
    OutFile.PutDecimal(Section);
    // Write type
    OutFile.Put(", ");
    switch (Sections[Section].Type & 0xFF) {
    case 1: OutFile.Put("code");  break;
    case 2: OutFile.Put("data");  break;
    case 3: OutFile.Put("bss");  break;
    case 4: OutFile.Put("const");  break;
    default: OutFile.Put("unknown type: ");  
        OutFile.PutHex(Sections[Section].Type & 0xFF);        
        break;
    }

    // New line
    OutFile.NewLine();

    if (Sections[Section].Type & 0x1000) {
        // Communal
        OutFile.Put(CommentSeparator);
        OutFile.Put(" Communal section not supported by YASM");
        OutFile.NewLine();
    }
}

void CDisassembler::WriteSegmentBeginGASM() {
    // Write begin of segment
    uint32 Type;                                  // Section type

    OutFile.NewLine();                            // Blank line

    // Check if Section is valid
    if (Section == 0 || Section >= Sections.GetNumEntries()) {
        // Illegal segment entry
        OutFile.Put("UNKNOWN SEGMENT");  OutFile.NewLine(); 
        return;
    }

    // Write SECTION directive
    OutFile.Put(".SECTION ");
    OutFile.Tabulate(AsmTab1);
    // Write segment name
    WriteSectionName(Section);
    // Tabulate
    OutFile.Put(" ");  OutFile.Tabulate(AsmTab2);
    // Flags not supported by all versions of Gas. Put as comment:
    OutFile.Put(CommentSeparator);
    // Write flags
    OutFile.Put('"');
    Type = Sections[Section].Type & 0xFF;
    if (Type) OutFile.Put('a');                   // Allocatable
    if (Type != 1 && Type != 4) OutFile.Put('w'); // Writeable
    if (Type == 1) OutFile.Put('x');              // Executable
    OutFile.Put('"');
    if (Type) OutFile.Put(", @progbits");         // Allocatable

    // Tabulate to comment
    OutFile.Put(" ");  OutFile.Tabulate(AsmTab3);
    OutFile.Put(CommentSeparator);
    // Write section number
    OutFile.Put("section number ");  
    OutFile.PutDecimal(Section);
    // Write type
    OutFile.Put(", ");
    switch (Sections[Section].Type & 0xFF) {
    case 1: OutFile.Put("code");  break;
    case 2: OutFile.Put("data");  break;
    case 3: OutFile.Put("bss");  break;
    case 4: OutFile.Put("const");  break;
    default: OutFile.Put("unknown");  break;
    }
    OutFile.NewLine();                            // Blank line
    if (Sections[Section].Type & 0x1000) {
        // Communal
        OutFile.Put(CommentSeparator);
        OutFile.Put(" Communal section ");
        OutFile.NewLine();
    }

    // Write alignment
    OutFile.Tabulate(AsmTab1);
    OutFile.Put(".ALIGN");
    OutFile.Tabulate(AsmTab2);
    OutFile.PutDecimal(1 << Sections[Section].Align);

    // New line
    OutFile.NewLine();
}


void CDisassembler::WriteSegmentEnd() {
    // Write end of segment
    OutFile.NewLine();

    if (Syntax != SUBTYPE_MASM) {
        // Not MASM syntax, write only blank line
        return;
    }

    // Check if Section is valid
    if (Section == 0 || Section >= Sections.GetNumEntries()) {
        // Illegal segment entry
        OutFile.Put("UNKNOWN ENDS");  OutFile.NewLine(); 
        return;
    }

    // Write segment name
    const char * segname = NameBuffer.Buf() + Sections[Section].Name;
    OutFile.Put(segname);

    // Tabulate
    OutFile.Put(" "); OutFile.Tabulate(AsmTab1);
    // Write "segment"
    OutFile.Put("ENDS");
    // New line
    OutFile.NewLine();
}



void CDisassembler::WriteFunctionBegin() {
    // Write begin of function IFunction

    // Check if IFunction is valid
    if (IFunction == 0 || IFunction >= FunctionList.GetNumEntries()) {
        // Should not occur
        OutFile.Put(CommentSeparator);
        OutFile.Put("Internal error: undefined function begin");
        return;
    }

    // Get symbol old index
    uint32 symi = FunctionList[IFunction].OldSymbolIndex;

    // Get symbol record
    uint32 SymI = Symbols.Old2NewIndex(symi);

    OutFile.NewLine();                        // Blank line

    // Remember that symbol has been written
    Symbols[SymI].Scope |= 0x100;

    // Check alignment if preceded by NOP
    if ((FlagPrevious & 1) && (IBegin & 0x0F) == 0 && Sections[Section].Align >= 4) {
        WriteAlign(16);
    }

    if (Symbols[SymI].Name == 0) {
        // Has no name. Probably only NOP fillers
        return;
    }

    // Write function name etc.
    switch (Syntax) {
    case SUBTYPE_MASM:
        WriteFunctionBeginMASM(SymI, Symbols[SymI].Scope);  break;
    case SUBTYPE_YASM:
        WriteFunctionBeginYASM(SymI, Symbols[SymI].Scope);  break;
    case SUBTYPE_GASM:
        WriteFunctionBeginGASM(SymI, Symbols[SymI].Scope);  break;
    }
}

void CDisassembler::WriteFunctionBeginMASM(uint32 symi, uint32 scope) {
    // Write begin of function, MASM syntax
    // Write name
    WriteSymbolName(symi);
    // Space
    OutFile.Put(" "); OutFile.Tabulate(AsmTab1);

    if (scope & 0x1C) {
        // Scope is public
        // Write "PROC"
        OutFile.Put("PROC");
        // Write "NEAR" unless 64 bit mode
        if (WordSize < 64) OutFile.Put(" NEAR");
        // Check if weak
        if (scope & 8) {
            OutFile.NewLine();
            OutFile.Put(CommentSeparator);
            OutFile.Put(" WEAK ");
            WriteSymbolName(symi);
        }
        // Check if communal
        if (scope & 0x10) {
            OutFile.NewLine();
            OutFile.Put(CommentSeparator);
            OutFile.Put(" COMDEF ");
            WriteSymbolName(symi);
        }
    }
    else {
        // Scope is local
        OutFile.Put("LABEL NEAR");
    }
    // Check if Gnu indirect
    if (Symbols[symi].Type & 0x40000000) {
        OutFile.Put(CommentSeparator);
        OutFile.Put("Gnu indirect function"); // Cannot be represented in Masm syntax
    }
    // End line
    OutFile.NewLine();
}

void CDisassembler::WriteFunctionBeginYASM(uint32 symi, uint32 scope) {
    // Write begin of function, YASM syntax
    // Write name
    WriteSymbolName(symi);
    // Colon
    OutFile.Put(":"); OutFile.Tabulate(AsmTab1);

    if (scope & 0x1C) {
        // Scope is public
        // Write comment
        OutFile.Put(CommentSeparator);
        OutFile.Put("Function begin");
        // Check if weak
        if (scope & 8) {
            OutFile.Put(", weak");
        }
        // Check if communal
        if (scope & 0x10) {
            OutFile.Put(", communal");
        }
    }
    else {
        // Scope is local. Write comment
        OutFile.Put(CommentSeparator);
        OutFile.Put("Local function");
    }
    // Check if Gnu indirect
    if (Symbols[symi].Type & 0x40000000) {
        OutFile.Put(CommentSeparator);
        OutFile.Put("Gnu indirect function"); // Cannot be represented in NASM/YASM syntax
    }
    // End line
    OutFile.NewLine();
}

void CDisassembler::WriteFunctionBeginGASM(uint32 symi, uint32 scope) {
    // Write begin of function, GAS syntax
    WriteSymbolName(symi);                        // Write name
    OutFile.Put(":"); 
    OutFile.Tabulate(AsmTab3);  OutFile.Put(CommentSeparator);
    if (scope & 3) OutFile.Put("Local ");
    if (scope & 8) OutFile.Put("weak ");
    if (scope & 0x10) OutFile.Put("communal ");
    OutFile.Put("Function");
    OutFile.NewLine();
    OutFile.Tabulate(AsmTab1);
    OutFile.Put(".type ");
    OutFile.Tabulate(AsmTab2);
    WriteSymbolName(symi); // Write name
    if (Symbols[symi].Type & 0x40000000) {
        OutFile.Put(", @gnu_indirect_function");
    }
    else {
        OutFile.Put(", @function");
    }
    OutFile.NewLine();
}


void CDisassembler::WriteFunctionEnd() {
    // Write end of function

    // Check if IFunction is valid
    if (IFunction == 0 || IFunction >= FunctionList.GetNumEntries()) {
        // Should not occur
        OutFile.Put(CommentSeparator);
        OutFile.Put("Internal error: undefined function end");
        return;
    }

    // Get symbol index
    uint32 SymOldI = FunctionList[IFunction].OldSymbolIndex;
    uint32 SymNewI = Symbols.Old2NewIndex(SymOldI);

    // check scope
    if (Symbols[SymNewI].Scope & 0x1C) {
        // Has public scope. Write end of function
        switch (Syntax) {
        case SUBTYPE_MASM:
            WriteFunctionEndMASM(SymNewI);  break;
        case SUBTYPE_YASM:
            WriteFunctionEndYASM(SymNewI);  break;
        case SUBTYPE_GASM:
            WriteFunctionEndGASM(SymNewI);  break;
        }
    }
}

void CDisassembler::WriteFunctionEndMASM(uint32 symi) {
    // Write end of function, MASM syntax
    // Write name
    WriteSymbolName(symi);

    // Space
    OutFile.Put(" "); OutFile.Tabulate(AsmTab1);
    // Write "ENDP"
    OutFile.Put("ENDP");
    OutFile.NewLine();
}

void CDisassembler::WriteFunctionEndYASM(uint32 symi) {
    // Write end of function, YASM syntax
    // Write comment
    OutFile.Put(CommentSeparator);
    // Write name
    WriteSymbolName(symi);
    OutFile.Put(" End of function");
    OutFile.NewLine();
}

void CDisassembler::WriteFunctionEndGASM(uint32 symi){
    // Write end of function, GAS syntax
    // Write .size directive
    OutFile.Tabulate(AsmTab1);
    OutFile.Put(".size ");
    OutFile.Tabulate(AsmTab2);
    WriteSymbolName(symi);                        // Name of function
    OutFile.Put(", . - ");
    WriteSymbolName(symi);                        // Name of function
    OutFile.Tabulate(AsmTab3);
    OutFile.Put(CommentSeparator);
    OutFile.Put("End of function is probably here");
    OutFile.NewLine();
}


void CDisassembler::WriteCodeLabel(uint32 symi) {
    // Write private or public code label. symi is new symbol index

    // Get scope
    uint32 Scope = Symbols[symi].Scope;

    // Check scope
    if (Scope & 0x100) return;                    // Has been written as function begin

    if (Scope == 0) {
        // Inaccessible. No name. Make blank line
        OutFile.NewLine();
        // Remember position for warning check
        LabelInaccessible = IBegin;
        return;
    }

    // Begin on new line if preceded by another symbol
    if (OutFile.GetColumn()) OutFile.NewLine(); 

    // Check alignment if preceded by NOP
    if ((Scope & 0xFF) > 1 && (FlagPrevious & 1) && (IBegin & 0x0F) == 0 && Sections[Section].Align >= 4) {
        WriteAlign(16);
    }

    switch (Syntax) {
    case SUBTYPE_MASM:
        WriteCodeLabelMASM(symi, Symbols[symi].Scope);  break;
    case SUBTYPE_YASM:
        WriteCodeLabelYASM(symi, Symbols[symi].Scope);  break;
    case SUBTYPE_GASM:
        WriteCodeLabelGASM(symi, Symbols[symi].Scope);  break;
    }

    // Remember this has been written
    Symbols[symi].Scope |= 0x100;
}


void CDisassembler::WriteCodeLabelMASM(uint32 symi, uint32 scope) {
    // Write private or public code label, MASM syntax
    if ((scope & 0xFF) > 1) {
        // Scope > function local. Write as label near
        // Check if extra linefeed needed
        // if (!(IFunction && FunctionList[IFunction].Start == IBegin)) 
        // New line
        OutFile.NewLine();

        // Write name
        WriteSymbolName(symi);
        // Space
        OutFile.Put(" "); OutFile.Tabulate(AsmTab1);
        // Write "LABEL"
        OutFile.Put("LABEL");
        // Write "NEAR" even 64 bit mode
        OutFile.Put(" NEAR");
        // New line
        OutFile.NewLine();

        // Check if weak
        if (scope & 8) {
            OutFile.Put(CommentSeparator);
            OutFile.Put(" WEAK ");
            WriteSymbolName(symi);
            OutFile.NewLine();
        }
        // Check if communal
        if (scope & 0x10) {
            OutFile.Put(CommentSeparator);
            OutFile.Put(" COMDEF ");
            WriteSymbolName(symi);
            OutFile.NewLine();
        }
    }
    else {
        // Symbol is local to current function. Write name with colon
        if (FlagPrevious & 2) {
            // Insert blank line if previous instruction was unconditional jump or return
            OutFile.NewLine();
        }
        // Write name
        WriteSymbolName(symi);
        // Write ":"
        OutFile.Put(":");
        if (OutFile.GetColumn() > AsmTab1) {
            // Past tabstop. Go to next line
            OutFile.NewLine();                   // New line
        }
    }
}

void CDisassembler::WriteCodeLabelYASM(uint32 symi, uint32 scope) {
    // Write private or public code label, YASM syntax
    if ((scope & 0xFF) > 2) {
        // Scope is public
        OutFile.NewLine();
        // Write name
        WriteSymbolName(symi);
        OutFile.Put(":");

        // Check if weak
        if (scope & 8) {
            OutFile.Put(CommentSeparator);
            OutFile.Put(" weak ");
            WriteSymbolName(symi);
        }
        // Check if communal
        if (scope & 0x10) {
            OutFile.Put(CommentSeparator);
            OutFile.Put(" communal ");
            WriteSymbolName(symi);
        }
        OutFile.NewLine();
    }
    else {
        // Symbol is local to current function. Write name with colon
        if (FlagPrevious & 2) {
            // Insert blank line if previous instruction was unconditional jump or return
            OutFile.NewLine();
        }
        // Write name
        WriteSymbolName(symi);
        // Write ":"
        OutFile.Put(":");
        if (OutFile.GetColumn() > AsmTab1) {
            // Past tabstop. Go to next line
            OutFile.NewLine();                   // New line
        }
    }
}

void CDisassembler::WriteCodeLabelGASM(uint32 symi, uint32 scope) {
    // Write private or public code label, GAS syntax same as YASM syntax
    WriteCodeLabelYASM(symi, scope);
}

void CDisassembler::WriteAssume() {
    // Write assume directive for segment register if MASM syntax
    if (Syntax != SUBTYPE_MASM) return;
    if (!s.AddressField) return;

    int32 SegReg, PrefixSeg;                      // Segment register used
    uint32 symo;                                  // Target symbol old index
    uint32 symi;                                  // Target symbol new index
    int32 TargetSegment;                          // Target segment/section
    int32 TargetGroup;                            // Group containing target segment

    // Find which segment register is used for addressing memory operand
    SegReg = 3;  // DS is default
    if (s.BaseReg == 4+1 || s.BaseReg == 5+1) {
        // Base register is (E)BP or ESP
        SegReg = 2;     // SS register used unless there is a prefix
    }
    if (s.Prefixes[0]) {
        // There is a segment prefix
        PrefixSeg = GetSegmentRegisterFromPrefix();
        if (PrefixSeg >= 0 && PrefixSeg <= 5) {
            // Segment prefix is valid. Segment determined by segment prefix
            SegReg = PrefixSeg;
        }
    }
    // Default target segment is none
    TargetSegment = TargetGroup = 0;

    // Find symbol referenced by next instruction
    if (s.AddressRelocation && s.AddressRelocation < Relocations.GetNumEntries()) {
        symo = Relocations[s.AddressRelocation].TargetOldIndex; // Target symbol old index
        if (symo) {
            symi = Symbols.Old2NewIndex(symo);                   // Target symbol new index
            if (symi) {
                TargetSegment = Symbols[symi].Section;            // Target segment
                if (TargetSegment < 0 || TargetSegment >= (int32)Sections.GetNumEntries()) {
                    TargetSegment = 0;
                }
                else {
                    TargetGroup = Sections[TargetSegment].Group;   // Group containing target segment
                    if (TargetGroup <= ASM_SEGMENT_ERROR || TargetGroup >= (int32)Sections.GetNumEntries()) {
                        TargetGroup = 0;
                    }
                }
            }
        }
    }
    if (TargetSegment) {
        // Target has a segment. Check if it is different from currently assumed segment
        if (TargetSegment != Assumes[SegReg] && TargetGroup != Assumes[SegReg]) {
            // Assume directive needed
            // If segment belongs to a group then the group takes precedence
            if (TargetGroup) TargetSegment = TargetGroup;
            // Write assume directive
            OutFile.Put("ASSUME ");
            OutFile.Tabulate(AsmTab1);
            OutFile.Put(RegisterNamesSeg[SegReg]);  // Name of segment register used
            OutFile.Put(":");
            WriteSectionName(TargetSegment);        // Name of segment or group referenced
            OutFile.NewLine();
            Assumes[SegReg] = TargetSegment;
        }
    }
    else {
        // Target segment not specified. Assumed value may be anyting but 'error'
        if (Assumes[SegReg] <= ASM_SEGMENT_ERROR) {
            // Segment register is assumed to 'error'. Change assume to 'nothing'
            OutFile.Put("ASSUME ");
            OutFile.Tabulate(AsmTab1);
            OutFile.Put(RegisterNamesSeg[SegReg]);  // Name of segment register used
            OutFile.Put(":NOTHING");
            OutFile.NewLine();
            Assumes[SegReg] = ASM_SEGMENT_NOTHING;
        }
    }
}


void CDisassembler::WriteInstruction() {
    // Write instruction and operands
    uint32 NumOperands = 0;                       // Number of operands written
    uint32 i;                                     // Loop index
    const char * OpName;                          // Opcode name

    if (s.AddressFieldSize && Syntax == SUBTYPE_MASM) {
        // There is a memory operand. Check if ASSUME directive needed
        WriteAssume();
    }

    if (CodeMode & 6) {
        // Code is dubious. Show as comment only
        OutFile.Put(CommentSeparator);             // Start comment
    }
    else if ((s.OpcodeDef->Options & 0x20) && s.OpcodeStart1 > IBegin) {
        // Write prefixes explicitly. 
        // This is used for rare cases where the assembler cannot generate the prefix
        OutFile.Tabulate(AsmTab1);                 // Tabulate
        OutFile.Put(Syntax == SUBTYPE_GASM ? ".byte " : "DB ");
        OutFile.Tabulate(AsmTab2);                 // Tabulate
        for (i = IBegin; i < s.OpcodeStart1; i++) {
            if (i > IBegin) OutFile.Put(", ");
            OutFile.PutHex(Get<uint8>(i), 1);
        }
        OutFile.Tabulate(AsmTab3);                 // Tabulate
        OutFile.Put(CommentSeparator);
        if ((s.OpcodeDef->AllowedPrefixes & 8) && Get<uint8>(IBegin) == 0xF2) {
            OutFile.Put("BND prefix coded explicitly");    // Comment
        }
        else {        
            OutFile.Put("Prefix coded explicitly");    // Comment
        }
        OutFile.NewLine();
    }

    if ((s.Operands[0] & 0xF0) == 0xC0 || (s.Operands[1] & 0xF0) == 0xC0) {
        // String instruction or xlat instruction
        WriteStringInstruction();
        return;
    }

    OutFile.Tabulate(AsmTab1);                     // Tabulate

    if ((s.OpcodeDef->AllowedPrefixes & 0xC40) == 0xC40) {
        switch (s.Prefixes[5]) {
        case 0xF2:
            OutFile.Put("xacquire ");  break;      // xacquire prefix
        case 0xF3:
            OutFile.Put("xrelease ");  break;      // xrelease prefix
        }
    }
    if (s.Prefixes[2]) {
        OutFile.Put("lock ");                      // Lock prefix
    }

    // Get opcode name
    if (s.OpcodeDef->Name) {
        // Opcode name
        OpName = s.OpcodeDef->Name;
        // Search for opcode comment
        s.OpComment = strchr(OpName, ';');
        if (s.OpComment) s.OpComment++;            // Point to after ';'
    }
    else {
        OpName = "UNDEFINED";                      // Undefined code with no name
        s.OpComment = 0;
    }

    // Check prefix option
    if ((s.OpcodeDef->Options & 2) && (s.Prefixes[7] & 0x30)) {
        // Put prefix 'v' for VEX-prefixed instruction
        OutFile.Put('v');
    }

    // Write opcode name
    if (s.OpComment) {
        // OpName string contains opcode name and comment, separated by ';'
        while (*OpName != ';' && *OpName != 0) {   // Write opcode name until comment
            OutFile.Put(*(OpName++));
        }
    }
    else {
        OutFile.Put(OpName);                       // Write normal opcode name
    }

    // Check suffix option
    if (s.OpcodeDef->Options & 1) {
        // Append suffix for operand size or type to name
        if ((s.OpcodeDef->AllowedPrefixes & 0x7000) == 0x1000) {
            // F.P. operand size defined by W prefix bit
            i = s.Prefixes[7] & 8;  // W prefix bit
            OutFile.Put(i ? 'd' : 's');
        }
        else if ((s.OpcodeDef->AllowedPrefixes & 0x7000) == 0x3000) {
            // Integer or f.p. operand size defined by W prefix bit
            bool f = false;
            // Find out if operands are integer or f.p.
            for (i = 0; i < s.MaxNumOperands; i++) {
                if ((s.Operands[i] & 0xF0) == 0x40) {
                    f = true; break;
                }
            }
            i = s.Prefixes[7] & 8;  // W prefix bit
            if (f) {
                OutFile.Put(i ? 'd' : 's');  // float precision suffix
            }
            else {            
                OutFile.Put(i ? 'q' : 'd');  // integer size suffix
            }
        }
        else if ((s.OpcodeDef->AllowedPrefixes & 0x7000) == 0x4000) {
            // Integer operand size defined by W prefix bit
            i = s.Prefixes[7] & 8;  // W prefix bit
            OutFile.Put(i ? 'w' : 'b');
        }
        else if ((s.OpcodeDef->AllowedPrefixes & 0x7000) == 0x5000) {
            // mask register operand size defined by W prefix bit and 66 prefix
            i  = (s.Prefixes[7] & 8) >> 2;      // W prefix bit
            i |= s.Prefixes[5] != 0x66;         // 66 prefix bit
            OutFile.Put("bwdq"[i]);
        }
        else if (s.OpcodeDef->AllowedPrefixes & 0xE00) {
            // F.P. operand type and size defined by prefixes
            switch (s.Prefixes[5]) {
            case 0:     // No prefix = ps
                OutFile.Put("ps");  break;
            case 0x66:  // 66 prefix = pd
                OutFile.Put("pd");  break;
            case 0xF3:  // F3 prefix = ss
                OutFile.Put("ss");  break;
            case 0xF2:  // F2 prefix = sd
                OutFile.Put("sd");  break;
            default:
                err.submit(9000); // Should not occur
            }
        }
        else if (s.OpcodeDef->AllowedPrefixes & 0x100){
            // Integer operand size defined by prefixes
            // Suffix for operand size
            i = s.OperandSize / 8;
            if (i <= 8) {
                static const char SizeSuffixes[] = " bw d f q"; // Table of suffixes
                OutFile.Put(SizeSuffixes[i]);
            }
        }
    }
    // Alternative suffix option
    if (s.OpcodeDef->Options & 0x1000) {
        // Append alternative suffix for vector element size to name
        if ((s.OpcodeDef->AllowedPrefixes & 0x7000) == 0x3000) {
            // Integer operand size defined by W prefix bit
            i = ((s.Prefixes[7] & 8) + 8) * 4;  // W prefix bit -> 8 / 16
            OutFile.PutDecimal(i);
        }
        if ((s.OpcodeDef->AllowedPrefixes & 0x7000) == 0x4000) { // 32 / 64
            i = (s.Prefixes[7] & 8) + 8;  // W prefix bit -> 8 / 16
            OutFile.PutDecimal(i);
        }
    }
    // More suffix option
    if ((s.OpcodeDef->Options & 0x400) && s.ImmediateFieldSize == 8) {
        // 64 bit immediate mov
        if (Syntax == SUBTYPE_GASM) OutFile.Put("abs");
    }   

    // Space between opcode name and operands
    OutFile.Put(" "); OutFile.Tabulate(AsmTab2);  // Tabulate. At least one space

    // Loop for all operands to write
    for (i = 0; i < s.MaxNumOperands; i++) {
        if (s.Operands[i] & 0xFFFF) {

            // Write operand i
            if (NumOperands++) {
                // At least one operand before this one. Separate by ", "
                OutFile.Put(", ");
            }

            // Write constant and jump operands
            switch (s.Operands[i] & 0xF0) {
            case 0x10: case 0x20: case 0x30: case 0x80:
                WriteImmediateOperand(s.Operands[i]);
                continue;
            }

            // Write register and memory operands
            uint32 optype = (s.Operands[i] >> 16) & 0x0F;
            switch (optype) {
            case 0:        // Other type of operand
                WriteOtherOperand(s.Operands[i]);  break;

            case 0x1:  // Direct memory operand
                WriteRMOperand(s.Operands[i]);  break;

            case 0x2:  // Register operand indicated by last bits of opcode
                WriteShortRegOperand(s.Operands[i]);  break;

            case 0x3:  // Register or memory operand indicated by mod/rm bits
                WriteRMOperand(s.Operands[i]);  break;

            case 0x4:  // Register operand indicated by reg bits
                WriteRegOperand(s.Operands[i]);  break;

            case 0x5:  // Register operand indicated by dest bits of DREX byte
                WriteDREXOperand(s.Operands[i]);  break;

            case 0x6:  // Register operand indicated by VEX.vvvv bits
                WriteVEXOperand(s.Operands[i], 0);  break;

            case 0x7:  // Register operand indicated by bits 4-7 of immediate operand
                WriteVEXOperand(s.Operands[i], 1);  break;

            case 0x8:  // Register operand indicated by bits 0-3 of immediate operand
                WriteVEXOperand(s.Operands[i], 2);  break; // Unused. For future use
            }
            int isMem = optype == 3 && s.Mod != 3;
            if (s.Prefixes[3] == 0x62) { // EVEX and MVEX prefix can have extra operand attributes
                if (s.Prefixes[6] & 0x20) {                
                    WriteOperandAttributeEVEX(i, isMem);
                }
                else {
                    WriteOperandAttributeMVEX(i, isMem);
                }
            }
            if (s.Prefixes[3] == 0x62 && (i == s.MaxNumOperands - 1 || (s.Operands[i+1] & 0xFFF) < 0x40)) {
                // This is the last SIMD operand
                if (!(s.Operands[4] & 0x80000000)) {
                    s.Operands[4] |= 0x80000000;  // Make sure we don't write this twice
                    if (s.Prefixes[6] & 0x20) {                
                        WriteOperandAttributeEVEX(98, isMem);
                    }
                    else {
                        WriteOperandAttributeMVEX(98, isMem);
                    }
                }
            }
        }
    }
    if (s.Prefixes[3] == 0x62) { // EVEX and MVEX prefix can have extra attributes after operands
        if (s.Prefixes[6] & 0x20) {                
            WriteOperandAttributeEVEX(99, 0);
        }
        else {
            WriteOperandAttributeMVEX(99, 0);
        }
    }
    if (s.OpComment) {
        // Write opcode comment
        OutFile.Put(' ');
        OutFile.Put(CommentSeparator);
        OutFile.Put(s.OpComment);
    }
}


void CDisassembler::WriteStringInstruction() {
    // Write string instruction or xlat instruction
    uint32 NumOperands = 0;                       // Number of operands written
    uint32 i;                                     // Loop index
    uint32 Segment;                               // Possible segment prefix

    if (!(s.OpcodeDef->AllowedPrefixes & 0x1100)) {
        // Operand size is 8 if operand size prefixes not allowed
        s.OperandSize = 8;
    }

    OutFile.Tabulate(AsmTab1);                    // Tabulate

    if (Syntax != SUBTYPE_MASM && s.Prefixes[0] && (s.OpcodeDef->AllowedPrefixes & 4)) {
        // Get segment prefix
        Segment = GetSegmentRegisterFromPrefix();  // Interpret segment prefix
        // Write segment override
        OutFile.Put(RegisterNamesSeg[Segment]);
        OutFile.Put(" ");
    }     

    // Check repeat prefix
    if (s.OpcodeDef->AllowedPrefixes & 0x20) {
        if (s.Prefixes[3]) {
            // Repeat prefix
            OutFile.Put("rep ");
        }
    }
    else if (s.OpcodeDef->AllowedPrefixes & 0x40) {
        if (s.Prefixes[3] == 0xF2) {
            // repne prefix
            OutFile.Put("repne ");
        }
        else if (s.Prefixes[3] == 0xF3) {
            // repe prefix
            OutFile.Put("repe ");
        }
    }

    // Write opcode name
    OutFile.Put(s.OpcodeDef->Name);               // Opcode name

    if (Syntax == SUBTYPE_MASM
        && (((s.OpcodeDef->AllowedPrefixes & 4) && s.Prefixes[0]) 
        || ((s.OpcodeDef->AllowedPrefixes & 1) && s.Prefixes[1]))) {
            // Has segment or address size prefix. Must write operands explicitly
            OutFile.Put(" ");                          // Space before operands

            // Check address size for pointer registers
            const char * * PointerRegisterNames;
            switch (s.AddressSize) {
            case 16:
                PointerRegisterNames = RegisterNames16;  break;
            case 32:
                PointerRegisterNames = RegisterNames32;  break;
            case 64:
                PointerRegisterNames = RegisterNames64;  break;
            default: 
                PointerRegisterNames = 0;  // should not occur
            }

            // Loop for possibly two operands
            for (i = 0; i < 2; i++) {
                if (s.Operands[i]) {
                    // Operand i defined
                    if (NumOperands++) {
                        // An operand before this one. Separate by ", "
                        OutFile.Put(", ");
                    }
                    if (NumOperands == 1) {
                        // Write operand size for first operand
                        switch (s.OperandSize) {
                        case 8:
                            OutFile.Put("byte  ");  break;
                        case 16:
                            OutFile.Put("word  ");  break;
                        case 32:
                            OutFile.Put("dword  ");  break;
                        case 64:
                            OutFile.Put("qword  ");  break;
                        }
                    }
                    // Get segment
                    Segment = 1;                        // Default segment is DS
                    if (s.Prefixes[0]) {
                        Segment = GetSegmentRegisterFromPrefix(); // Interpret segment prefix
                    }
                    if ((s.Operands[i] & 0xCF) == 0xC2) {
                        Segment = 0;                      // Segment is ES regardless of prefix for [edi] operand
                    }
                    // Write segment override
                    OutFile.Put(RegisterNamesSeg[Segment]);
                    OutFile.Put(":");
                    // Opening "["
                    OutFile.Put("[");

                    // Write pointer register
                    switch (s.Operands[i] & 0xCF) {
                    case 0xC0:  // [bx], [ebx] or [rbx]
                        OutFile.Put(PointerRegisterNames[3]);  
                        break;
                    case 0xC1:  // [si], [esi] or [rsi]
                        OutFile.Put(PointerRegisterNames[6]);  
                        break;
                    case 0xC2:  // [di], [edi] or [rdi]
                        OutFile.Put(PointerRegisterNames[7]);  
                        break;
                    }
                    // Closing "]"
                    OutFile.Put("]");
                }
            }
    }
    else {
        // We don't have to write the operands
        // Append suffix for operand size, except for xlat
        if ((s.Operands[1] & 0xCF) != 0xC0) {

            // Suffix for operand size
            uint32 i = s.OperandSize / 8;
            if (i <= 8) {
                static const char SizeSuffixes[] = " bw d   q"; // Table of suffixes
                OutFile.Put(SizeSuffixes[i]);
            }
        }
    }
}


void CDisassembler::WriteCodeComment() {
    // Write hex listing of instruction as comment after instruction
    uint32 i;                                     // Index to current byte
    uint32 FieldSize;                             // Number of bytes in field
    const char * Spacer;                          // Space between fields

    OutFile.Tabulate(AsmTab3);                    // Tabulate to comment field
    OutFile.Put(CommentSeparator);                // Start comment

    // Write address
    if (SectionEnd + SectionAddress + (uint32)ImageBase > 0xFFFF) {
        // Write 32 bit address
        OutFile.PutHex(IBegin + SectionAddress + (uint32)ImageBase);
    }
    else {
        // Write 16 bit address
        OutFile.PutHex((uint16)(IBegin + SectionAddress));
    }

    // Space after address
    OutFile.Put(" _");

    // Start of instruction
    i = IBegin;

    // Write bytes
    while (i < IEnd) {
        FieldSize = 1;                             // Size of field to write
        Spacer = " ";                              // Space between fields

        // Spacer and FieldSize depends on fields
        if (i == s.OpcodeStart1 && i > IBegin) {
            Spacer = ": ";                          // Space between prefixes and opcode
        }
        if (i == s.OpcodeStart2 + 1) {
            Spacer = ". ";                          // Space between opcode and mod/reg/rm bytes
        }
        if (i == s.AddressField && s.AddressFieldSize) {
            Spacer = ", ";                          // Space before address field
            FieldSize = s.AddressFieldSize;
        }
        if (i == s.ImmediateField && s.ImmediateFieldSize) {
            Spacer = ", ";                          // Space before immediate operand field
            FieldSize = s.ImmediateFieldSize;
        }
        // Write space
        OutFile.Put(Spacer);

        // Write byte or bytes
        switch (FieldSize) {
        case 1:  // Write single byte
            OutFile.PutHex(Get<uint8>(i));
            break;
        case 2:  // Write two bytes
            OutFile.PutHex(Get<uint16>(i));
            break;
        case 3:  // Write three bytes (operands for "enter" instruction)
            OutFile.PutHex(Get<uint16>(i));
            OutFile.Put(", ");
            OutFile.PutHex(Get<uint8>(i+2));
            break;
        case 4:  // Write four bytes
            if ((s.Operands[0] & 0xFE) == 0x84) {
                // Far jump/call address
                OutFile.PutHex(Get<uint16>(i));
                OutFile.Put(" ");
                OutFile.PutHex(Get<uint16>(i+2));
            }
            else {
                // Any other 32 bit operand
                OutFile.PutHex(Get<uint32>(i));
            }
            break;
        case 6:  // Write six bytes (far jump address)
            OutFile.PutHex(Get<uint32>(i));
            OutFile.Put(" ");
            OutFile.PutHex(Get<uint16>(i+4));
            break;
        case 8:  // Write eight bytes
            OutFile.PutHex(Get<uint64>(i));
            break;
        }
        // Search for relocation
        SARelocation rel1;                            // Make relocation records for searching
        rel1.Section = Section;
        rel1.Offset  = i;                             // rel1 marks current field in instruction

        // Is there a relocation source exactly here?
        int32 irel = Relocations.Exists(rel1);        // Finds relocation with source = i

        if (irel > 0) {
            // This field has a relocation. Indicate relocation type
            // 0 = unknown, 1 = direct, 2 = self-relative, 3 = image-relative, 
            // 4 = segment relative, 5 = relative to arbitrary ref. point, 8 = segment address/descriptor
            uint32 RelType = Relocations[irel].Type;
            if (RelType) {
                OutFile.Put(Lookup(RelocationTypeNames, RelType));
            }
            if (Relocations[irel].Size > FieldSize) {
                // Relocation has wrong size
                OutFile.Put(" Misplaced relocation.");
            }
        }

        // Point to next byte
        i += FieldSize;
    }
    // New line
    OutFile.NewLine();
}


void CDisassembler::CountInstructions() {
    // Count total number of instructions defined in opcodes.cpp
    // Two instructions are regarded as the same and counted as one if they 
    // have the same name and differ only in the bits that define register 
    // name, operand size, etc.

    uint32 map;                                   // Map number
    uint32 index;                                 // Index into map
    uint32 n;                                     // Number of instructions with same code
    uint32 iset;                                  // Instruction set
    uint32 instructions = 0;                      // Total number of instructions
    uint32 mmxinstr = 0;                          // Number of MMX instructions
    uint32 sseinstr = 0;                          // Number of SSE instructions
    uint32 sse2instr = 0;                         // Number of SSE2 instructions
    uint32 sse3instr = 0;                         // Number of SSE3 instructions
    uint32 ssse3instr = 0;                        // Number of SSSE3 instructions
    uint32 sse41instr = 0;                        // Number of SSE4.1 instructions
    uint32 sse42instr = 0;                        // Number of SSE4.2 instructions
    uint32 AVXinstr  = 0;                         // Number of AVX instructions
    uint32 FMAinstr  = 0;                         // Number of FMA3 and later instructions
    uint32 AVX2instr  = 0;                        // Number of AVX2 instructions
    uint32 BMIinstr  = 0;                         // Number of BMI instructions and other small instruction sets
    uint32 AVX512instr = 0;                       // Number of AVX-512 instructions
    uint32 MICinstr = 0;                          // Number of MIC instructions
    uint32 AMDinstr = 0;                          // Number of AMD instructions
    uint32 VIAinstr = 0;                          // Number of AMD instructions
    uint32 privilinstr = 0;                       // Number of privileged instructions
    uint32 undocinstr = 0;                        // Number of undocumented instructions
    uint32 droppedinstr = 0;                      // Number of opcodes planned but never implemented
    uint32 VEXdouble = 0;                         // Number of instructions that have both VEX and non-VEX version
    SOpcodeDef const * opcode;                    // Pointer to map entry

    // Loop through all maps
    for (map = 0; map < NumOpcodeTables1; map++) {
        // Loop through each map
        for (index = 0; index < OpcodeTableLength[map]; index++) {
            opcode = OpcodeTables[map] + index;
            if (opcode->InstructionFormat && opcode->Name 
            && !opcode->TableLink && !(opcode->InstructionFormat & 0x8000)) {
                // instruction is defined
                if ((opcode->InstructionFormat & 0xFFF) == 3
                && index > 0 && (opcode-1)->Name 
                && strcmp(opcode->Name, (opcode-1)->Name) == 0) {
                    // Same as previous instruction, just with another register
                    continue;                         // Don't count this
                }
                n = 1;                               // Default = one instruction per map entry
                // Check if we have multiple instructions with different prefixes
                if (opcode->Options & 1) {
                    if (opcode->AllowedPrefixes & 0x3000) {
                        n++;                           // Extra instruction with W prefix bit
                    }
                    else if (opcode->AllowedPrefixes & 0xE00) {               
                        if (opcode->AllowedPrefixes & 0x200) n++; // Extra instruction with 66 prefix
                        if (opcode->AllowedPrefixes & 0x400) n++; // Extra instruction with F3 prefix
                        if (opcode->AllowedPrefixes & 0x800) n++; // Extra instruction with F2 prefix
                    }
                    else if (opcode->AllowedPrefixes & 0x100) {
                        n++;                                      // Extra instruction with 66 prefix
                        if (opcode->AllowedPrefixes & 0x1000) n++;// Extra instruction with L prefix bit
                    }
                }
                if (opcode->Options & 2) VEXdouble += n; // Instructions that have both VEX and non-VEX version
                instructions += n;                   // Count total instructions

                iset = opcode->InstructionSet;       // Instruction set
                if (iset & 0x20000) {
                    droppedinstr += n; iset = 0;      // Opcodes planned but never implemented
                }
                if (iset & 0x800) privilinstr += n;  // Privileged instruction
                if (opcode->InstructionFormat & 0x4000) undocinstr += n; // Undocumented instruction

                switch (iset & 0x37FF) {
                case 7:  // MMX
                    mmxinstr += n;  break;
                case 0x11:  // SSE
                    sseinstr += n;  break;
                case 0x12:  // SSE2
                    sse2instr += n;  break;
                case 0x13: // SSE3
                    sse3instr += n;  break;
                case 0x14: // SSSE3
                    ssse3instr += n;  break;
                case 0x15: // SSE4.1
                    sse41instr += n;  break;
                case 0x16: // SSE4.2
                    sse42instr += n;  break;
                case 0x17: case 0x18: case 0x19: // VEX etc.
                    AVXinstr += n;  break;
                case 0x1A: case 0x1B:            // FMA and later instructions
                    FMAinstr += n;  break;
                case 0x1C:                       // AVX2 instructions
                    AVX2instr += n; break;
                case 0x1D: case 0x1E:            // BMI and other small instruction sets
                    BMIinstr += n; break;
                case 0x20:                       // AVX-512 instructions
                    AVX512instr += n; break;
                case 0x80:                       // MIC instructions
                    MICinstr += n; break;
                case 0x1001: case 0x1002: case 0x1004: case 0x1005: case 0x1006:  // AMD
                    AMDinstr += n;  break;
                case 0x2001: // VIA
                    VIAinstr += n;  break;               
                }
            }
        }
    }

    // output result
    printf("\n\nNumber of instruction opcodes supported by disassembler:\n%5i Total, including:", 
        instructions);
    printf("\n%5i Privileged instructions", privilinstr);
    printf("\n%5i MMX    instructions", mmxinstr);
    printf("\n%5i SSE    instructions", sseinstr);
    printf("\n%5i SSE2   instructions", sse2instr);
    printf("\n%5i SSE3   instructions", sse3instr);
    printf("\n%5i SSSE3  instructions", ssse3instr);
    printf("\n%5i SSE4.1 instructions", sse41instr);
    printf("\n%5i SSE4.2 instructions", sse42instr);
    printf("\n%5i AVX    instructions etc.", AVXinstr);
    printf("\n%5i AVX2   instructions", AVX2instr);
    printf("\n%5i FMA3   instructions", FMAinstr);
    printf("\n%5i BMI/micsellaneous instr.", BMIinstr);
    printf("\n%5i AVX-512 instructions", AVX512instr);
    printf("\n%5i MIC/Xeon Phi instructions", MICinstr);
    printf("\n%5i AMD    instructions", AMDinstr);
    printf("\n%5i VIA    instructions", VIAinstr);   
    printf("\n%5i instructions planned but never implemented in any CPU", droppedinstr);
    printf("\n%5i undocumented or illegal instructions", undocinstr);
    printf("\n%5i instructions have both VEX and non-VEX versions", VEXdouble);
    printf("\n");

#if 0   // temporary test code

    // find entries with 0x2000 prefix code
    printf("\n\nInstructions with operand swap flag:\n");
    // Loop through all maps
    for (map = 0; map < NumOpcodeTables1; map++) {
        // Loop through each map
        for (index = 0; index < OpcodeTableLength[map]; index++) {
            opcode = OpcodeTables[map] + index;
            if ((opcode->AllowedPrefixes & 0x2000) == 0x2000) {
                printf("\n%04X %02X  %s", map, index, opcode->Name);
            }
        }
    }

    /*
    printf("\n\nTables linked by type 0x0E:\n");
    // Loop through all maps
    for (map = 0; map < NumOpcodeTables1; map++) {
        // Loop through each map
        for (index = 0; index < OpcodeTableLength[map]; index++) {
            opcode = OpcodeTables[map] + index;
            if (opcode->TableLink == 0x0E) {
                printf("  0x%02X", opcode->InstructionSet);
            }
        }
    }*/

    printf("\n");

#endif
}
