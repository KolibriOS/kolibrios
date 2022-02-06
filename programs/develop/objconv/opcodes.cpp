/****************************    opcodes.cpp    *******************************
* Author:        Agner Fog
* Date created:  2007-02-21 
* Last modified: 2018-10-08
* Project:       objconv
* Module:        opcodes.cpp
* Description:
* Definition of opcode maps used by disassembler
*
* Copyright 2007-2018 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/


/*************************** Define opcode maps ******************************

Each line in the tables defines an instruction. 
Name is the name of the instruction, possibly without suffix for operand size etc.
Instset defines which instruction set is required.
Prefix defines which prefixes are allowed or required and what they do.
Format defines which scheme the instruction code is modeled after.
Dest is the type of the destination operand.
Source1-3 defines the types of up to 3 source operands.
Link indicates branching into a subpage.
Options is used for various types of additional information.

These code tables are organized as a big branching tree. 
A line can branch into a subpage if more than one instruction or variant
begins with the same code bytes. Each subpage can branch further to form
a tree structure many levels deep. The first page, OpcodeMap0, is indexed
by the first code byte after any prefixes. The subpages can be indexed by
several different criteria, such as subsequent bytes, various bit-fields,
or by the values of any prefixes that come before the code byte. The
branching criteria are indicated in the 'link' column, while the submap
number is indicated in the 'instset' field.

The interpretation of an instruction may start at the root, OpcodeMap0, 
and follow any branches until the final leaf is found.
Instructions with VEX, EVEX or MVEX prefix use the VEX.mm bits as 
shortcuts to the subpages OpcodeMap1, OpcodeMap2 and OpcodeMap4.

The values in the tables do not use names for the constants because each
value would need the combination of several names so that the lines would
be extremely long and very difficult to align in a readable way. The meaning
of the values in each field in the map entries is defined in disasm.h.

OpcodeTables[] is an array of pointers to all the maps.

OpcodeTableLength[] indicates the size of each map.

If a map is incomplete, then the last entry should indicate a default for
the missing entries, i.e. how to display the illegal or unknown instruction
codes.

New entries can be added whenever a new extension to the instruction set is
introduced.

*****************************************************************************/

#include "stdafx.h"

// Primary opcode map. This is the root of the opcode lookup tree
SOpcodeDef OpcodeMap0[256] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"add",       0     , 0xC50  , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0     },    // 00
   {"add",       0     , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 01
   {"add",       0     , 0      , 0x12  , 0x1001, 0x1   , 0     , 0     , 0     , 0     , 0     , 0     },    // 02
   {"add",       0     , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 03
   {"add",       0     , 0      , 0x41  , 0xA1  , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 04
   {"add",       0     , 0x1100 , 0x81  , 0xA9  , 0x28  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 05
   {"push es",   0x8000, 0x2    , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 06
   {"pop  es",   0x8000, 0x2    , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 07
   {"or",        0     , 0xC50  , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0     },    // 08
   {"or",        0     , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 09
   {"or",        0     , 0      , 0x12  , 0x1001, 0x1   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0A
   {"or",        0     , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0B
   {"or",        0     , 0      , 0x41  , 0xA1  , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 0C
   {"or",        0     , 0x1100 , 0x81  , 0xA9  , 0x39  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 0D
   {"push cs",   0x8000, 0x2    , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0E
   {0,           0x1   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   , 0     },    // 0F link to OpcodeMap1
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"adc",       0     , 0xC50  , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0     },    // 10
   {"adc",       0     , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 11
   {"adc",       0     , 0      , 0x12  , 0x1001, 0x1   , 0     , 0     , 0     , 0     , 0     , 0     },    // 12
   {"adc",       0     , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 13
   {"adc",       0     , 0      , 0x41  , 0xA1  , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 14
   {"adc",       0     , 0x1100 , 0x81  , 0xA9  , 0x29  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 15
   {"push ss",   0x8000, 0x2    , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 16
   {"pop  ss",   0x8000, 0x2    , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 17
   {"sbb",       0     , 0xC50  , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0     },    // 18
   {"sbb",       0     , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 19
   {"sbb",       0     , 0      , 0x12  , 0x1001, 0x1   , 0     , 0     , 0     , 0     , 0     , 0     },    // 1A
   {"sbb",       0     , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 1B
   {"sbb",       0     , 0      , 0x41  , 0xA1  , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 1C
   {"sbb",       0     , 0x1100 , 0x81  , 0xA9  , 0x29  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 1D
   {"push ds",   0x8000, 0x2    , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 1E
   {"pop  ds",   0x8000, 0x2    , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 1F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"and",       0     , 0xC50  , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0     },    // 20
   {"and",       0     , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 21
   {"and",       0     , 0      , 0x12  , 0x1001, 0x1   , 0     , 0     , 0     , 0     , 0     , 0     },    // 22
   {"and",       0     , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 23
   {"and",       0     , 0      , 0x41  , 0xA1  , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 24
   {"and",       0     , 0x1100 , 0x81  , 0xA9  , 0x39  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 25
   {"es:",       0     , 0      , 0x8001, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 26
   {"daa",       0x8000, 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 27
   {"sub",       0     , 0xC50  , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0     },    // 28
   {"sub",       0     , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 29
   {"sub",       0     , 0      , 0x12  , 0x1001, 0x1   , 0     , 0     , 0     , 0     , 0     , 0     },    // 2A
   {"sub",       0     , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 2B
   {"sub",       0     , 0      , 0x41  , 0xA1  , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 2C
   {"sub",       0     , 0x1100 , 0x81  , 0xA9  , 0x29  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 2D
   {"cs:",       0     , 0      , 0x8001, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 2E
   {"das",       0x8000, 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 2F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"xor",       0     , 0xC50  , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0     },    // 30
   {"xor",       0     , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 31
   {"xor",       0     , 0      , 0x12  , 0x1001, 0x1   , 0     , 0     , 0     , 0     , 0     , 0     },    // 32
   {"xor",       0     , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 33
   {"xor",       0     , 0      , 0x41  , 0xA1  , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 34
   {"xor",       0     , 0x1100 , 0x81  , 0xA9  , 0x39  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 35
   {"ss:",       0     , 0      , 0x8001, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 36
   {"aaa",       0x8000, 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 37
   {"cmp",       0     , 0      , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0x4   },    // 38
   {"cmp",       0     , 0x1100 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0x4   },    // 39
   {"cmp",       0     , 0      , 0x12  , 0x1001, 0x1   , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 3A
   {"cmp",       0     , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 3B
   {"cmp",       0     , 0      , 0x41  , 0xA1  , 0x11  , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 3C
   {"cmp",       0     , 0x1100 , 0x81  , 0xA9  , 0x19  , 0     , 0     , 0     , 0     , 0     , 0x84  },    // 3D
   {"ds:",       0     , 0      , 0x8001, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 3E
   {"aas",       0x8000, 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 3F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"inc",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 40
   {"inc",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 41
   {"inc",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 42
   {"inc",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 43
   {"inc",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 44
   {"inc",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 45
   {"inc",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 46
   {"inc",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 47
   {"dec",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 48
   {"dec",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 49
   {"dec",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 4A
   {"dec",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 4B
   {"dec",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 4C
   {"dec",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 4D
   {"dec",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 4E
   {"dec",       0x8000, 0x100  , 0x3   , 0x1008, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 4F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"push",      0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 50
   {"push",      0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 51
   {"push",      0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 52
   {"push",      0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 53
   {"push",      0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 54
   {"push",      0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 55
   {"push",      0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 56
   {"push",      0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 57
   {"pop",       0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 58
   {"pop",       0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 59
   {"pop",       0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 5A
   {"pop",       0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 5B
   {"pop",       0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 5C
   {"pop",       0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 5D
   {"pop",       0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 5E
   {"pop",       0     , 0x2102 , 0x3   , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 5F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pusha",     0x8001, 0x102  , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x5   },    // 60
   {"popa",      0x8001, 0x102  , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   },    // 61
   {"bound",     0x8001, 0x106  , 0x12  , 0x1008, 0x2009, 0     , 0     , 0     , 0     , 0     , 0     },    // 62
   {0,           0x3B  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x7   , 0     },    // 63 Link to arpl/movsxd
   {"fs:",       0     , 0      , 0x8001, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 64
   {"gs:",       0     , 0      , 0x8001, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 65
   {"operand size:",0x0, 0      , 0x8000, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 66
   {"address size:",0x0, 0      , 0x8000, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 67
   {"push",      0     , 0x2102 , 0x82  , 0     , 0x29  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 68 push imm word
   {"imul",      0x1   , 0x1100 , 0x92  , 0x1009, 0x9   , 0x29  , 0     , 0     , 0     , 0     , 0x80  },    // 69 imul r,m,iv
   {"push",      0     , 0x2102 , 0x42  , 0     , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 6A push imm byte
   {"imul",      0x1   , 0x1100 , 0x52  , 0x1009, 0x9   , 0x21  , 0     , 0     , 0     , 0     , 0     },    // 6B imul r,m,ib
   {"insb",      0     , 0x21   , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 6C insb
   {"ins",       0     , 0x121  , 0x1   , 0x20C2, 0xB2  , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 6D insw
   {"outsb",     0     , 0x21   , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 6E outsb
   {"outs",      0     , 0x121  , 0x1   , 0xB2  , 0x20C2, 0     , 0     , 0     , 0     , 0     , 0x8   },    // 6F outs
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"jo",        0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 70 conditional short jumps
   {"jno",       0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 71
   {"jc",        0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 72
   {"jnc",       0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 73
   {"jz",        0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 74
   {"jnz",       0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 75
   {"jbe",       0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 76
   {"ja",        0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 77
   {"js",        0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 78
   {"jns",       0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 79
   {"jpe",       0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 7A
   {"jpo",       0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 7B
   {"jl",        0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 7C
   {"jge",       0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 7D
   {"jle",       0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 7E
   {"jg",        0     , 0x88   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 7F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"grp1",      0x1A  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 80 link to immediate grp 1
   {"grp1",      0x1B  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 81 link to immediate grp 1
   {"grp1",      0x1C  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 82 link to immediate grp 1
   {"grp1",      0x1D  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 83 link to immediate grp 1
   {"test",      0     , 0      , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0x4   },    // 84
   {"test",      0     , 0x1100 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0x4   },    // 85
   {"xchg",      0     , 0xC50  , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0x48  },    // 86
   {"xchg",      0     , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0x48  },    // 87
   {"mov",       0     , 0xC40  , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0x40  },    // 88
   {"mov",       0     , 0x1D40 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0x40  },    // 89
   {"mov",       0     , 0      , 0x12  , 0x1001, 0x1   , 0     , 0     , 0     , 0     , 0     , 0x40  },    // 8A
   {"mov",       0     , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0x40  },    // 8B
   {"mov",       0     , 0x1100 , 0x13  , 0x9   , 0x91  , 0     , 0     , 0     , 0     , 0     , 0     },    // 8C mov r16,segreg
   {"lea",       0     , 0x1101 , 0x12  , 0x1009, 0x2009, 0     , 0     , 0     , 0     , 0     , 0xC0  },    // 8D
   {"mov",       0     , 0x1100 , 0x12  , 0x91  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 8E mov segreg,r16
   {"pop",       0x28  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 8F Link to group 1A
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"nop",       0x3C  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 90 NOP/Pause. Link to map
   {"xchg",      0     , 0x1100 , 0x3   , 0x1009, 0xA9  , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 91 xchg cx,ax
   {"xchg",      0     , 0x1100 , 0x3   , 0x1009, 0xA9  , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 92 xchg dx,ax
   {"xchg",      0     , 0x1100 , 0x3   , 0x1009, 0xA9  , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 93 xchg bx,ax
   {"xchg",      0     , 0x1100 , 0x3   , 0x1009, 0xA9  , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 94 xchg sp,ax
   {"xchg",      0     , 0x1100 , 0x3   , 0x1009, 0xA9  , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 95 xchg bp,ax
   {"xchg",      0     , 0x1100 , 0x3   , 0x1009, 0xA9  , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 96 xchg si,ax
   {"xchg",      0     , 0x1100 , 0x3   , 0x1009, 0xA9  , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 97 xchg di,ax
   {"cbw",       0x39  , 0x1100 , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // 98 Link to map
   {"cwd",       0x3A  , 0x1100 , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // 99 Link to map
   {"call",      0x8000, 0x182  , 0x200 , 0x85  , 0     , 0     , 0     , 0     , 0     , 0     , 0x28  },    // 9A call far
   {"fwait",     0x100 , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 9B
   {"pushf",     0x3E  , 0x2100 , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // 9C Link to map: pushf/d/q
   {"popf",      0x3F  , 0x2100 , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // 9D Link to map: popf/d/q
   {"sahf",      0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 9E
   {"lahf",      0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 9F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"mov",       0     , 0x5    , 0x401 , 0x10A1, 0x2001, 0     , 0     , 0     , 0     , 0     , 0     },    // A0 mov al,mem
   {"mov",       0     , 0x1105 , 0x401 , 0x10A9, 0x2009, 0     , 0     , 0     , 0     , 0     , 0     },    // A1 mov ax,mem
   {"mov",       0     , 0x5    , 0x401 , 0x2001, 0x10A1, 0     , 0     , 0     , 0     , 0     , 0     },    // A2 mov mem,al
   {"mov",       0     , 0x1105 , 0x401 , 0x2009, 0x10A9, 0     , 0     , 0     , 0     , 0     , 0     },    // A3 mov mem,ax
   {"movs",      0     , 0x25   , 0x1   , 0x20C2, 0x20C1, 0     , 0     , 0     , 0     , 0     , 0x8   },    // A4 movsb
   {"movs",      0     , 0x1125 , 0x1   , 0x20C2, 0x20C1, 0     , 0     , 0     , 0     , 0     , 0x8   },    // A5 movsw
   {"cmps",      0     , 0x45   , 0x1   , 0x20C2, 0x20C1, 0     , 0     , 0     , 0     , 0     , 0x8   },    // A6 cmpsb
   {"cmps",      0     , 0x1145 , 0x1   , 0x20C2, 0x20C1, 0     , 0     , 0     , 0     , 0     , 0x8   },    // A7 cmpsw
   {"test",      0     , 0      , 0x41  , 0x10A1, 0x31  , 0     , 0     , 0     , 0     , 0     , 0x4   },    // A8 test al,ib
   {"test",      0     , 0x1100 , 0x81  , 0x10A9, 0x39  , 0     , 0     , 0     , 0     , 0     , 0x4   },    // A9 test ax,iw
   {"stos",      0     , 0x21   , 0x1   , 0x20C2, 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // AA stosb
   {"stos",      0     , 0x1121 , 0x1   , 0x20C2, 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // AB stosw
   {"lods",      0     , 0x25   , 0x1   , 0     , 0x20C1, 0     , 0     , 0     , 0     , 0     , 0x8   },    // AC lodsb
   {"lods",      0     , 0x1125 , 0x1   , 0     , 0x20C1, 0     , 0     , 0     , 0     , 0     , 0x8   },    // AD lodsw
   {"scas",      0     , 0x41   , 0x1   , 0x20C2, 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // AE scasb
   {"scas",      0     , 0x1141 , 0x1   , 0x20C2, 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // AF scasw
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"mov",       0     , 0      , 0x43  , 0x1001, 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // B0 mov al,ib
   {"mov",       0     , 0      , 0x43  , 0x1001, 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // B1 mov cl,ib
   {"mov",       0     , 0      , 0x43  , 0x1001, 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // B2 mov dl,ib
   {"mov",       0     , 0      , 0x43  , 0x1001, 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // B3 mov bl,ib
   {"mov",       0     , 0      , 0x43  , 0x1001, 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // B4 mov ah,ib
   {"mov",       0     , 0      , 0x43  , 0x1001, 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // B5 mov ch,ib
   {"mov",       0     , 0      , 0x43  , 0x1001, 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // B6 mov dh,ib
   {"mov",       0     , 0      , 0x43  , 0x1001, 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // B7 mov bh,ib
   {"mov",       0     , 0x1100 , 0x103 , 0x1009, 0x19  , 0     , 0     , 0     , 0     , 0     , 0x400 },    // B8 mov ax,iw
   {"mov",       0     , 0x1100 , 0x103 , 0x1009, 0x19  , 0     , 0     , 0     , 0     , 0     , 0x400 },    // B9 mov cx,iw
   {"mov",       0     , 0x1100 , 0x103 , 0x1009, 0x19  , 0     , 0     , 0     , 0     , 0     , 0x400 },    // BA mov dx,iw
   {"mov",       0     , 0x1100 , 0x103 , 0x1009, 0x19  , 0     , 0     , 0     , 0     , 0     , 0x400 },    // BB mov bx,iw
   {"mov",       0     , 0x1100 , 0x103 , 0x1009, 0x19  , 0     , 0     , 0     , 0     , 0     , 0x400 },    // BC mov sp,iw
   {"mov",       0     , 0x1100 , 0x103 , 0x1009, 0x19  , 0     , 0     , 0     , 0     , 0     , 0x400 },    // BD mov bp,iw
   {"mov",       0     , 0x1100 , 0x103 , 0x1009, 0x19  , 0     , 0     , 0     , 0     , 0     , 0x400 },    // BE mov si,iw
   {"mov",       0     , 0x1100 , 0x103 , 0x1009, 0x19  , 0     , 0     , 0     , 0     , 0     , 0x400 },    // BF mov di,iw
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"grp2",      0x1E  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // C0 link to grp 2
   {"grp2",      0x1F  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // C1 link to grp 2
   {"ret",       0     , 0x21AA , 0x22  , 0     , 0x12  , 0     , 0     , 0     , 0     , 0     , 0x30  },    // C2 retn iw
   {"ret",       0     , 0x21AA , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x30  },    // C3 retn
   {"les",       0x8000, 0x100  , 0x812 , 0x1009, 0x200C, 0     , 0     , 0     , 0     , 0     , 0     },    // C4 les
   {"lds",       0x8000, 0x100  , 0x812 , 0x1009, 0x200C, 0     , 0     , 0     , 0     , 0     , 0     },    // C5 lds
   {"mov",       0x2F  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // C6 link to grp 11
   {"mov",       0x30  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // C7 link to grp 11
   {"enter",     0     , 0      , 0x62  , 0     , 0x12  , 0x11  , 0     , 0     , 0     , 0     , 0x8   },    // C8
   {"leave",     0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // C9
   {"retf",      0     , 0x2182 , 0x22  , 0     , 0x12  , 0     , 0     , 0     , 0     , 0     , 0x10  },    // CA retf iw
   {"retf",      0     , 0x2182 , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x10  },    // CB retf
   {"int 3;breakpoint or filler",0,0,2  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x48  },    // CC
   {"int",       0     , 0      , 0x42  , 0     , 0x11  , 0     , 0     , 0     , 0     , 0     , 0x8   },    // CD
   {"into",      0x8000, 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // CE
   {0,           0x19  , 0x1100 , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // CF link to IRET
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"grp2",      0x20  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // D0 link to grp 2
   {"grp2",      0x21  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // D1 link to grp 2
   {"grp2",      0x22  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // D2 link to grp 2
   {"grp2",      0x23  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // D3 link to grp 2
   {"aam",       0x8000, 0      , 0x42  , 0     , 0x11  , 0     , 0     , 0     , 0     , 0     , 0x8   },    // D4. Don't show immediate operand if = 10 !
   {"aad",       0x8000, 0      , 0x42  , 0     , 0x11  , 0     , 0     , 0     , 0     , 0     , 0x8   },    // D5. Don't show immediate operand if = 10 !
   {"salc",      0x8000, 0      , 0x4002, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // D6 salc (undocumented opcode)
   {"xlat",      0x92  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x10  , 0     },    // D7. Link to xlat
   {"x87 instr", 0x8   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // D8 link to FP grp
   {"x87 instr", 0x9   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // D9 link to FP grp
   {"x87 instr", 0xA   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // DA link to FP grp
   {"x87 instr", 0xB   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // DB link to FP grp
   {"x87 instr", 0xC   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // DC link to FP grp
   {"x87 instr", 0xD   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // DD link to FP grp
   {"x87 instr", 0xE   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // DE link to FP grp
   {"x87 instr", 0xF   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // DF link to FP grp
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"loopne",    0     , 0x80   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // E0
   {"loope",     0     , 0x80   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // E1
   {"loop",      0     , 0x80   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // E2
   {"j(e/r)cxz", 0x3D  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xA   , 0     },    // E3 link to map
   {"in",        0x800 , 0      , 0x41  , 0xA1  , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // E4 in al,ib
   {"in",        0x800 , 0x100  , 0x41  , 0xA8  , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // E5 in ax,ib
   {"out",       0x800 , 0      , 0x41  , 0x32  , 0xA1  , 0     , 0     , 0     , 0     , 0     , 0     },    // E6 out ib,al
   {"out",       0x800 , 0x100  , 0x41  , 0x32  , 0xA8  , 0     , 0     , 0     , 0     , 0     , 0     },    // E7 out ib,ax
   {"call",      0     , 0xAA   , 0x82  , 0x83  , 0     , 0     , 0     , 0     , 0     , 0     , 0x28  },    // E8 call near
   {"jmp",       0     , 0xA8   , 0x82  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xB0  },    // E9 jmp near
   {"jmp",       0x8000, 0x80   , 0x202 , 0x84  , 0     , 0     , 0     , 0     , 0     , 0     , 0x30  },    // EA jmp far
   {"jmp",       0     , 0xA8   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0x30  },    // EB jmp short
   {"in",        0x800 , 0      , 0x1   , 0xA1  , 0xB2  , 0     , 0     , 0     , 0     , 0     , 0     },    // EC in al,dx
   {"in",        0x800 , 0x100  , 0x1   , 0xA8  , 0xB2  , 0     , 0     , 0     , 0     , 0     , 0     },    // ED in ax,dx
   {"out",       0x800 , 0      , 0x1   , 0xB2  , 0xA1  , 0     , 0     , 0     , 0     , 0     , 0     },    // EE out dx,al
   {"out",       0x800 , 0x100  , 0x1   , 0xB2  , 0xA8  , 0     , 0     , 0     , 0     , 0     , 0     },    // EF out dx,ax
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"lock:",     0     , 0      , 0x8000, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F0 lock prefix
   {"icebp",     0x8000, 0      , 0x4002, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F1 ICE breakpoint, undocumented opcode
   {"repne:",    0     , 0      , 0x8000, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 repne prefix
   {"repe:",     0     , 0      , 0x8000, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F3 repe  prefix
   {"hlt",       0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x48  },    // F4
   {"cmc",       0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F5
   {"grp3",      0x24  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // F6 link to grp 3
   {"grp3",      0x25  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // F7 link to grp 3
   {"clc",       0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F8
   {"stc",       0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F9
   {"cli",       0x800 , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // FA
   {"sti",       0x800 , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // FB
   {"cld",       0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // FC
   {"std",       0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // FD
   {"grp4",      0x26  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // FE link to grp 4
   {"grp5",      0x27  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     }     // FF link to grp 5
};

// Secondary opcode map for 2-byte opcode. First byte = 0F
// Indexed by second opcode byte
SOpcodeDef OpcodeMap1[256] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"grp6",      0x2A  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 0F 00 link to grp 6; sldt etc.
   {"grp7",      0x2B  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // 0F 01 link to grp 7; sgdt etc.
   {0,           0x5E  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 02 link to lar
   {0,           0x5F  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 03 link to lsl
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 04 Illegal
   {"syscall",   0x5   , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 0F 05
   {"clts",      0x805 , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 06
   {"sysret",    0x805 , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x10  },    // 0F 07
   {"invd",      0x804 , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 08
   {"wbinvd",    0x804 , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 09
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0A Illegal
   {"ud2",       0x3   , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x10  },    // 0F 0B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0C Illegal
   {0,           0xD1  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 0F 0D Link to prefetch
   {"FEMS",      0x1001, 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F OE. AMD only
   {0,           0x6   , 0      , 0x52  , 0     , 0     , 0     , 0     , 0     , 0     , 0x6   , 0     },    // 0F 0F. Link to tertiary map for AMD 3DNow instructions
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x40  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 10 Link to tertiary map: movups, etc.
   {0,           0x41  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 11 Link to tertiary map: movups, etc.
   {0,           0x42  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 12 Link to tertiary map: movlps, etc.
   {"movl",      0x11  ,0x812200, 0x13  , 0x234F, 0x144F, 0     , 0     , 0x1000, 0     , 0     , 0x3   },    // 0F 13 movlps/pd
   {"unpckl",    0x11  ,0x8D2200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x31  , 0     , 0     , 0x3   },    // 0F 14 unpcklps/pd
   {"unpckh",    0x11  ,0x8D2200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x31  , 0     , 0     , 0x3   },    // 0F 15 unpckhps/pd
   {0,           0x44  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 16 Link to tertiary map: movhps, etc.
   {"movh",      0x11  ,0x812200, 0x13  , 0x234F, 0x144F, 0     , 0     , 0x1000, 0     , 0     , 0x3   },    // 0F 17 movhps/pd
   {0,           0x35  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 0F 18 Link to tertiary map: group 16
   {"hint",      0x6   , 0      , 0x2012, 0     , 0x6   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 19. Hint instructions reserved for future use
   {0,           0x110 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 1A. Link to BNDMK etc
   {0,           0x111 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 1B. Link to BNDCL etc
   {"hint",      0x6   , 0      , 0x2012, 0     , 0x6   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 1C. Hint instructions reserved for future use
   {"hint",      0x6   , 0      , 0x2012, 0     , 0x6   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 1D. Hint instructions reserved for future use
   {"hint",      0x135 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 9     , 0     },    // 0F 1E. link to endbr64 etc.
   {"nop",       0x6   , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x40  },    // 0F 1F. Multi-byte nop
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"mov",       0x803 , 0      , 0x13  , 0x100A, 0x92  , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 20. mov r32/64,cr
   {"mov",       0x803 , 0x1000 , 0x13  , 0x1009, 0x93  , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 21. mov r32,dr
   {"mov",       0x803 , 0      , 0x12  , 0x92  , 0x100A, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 22. mov cr,r32/64
   {"mov",       0x803 , 0x1000 , 0x12  , 0x93  , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 23. mov dr,r32
#if 0 // Opcode 0F 24 has two meanings:
      // 1: mov r32,tr (obsolete, 80386 only)
   {"mov;80386 only",0x803,0x0  , 0x4013, 0x1003, 0x94  , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 24. mov r32,tr (80386 only)
#else
      // 2: start of 3-byte opcode for AMD SSE5 instructions with DREX byte
   {0,           0x68  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   , 0     },    // 0F 24. Link to tertiary map for 3-byte opcodes AMD SSE5 with four operands
#endif
   {0,           0x69  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   , 0     },    // 0F 25. Link to tertiary map for 3-byte opcodes AMD SSE5 with three operands + immediate byte
   {"mov;80386 only",0x803,0    , 0x4012, 0x94  , 0x1003, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 26. mov tr,r32 (80386 only)
   {0,           0x803 , 0      , 0x4012, 0x1003, 0x1003, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 27. illegal
   {"mova",      0x11  ,0xC52200, 0x12  , 0x124F, 0x24F,  0     , 0     , 0x30  , 0x1204, 0     , 0x103 },    // 0F 28. movaps/pd
   {"mova",      0xBC  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 29. Link to movaps/pd
   {0,           0x46  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 2A. Link to tertiary map: cvtpi2ps, etc.
   {0,           0xD2  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 2B. Link to tertiary map: movntps/pd,AMD: also ss/sd
   {0,           0x47  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 2C. Link to tertiary map: cvttps2pi, etc.
   {0,           0x48  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 2D. Link to tertiary map: cvtps2pi, etc.
   {0,           0x4B  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 2E. Link to tertiary map: ucomiss/sd
   {0,           0x4C  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 2F. Link to tertiary map: comiss/sd
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"wrmsr",     0x805 , 0x1000 , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 30
   {"rdtsc",     0x5   , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 0F 31
   {"rdmsr",     0x805 , 0x1000 , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 0F 32
   {"rdpmc",     0x5   , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 0F 33
   {"sysenter",  0x8   , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 0F 34
   {"sysexit;Same name with or without 48h prefix",0x808,0x1000,1,0,0,0,0, 0     ,0x0   , 0     , 0     },    // 0F 35
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 36 Illegal
   {"getsec",    0x14  , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 0F 37
   {0,           0x2   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   , 0     },    // 0F 38. Link to tertiary map for 3-byte opcodes
   {0,           0x3   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   , 0     },    // 0F 39. Link to tertiary map for 3-byte opcodes
   {0,           0x4   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   , 0     },    // 0F 3A. Link to tertiary map for 3-byte opcodes
   {0,           0x5   , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   , 0     },    // 0F 3B. Link to tertiary map for 3-byte opcodes
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3C Illegal
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3D Illegal
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3E Illegal
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3F (VIA/Centaur ALTINST ?)
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cmovo",     0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 40. cmov
   {"cmovno",    0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 41. cmov
   {"cmovc",     0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 42. cmov
   {"cmovnc",    0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 43. cmov
   {"cmove",     0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 44. cmov
   {"cmovne",    0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 45. cmov
   {"cmovbe",    0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 46. cmov
   {"cmova",     0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 47. cmov
   {"cmovs",     0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 48. cmov
   {"cmovns",    0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 49. cmov
   {"cmovpe",    0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 4A. cmov
   {"cmovpo",    0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 4B. cmov
   {"cmovl",     0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 4C. cmov
   {"cmovge",    0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 4D. cmov
   {"cmovle",    0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 4E  cmov
   {"cmovg",     0x6   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 4F. cmov
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movmskp",   0xCA  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 50. Link to movmskps/pd
   {"sqrt",      0x76  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 51. Link to sqrtps/pd/ss/sd
   {"rsqrt",     0x77  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 52. Link to rsqrtps/ss
   {"rcp",       0x78  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 53. Link to rcpps/ss
   {"and",       0x11  ,0x8D2200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x21  , 0     , 0     , 0x3   },    // 0F 54. andps/pd
   {"andn",      0x11  ,0x8D2200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x21  , 0     , 0     , 0x3   },    // 0F 55. andnps/pd
   {"or",        0x11  ,0x8D2200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x21  , 0     , 0     , 0x3   },    // 0F 56. orps/pd
   {"xor",       0x11  ,0x8D2200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x21  , 0     , 0     , 0x3   },    // 0F 57. xorps/pd
   {"add",       0x11  ,0xE92E00, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x3   },    // 0F 58. addps/pd/ss/sd
   {"mul",       0x11  ,0xE92E00, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x3   },    // 0F 59. mulps/pd/ss/sd
   {0,           0x49  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 5A. Link to cvtps2pd, etc.
   {0,           0x4A  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 5B. Link to cvtdq2ps, etc.
   {"sub",       0x11  ,0xE92E00, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x3   },    // 0F 5C. subps/pd/ss/sd
   {"min",       0x11  ,0xA92E00, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x33  , 0     , 0     , 0x3   },    // 0F 5D. minps/pd/ss/sd
   {"div",       0x11  ,0xA92E00, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0     , 0     , 0x3   },    // 0F 5E. divps/pd/ss/sd
   {"max",       0x11  ,0xA92E00, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x33  , 0     , 0     , 0x3   },    // 0F 5F. maxps/pd/ss/sd
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"punpcklbw", 0x7   ,0x8D0200, 0x19  , 0x1101, 0x1101, 0x101 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 60
   {"punpcklwd", 0x7   ,0x8D2200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 61
   {"punpckldq", 0x7   ,0x8D0200, 0x19  , 0x1103, 0x1103, 0x103 , 0     , 0x21  , 0     , 0     , 0x2   },    // 0F 62
   {"packsswb",  0x7   ,0x8D2200, 0x19  , 0x1201, 0x1202, 0x202 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F 63
   {"pcmpgtb",   0x118 , 0      , 0x19  , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 64
   {"pcmpgtw",   0x119 , 0      , 0x19  , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 65
   {"pcmpgtd",   0xC8  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 66. link to pcmpgtd
   {"packuswb",  0x7   ,0x8D0200, 0x19  , 0x1201, 0x1202, 0x202 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F 67
   {"punpckhbw", 0x7   ,0x8D2200, 0x19  , 0x1101, 0x1101, 0x101 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F 68
   {"punpckhwd", 0x7   ,0x8D2200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F 69
   {"punpckhdq", 0x7   ,0x8D0200, 0x19  , 0x1103, 0x1103, 0x103 , 0     , 0x31  , 0     , 0     , 0x2   },    // 0F 6A
   {"packssdw",  0x7   ,0x8D0200, 0x19  , 0x1202, 0x1203, 0x203 , 0     , 0x21  , 0     , 0     , 0x2   },    // 0F 6B
   {"punpcklqdq",0x12  ,0x8DB200, 0x19  , 0x1204, 0x1204, 0x204 , 0     , 0x21  , 0     , 0     , 0x2   },    // 0F 6C
   {"punpckhqdq",0x12  ,0x8DB200, 0x19  , 0x1204, 0x1204, 0x204 , 0     , 0x21  , 0     , 0     , 0x2   },    // 0F 6D
   {0,           0x58  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // 0F 6E. Link to tertiary map: movd/movq
   {0,           0x4D  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 6F. Link to tertiary map: movq/movdqa/movdqu
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x4F  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 70. Link to tertiary map: pshufw, etc.
   {0,           0x31  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 0F 71. Link to tertiary map for group 12
   {0,           0x32  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 0F 72. Link to tertiary map for group 13
   {0,           0x33  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 0F 73. Link to tertiary map for group 14
   {"pcmpeqb",   0x116 , 0      , 0x19  , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 74
   {"pcmpeqw",   0x117 , 0      , 0x19  , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 75
   {"pcmpeqd",   0xC7  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 76. link to pcmpeqd
   {"emms",      0x79  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xB   , 0     },    // 0F 77. Link to emms, vzeroupper, vzeroall
   {0,           0x6C  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 78. Link to map for wmread, insrtq, extrq
   {0,           0x6D  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 79 without EVEX. Link to map for wmread, insrtq, extrq
   {0,           0x6A  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   , 0     },    // 0F 7A. Link to map for obsolete 3-byte opcodes AMD SSE5. Note: VEX 0F 7A is in map B1
   {0,           0x6B  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   , 0     },    // 0F 7B. Link to map for obsolete 3-byte opcodes AMD SSE5. Note: VEX 0F 7B is in map B1
   {0,           0x5C  , 0xA00  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 7C. Link to map: hadd
   {0,           0x5D  , 0xA00  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 7D. Link to map: hsub
   {0,           0x59  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 7E. Link to map: movd/movq
   {0,           0x4E  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 7F. Link to map: movq/movdqa/movdqu
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"jo",        0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 80
   {"jno",       0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 81
   {"jc",        0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 82
   {"jnc",       0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 83
   {"je",        0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 84
   {"jne",       0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 85
   {"jbe",       0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 86
   {"ja",        0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 87
   {"js",        0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 88
   {"jns",       0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 89
   {"jpe",       0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 8A
   {"jpo",       0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 8B
   {"jl",        0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 8C
   {"jge",       0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 8D
   {"jle",       0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 8E
   {"jg",        0x3   , 0x8    , 0x80  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0     , 0xA0  },    // 0F 8F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"seto",      0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 90
   {"setno",     0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 91
   {"setb",      0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 92
   {"setae",     0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 93
   {"sete",      0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 94
   {"setne",     0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 95
   {"setbe",     0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 96
   {"seta",      0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 97
   {"sets",      0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 98
   {"setns",     0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 99 
   {"setpe",     0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 9A
   {"setpo",     0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 9B
   {"setl",      0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 9C
   {"setge",     0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 9D
   {"setle",     0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 9E
   {"setg",      0x3   , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 9F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"push fs",   0x3   , 0x2    , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F A0
   {"pop  fs",   0x3   , 0x2    , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F A1
   {"cpuid",     0x4   , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 0F A2
   {"bt",        0x3   , 0x1100 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F A3
   {"shld",      0x3   , 0x1100 , 0x53  , 0x9   , 0x1009, 0x11  , 0     , 0     , 0     , 0     , 0     },    // 0F A4
   {"shld",      0x3   , 0x1100 , 0x13  , 0x9   , 0x9   , 0xB3  , 0     , 0     , 0     , 0     , 0     },    // 0F A5
   {0,           0xA6  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // 0F A6. Link to VIA instructions
   {0,           0xA7  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // 0F A7. Link to VIA instructions
   {"push gs",   0x3   , 0x2    , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F A8
   {"pop  gs",   0x3   , 0x2    , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F A9
   {"rsm",       0x803 , 0      , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F AA
   {"bts",       0x3   , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F AB
   {"shrd",      0x3   , 0x1100 , 0x53  , 0x9   , 0x1009, 0x11  , 0     , 0     , 0     , 0     , 0     },    // 0F AC
   {"shrd",      0x3   , 0x1100 , 0x13  , 0x9   , 0x9   , 0xB3  , 0     , 0     , 0     , 0     , 0     },    // 0F AD
   {0,           0x34  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // 0F AE. Link to tertiary map for group 15
   {"imul",      0x1   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F AF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cmpxchg",   0x3   , 0xC50  , 0x13  , 0x2001, 0x1001, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F B0
   {"cmpxchg",   0x3   , 0x1D50 , 0x13  , 0x2009, 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F B1
   {"lss",       0     , 0x1100 , 0x812 , 0x1009, 0x200D, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F B2 (valid in 64-bit mode)
   {"btr",       0x3   , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F B3
   {"lfs",       0     , 0x1100 , 0x812 , 0x1009, 0x200D, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F B4
   {"lgs",       0     , 0x1100 , 0x812 , 0x1009, 0x200D, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F B5
   {"movzx",     0x3   , 0x1100 , 0x12  , 0x1009, 0x1   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F B6
   {"movzx",     0x3   , 0x1100 , 0x12  , 0x1009, 0x2   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F B7   
   {0,           0x60  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F B8. Link to tertiary map for popcnt, jmpe
   {0,           0x2E  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 0F B9. Link to tertiary map for group 10: ud1
   {0,           0x2C  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 0F BA. Link to tertiary map for group 8: bt
   {"btc",       0x3   , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F BB
   {"bsf",       0xAE  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F BC. Link to bsf etc.
   {0,           0xD3  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F BD. Link to tertiary map for BSR and LZCNT
   {"movsx",     0x3   , 0x1100 , 0x12  , 0x1009, 0x1   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F BE
   {"movsx",     0x3   , 0x1100 , 0x12  , 0x1009, 0x2   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F BF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"xadd",      0x4   , 0x0C50 , 0x13  , 0x1   , 0x1001, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F C0
   {"xadd",      0x4   , 0x1D50 , 0x13  , 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F C1
   {0,           0xF5  , 0      , 0x52  , 0     , 0     , 0     , 0     , 0     , 0     , 0x11  , 0     },    // 0F C2. Link to cmpps etc.
   {"movnti",    0x11  , 0x1000 , 0x13  , 0x2009, 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F C3
   {0,           0x29  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F C4. Link to pinsrw
   {"pextrw",    0x7   ,0x812200, 0x52  , 0x1002, 0x1102, 0x11  , 0     , 0x1000, 0     , 0     , 0x2   },    // 0F C5
   {"shuf",      0x11  ,0x8D2200, 0x59  , 0x124F, 0x124F, 0x24F , 0x11  , 0x31  , 0     , 0     , 0x3   },    // 0F C6
   {0,           0x50  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 0F C7. Link to tertiary map for group 9
   {"bswap",     0x3   , 0x1000 , 0x3   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F C8. bswap eax
   {"bswap",     0x3   , 0x1000 , 0x3   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F C9. bswap ecx
   {"bswap",     0x3   , 0x1000 , 0x3   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F CA. bswap edx
   {"bswap",     0x3   , 0x1000 , 0x3   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F CB. bswap ebx
   {"bswap",     0x3   , 0x1000 , 0x3   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F CC. bswap esp
   {"bswap",     0x3   , 0x1000 , 0x3   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F CD. bswap ebp
   {"bswap",     0x3   , 0x1000 , 0x3   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F CE. bswap esi
   {"bswap",     0x3   , 0x1000 , 0x3   , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F CF. bswap edi
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x2D  , 0xA00  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F D0. Link to addsubps/pd
   {"psrlw",     0x99  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F D1. Link to map for psrlw
   {"psrld",     0x9A  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F D2. Link to map for psrld
   {"psrlq",     0x9B  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F D3. Link to map for psrlq
   {"paddq",     0x12  ,0x8D3200, 0x19  , 0x1204, 0x1204, 0x204 , 0     , 0x21  , 0     , 0     , 0x2   },    // 0F D4
   {"pmullw",    0x7   ,0x8DA200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F D5
   {0,           0x53  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F D6. Link to tertiary map for movq2dq etc.
   {"pmovmskb",  0x93  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0B  , 0     },    // 0F D7. Link pmovmskb
   {"psubusb",   0x7   ,0x8D2200, 0x19  , 0x1101, 0x1101, 0x101 , 0     , 0x20   , 0     , 0     , 0x2   },    // 0F D8
   {"psubusw",   0x7   ,0x8D2200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F D9
   {"pminub",    0x7   ,0x8D0200, 0x19  , 0x1101, 0x1101, 0x101 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F DA
   {"pand",      0xC2  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F DB. link to pand
   {"paddusb",   0x7   ,0x8D2200, 0x19  , 0x1201, 0x1201, 0x201 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F DC
   {"paddusw",   0x7   ,0x8D2200, 0x19  , 0x1202, 0x1202, 0x202 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F DD
   {"pmaxub",    0x7   ,0x8D2200, 0x19  , 0x1101, 0x1101, 0x101 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F DE
   {"pandn",     0xC3  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F DF. link to pandn
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pavgb",     0x7   ,0x8D2200, 0x19  , 0x1201, 0x1201, 0x201 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F E0
   {"psraw",     0x9C  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F E1. Link to map for psraw
   {"psrad",     0x9D  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F E2. Link to map for psrad
   {"pavgw",     0x7   ,0x8D2200, 0x19  , 0x1202, 0x1202, 0x202 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F E3
   {"pmulhuw",   0x7   ,0x8DA200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F E4
   {"pmulhw",    0x7   ,0x8DA200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F E5
   {0,           0x54  , 0xE00  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F E6. Link to tertiary map for cvtpd2dq etc.
   {0,           0x55  , 0x200  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F E7. Link to tertiary map for movntq
   {"psubsb",    0x7   ,0x8D2200, 0x19  , 0x1101, 0x1101, 0x101 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F E8
   {"psubsw",    0x7   ,0x8D2200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20   , 0     , 0     , 0x2   },    // 0F E9
   {"pminsw",    0x7   ,0x8D2200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F EA
   {"por",       0xC4  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F EB. link to por
   {"paddsb",    0x7   ,0x8D2200, 0x19  , 0x1201, 0x1201, 0x201 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F EC
   {"paddsw",    0x7   ,0x8D2200, 0x19  , 0x1202, 0x1202, 0x202 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F ED
   {"pmaxsw",    0x7   ,0x8D2200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F EE
   {"pxor",      0xC5  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F EF. link to pxor
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x56  , 0x800  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F F0. Link to tertiary map for lddqu
   {"psllw",     0x96  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F F1. Link to map for psllw
   {"pslld",     0x97  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F F2. Link to map for pslld
   {"psllq",     0x98  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F F3. Link to map for psllq
   {"pmuludq",   0x7   ,0x8D2200, 0x19  , 0x1104, 0x1104, 0x104 , 0     , 0x31  , 0     , 0     , 0x2   },    // 0F F4 (32 bit memory operand is broadcast as 64 bit into every second dword)
   {"pmaddwd",   0x7   ,0x8D2200, 0x19  , 0x1103, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F F5
   {"psadbw",    0x7   ,0x8D2200, 0x19  , 0x1102, 0x1101, 0x101 , 0     , 0     , 0     , 0     , 0x2   },    // 0F F6
   {0,           0x57  , 0x200  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F F7. Link to tertiary map for maskmovq
   {"psubb",     0x7   ,0x8D2200, 0x19  , 0x1101, 0x1101, 0x101 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F F8
   {"psubw",     0x7   ,0x8D2200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F F9
   {"psubd",     0x7   ,0xCD0200, 0x19  , 0x1103, 0x1103, 0x103 , 0     , 0x21  , 0x1406, 0     , 0x2   },    // 0F FA   
   {"psubq",     0x7   ,0x8D2200, 0x19  , 0x1104, 0x1104, 0x104 , 0     , 0x21  , 0     , 0     , 0x2   },    // 0F FB
   {"paddb",     0x7   ,0x8D2200, 0x19  , 0x1201, 0x1201, 0x201 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F FC
   {"paddw",     0x7   ,0x8D2200, 0x19  , 0x1202, 0x1202, 0x202 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F FD
   {"paddd",     0x7   ,0xCD0200, 0x19  , 0x1103, 0x1103, 0x103 , 0     , 0x31  , 0x1406, 0     , 0x2   },    // 0F FE
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F FF


// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 38
// or VEX encoded with mmmm = 2
// Indexed by third opcode byte
SOpcodeDef OpcodeMap2[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pshufb",    0x14  ,0x8D2200, 0x19  , 0x1101, 0x1101, 0x101 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F 38 00
   {"phaddw",    0x14  , 0xD0200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 01
   {"phaddd",    0x14  , 0xD0200, 0x19  , 0x1103, 0x1103, 0x103 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 02
   {"phaddsw",   0x14  , 0xD0200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 03
   {"pmaddubsw", 0x14  ,0x8D2200, 0x19  , 0x1102, 0x1101, 0x101 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F 38 04
   {"phsubw",    0x14  , 0xD0200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 05
   {"phsubd",    0x14  , 0xD0200, 0x19  , 0x1103, 0x1103, 0x103 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 06
   {"phsubsw",   0x14  , 0xD0200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 07
   {"psignb",    0x14  , 0xD0200, 0x19  , 0x1101, 0x1101, 0x101 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 08
   {"psignw",    0x14  , 0xD0200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 09
   {"psignd",    0x14  , 0xD0200, 0x19  , 0x1103, 0x1103, 0x103 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 0A

   {"pmulhrsw",  0x14  ,0x8DA200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F 38 0B
   {"vpermilps", 0x19  ,0x8FA200, 0x19  , 0x124B, 0x124B, 0x24B , 0     , 0x31  , 0     , 0     , 0     },    // 0F 38 0C
   {"vpermilpd", 0x19  ,0x8FA200, 0x19  , 0x124C, 0x124C, 0x24C , 0     , 0x31  , 0     , 0     , 0     },    // 0F 38 0D
   {"vtestps",   0x19  , 0x78200, 0x12  , 0x124B, 0x24B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 0E
   {"vtestpd",   0x19  , 0x78200, 0x12  , 0x124B, 0x24B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 0F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0xEA  , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0x11  , 0     },    // 0F 38 10. Link pblendvb and vpsrlvw
   {"vpsravw",   0x1C  ,0x8FC200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x20  , 0     , 0     , 0     },    // 0F 38 11
   {"vpsllvw",   0x20  ,0x8FC200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x20  , 0     , 0     , 0     },    // 0F 38 12
   {"vcvtph2ps", 0x19  ,0x878200, 0x12  , 0x250 , 0xF4A , 0     , 0     , 0x2232, 0     , 0     , 0     },    // 0F 38 13
   {0,           0x8D  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xB   , 0     },    // 0F 38 14. Link to vprorvd blendvps and vpmovqw
   {0,           0x8E  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xB   , 0     },    // 0F 38 15. Link to vprolvd blendvpd and vpmovqd
   {"vpermp",    0x1C  ,0x9F9200 ,0x19  , 0x124F, 0x1203, 0x24F , 0     , 0x31  , 0     , 0     , 0x1   },    // 0F 38 16
   {"ptest",     0x15  , 0x58200, 0x12  , 0x1250, 0x250 , 0     , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 17. Also in AMD SSE5 instruction set
   {"vbroadcastss",0x19,0xC78200, 0x12  , 0x124B, 0x04B , 0     , 0     , 0x20  , 0x1048, 0     , 0     },    // 0F 38 18
   {0,           0x12A , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0E  , 0     },    // 0F 38 19. Link to vbroadcastsd
   {0,           0xE5  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0C  , 0     },    // 0F 38 1A. Link to broadcast instructions
   {0,           0x38  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0C  , 0     },    // 0F 38 1B. Link to vbroadcastf64x4
   {"pabsb",     0x14  ,0x85A200, 0x12  , 0x1201, 0x201 , 0     , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F 38 1C
   {"pabsw",     0x14  ,0x85A200, 0x12  , 0x1202, 0x202 , 0     , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F 38 1D
   {"pabsd",     0x14  ,0x85B200, 0x12  , 0x1203, 0x203 , 0     , 0     , 0x31  , 0     , 0     , 0x2   },    // 0F 38 1E
   {"vpabsq",    0x20  ,0x85B200, 0x12  , 0x1203, 0x203 , 0     , 0     , 0x31  , 0     , 0     , 0x0   },    // 0F 38 1F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x7A  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 20. Link pmovsxbw
   {0,           0x7B  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 21. Link pmovsxbd and vpmovdb
   {0,           0x7D  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 22. Link pmovsxbq and vpmovqb
   {0,           0x7F  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 23. Link pmovsxwd and vpmovdw
   {0,           0x80  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 24. Link pmovsxwq and vpmovqw
   {0,           0x82  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 25. Link pmovsxdq and vpmovqd
   {"vptestm",   0x20  ,0x8BC200, 0x19  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 1     },    // 0F 38 27
   {"vptestm",   0x20  ,0xCBB200, 0x19  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 1     },    // 0F 38 27
   {"pmuldq",    0x15  ,0x8DA200, 0x19  , 0x1204, 0x1204, 0x204 , 0     , 0x31  , 0     , 0     , 0x2   },    // 0F 38 28  (32 bit memory operand is broadcast as 64 bit into every second dword)
   {0,           0xE3  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 38 29. Link to pcmpeqq
   {0,           0x91  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 38 2A. Link to movntdqa and vpbroadcastmb2q
   {"packusdw",  0x15  ,0x8D8200, 0x19  , 0x1202, 0x1203, 0x203 , 0     , 0x21  , 0     , 0     , 0x2   },    // 0F 38 2B
   {0,           0xFD  , 0      , 0   ,   0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 38 2C. Link to vmaskmovps and vscalefps
   {0,           0xFE  , 0      , 0   ,   0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 38 2D. Link to vmaskmovss and vscalefss
   {"vmaskmovps",0x19  , 0xF8200, 0x1A,   0x224B, 0x124B, 0x124B, 0     , 0     , 0     , 0     , 0     },    // 0F 38 2E
   {"vmaskmovpd",0x19  , 0xF8200, 0x1A,   0x224C, 0x124C, 0x124C, 0     , 0     , 0     , 0     , 0     },    // 0F 38 2F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x83  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 30. Link pmovzxbv
   {0,           0x85  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 31. Link pmovzxbd and vpmovdb
   {0,           0x87  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 32. Link pmovzxbq and vpmovqb
   {0,           0x89  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 33. Link pmovzxwd and vpmovdw
   {0,           0x8A  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 34. Link pmovzxwq and vpmovqw
   {0,           0x8C  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 35. Link pmovzxdq and vpmovqd
   {"vperm",     0x1C  , 0xCFB200,0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1000, 0     , 0x1   },    // 0F 38 36
   {0,           0xE4  , 0      , 0x19  , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 38 37
   {0,           0x12C , 0      , 0x19  , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 38 38. Link to pminsb etc.
   {0,           0xE6  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 38 39. Link pminsd
   {0,           0xFF  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 38 3A
   {0       ,    0xE7  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0x2   },    // 0F 38 3B. Link pminud
   {"pmaxsb",    0x15  ,0x8DA200, 0x19  , 0x1201, 0x1201, 0x201 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F 38 3C
   {0,           0xE8  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 38 3D. Link pmaxsd
   {"pmaxuw",    0x15  ,0x8DA200, 0x19  , 0x1202, 0x1202, 0x202 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F 38 3E
   {0,           0xE9  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 38 3F. Link pmaxud
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pmull",     0x15  ,0xCDB200, 0x19  , 0x1203, 0x1203, 0x203 , 0     , 0x31  , 0x1406, 0     , 0x3   },    // 0F 38 40
   {"phminposuw",0x15  ,0x18200,  0x12  , 0x1402, 0x402 , 0     , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 41
   {"vgetexpp",  0x20  ,0xC29200, 0x12  , 0x124F, 0x24F , 0     , 0     , 0x33  , 0x1204, 0     , 0x101 },    // 0F 38 42
   {"vgetexps",  0x20  ,0xCA9200, 0x19  , 0x144F, 0x24F , 0x04F , 0     , 0x32  , 0x1204, 0     , 0x101 },    // 0F 38 43
   {"vplzcnt",   0x21  ,0x80B200, 0x12  , 0x1209, 0x0209, 0     , 0     , 0x31  , 0     , 0     , 0x1   },    // 0F 38 44
   {"vpsrlv",    0x1C  ,0xCFB200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1406, 0     , 0x1   },    // 0F 38 45
   {"vpsrav",    0x1C  ,0xCFB200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1406, 0     , 0x1   },    // 0F 38 46
   {"vpsllv",    0x1C  ,0xCFB200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1406, 0     , 0x1   },    // 0F 38 47
   {"(reserved)",0x0   ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 48
   {"(reserved)",0x0   ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 49
   {"(reserved)",0x0   ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 4A
   {"(reserved)",0x0   ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 4B
   {"vrcp14p"   ,0x20  ,0x8D9200, 0x12  , 0x124F, 0x024F, 0     , 0     , 0x31  , 0     , 0     , 0x1   },    // 0F 38 4C
   {"vrcp14s"   ,0x20  ,0x8D9200, 0x19  , 0x144F, 0x144F, 0x004F, 0     , 0x30  , 0     , 0     , 0x1   },    // 0F 38 4D
   {"vrsqrt14p", 0x20  ,0x8D9200, 0x12  , 0x124F, 0x024F, 0     , 0     , 0x31  , 0     , 0     , 0x1   },    // 0F 38 4E
   {"vrsqrt14s", 0x20  ,0x8D9200, 0x19  , 0x144F, 0x144F, 0x004F, 0     , 0x30  , 0     , 0     , 0x1   },    // 0F 38 4F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vaddnp",    0x80  ,0x4A9200, 0x19,   0x164F, 0x164F, 0x64F , 0     , 0     , 0x1304, 0     , 0x101 },    // 0F 38 50
   {"vgmaxabsps",0x80  ,0x428200, 0x12  , 0x164F, 0x64F , 0     , 0     , 0     , 0x1204, 0     , 0x100 },    // 0F 38 51
   {"vgminp",    0x80  ,0x429200, 0x12  , 0x164F, 0x64F , 0     , 0     , 0     , 0x1204, 0     , 0x101 },    // 0F 38 52
   {"vgmaxp",    0x80  ,0x429200, 0x12  , 0x164F, 0x64F , 0     , 0     , 0     , 0x1204, 0     , 0x101 },    // 0F 38 53
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 54
   {"vfixupnanp",0x80  ,0x4A9200, 0x19,   0x164F, 0x164F, 0x603 , 0     , 0     , 0x1206, 0     , 0x101 },    // 0F 38 55
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 56
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 57
   {"vpbroadcastd",0xA0, 0      , 0   ,   0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 58. Link to vpbroadcastd
   {"vpbroadcastq",0xA1, 0      , 0   ,   0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 59. Link to vpbroadcastq
   {0,           0x84  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F 38 5A. Link to broadcast instructions
   {"vbroadcasti64x4",0x80,0xC29200,0x12, 0x1604, 0x2504, 0     , 0     , 0x20  , 0x1013, 0     , 0x100 },    // 0F 38 5B
   {"vpadcd",    0x80  , 0x4A8200,0x19  , 0x1603, 0x95  , 0x603 , 0     , 0     , 0x1406, 0     , 0x100 },    // 0F 38 5C
   {"vpaddsetcd",0x80  , 0x4A8200,0x19  , 0x1603, 0x95  , 0x603 , 0     , 0     , 0x1406, 0     , 0x100 },    // 0F 38 5D
   {"vpsbbd",    0x80  , 0x4A8200,0x19  , 0x1603, 0x95  , 0x603 , 0     , 0     , 0x1406, 0     , 0x100 },    // 0F 38 5E
   {"vpsubsetbd",0x80  , 0x4A8200,0x19  , 0x1603, 0x95  , 0x603 , 0     , 0     , 0x1406, 0     , 0x100 },    // 0F 38 5F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 60
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 61
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 62
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 63
   {"vpblendm",  0x20  ,0xCAB200, 0x19,   0x1209, 0x1209, 0x209 , 0     , 0x21  , 0x1406, 0     , 0x001 },    // 0F 38 64 (alignment required only in Knights Corner)
   {"vblendmp",  0x80  ,0xCA9200, 0x19,   0x124F, 0x124F, 0x24F , 0     , 0x21  , 0x1404, 0     , 0x101 },    // 0F 38 65
   {"vpblendm",  0x20  ,0x8AC200, 0x19,   0x1209, 0x1209, 0x209 , 0     , 0x21  , 0x1406, 0     , 0x001 },    // 0F 38 66
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 67
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 68
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 69
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 6A
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 6B
   {"vpsubrd",   0x80  , 0x4A8200,0x19  , 0x1603, 0x95  , 0x603 , 0     , 0     , 0x1406, 0     , 0x100 },    // 0F 38 6C
   {"vsubrp",    0x80  ,0x4A9200, 0x19,   0x164F, 0x164F, 0x64F , 0     , 0     , 0x1304, 0     , 0x101 },    // 0F 38 6D
   {"vpsbbrd",   0x80  , 0x4A8200,0x19  , 0x1603, 0x95  , 0x603 , 0     , 0     , 0x1406, 0     , 0x100 },    // 0F 38 6E
   {"vpsubrsetbd",0x80 , 0x4A8200,0x19  , 0x1603, 0x95  , 0x603 , 0     , 0     , 0x1406, 0     , 0x100 },    // 0F 38 6F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 70
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 71
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 72
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 73
   {"vpcmpltd",  0x80  ,0x4B8200, 0x19  , 0x95  , 0x1603, 0x603 , 0     , 0     , 0x1406, 0     , 0x100 },    // 0F 38 74
   {"vpermi2" ,  0x23  ,0x8EC200, 0x19,   0x1209, 0x1209, 0x209 , 0     , 0x20  , 0     , 0     , 1     },    // 0F 38 75 (instruction set avx512vbmi for byte version)
   {"vpermi2",   0x20  ,0x8AB200, 0x19,   0x1609, 0x1609, 0x0609, 0     , 0x31  , 0     , 0     , 0x1   },    // 0F 38 76
   {"vpermi2p",  0x20  ,0x8A9200, 0x19,   0x164F, 0x164F, 0x064F, 0     , 0x31  , 0     , 0     , 0x1   },    // 0F 38 77
   {"vpbroadcastb",0x9E, 0      , 0   ,   0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 78. Link to vpbroadcastb
   {"vpbroadcastw",0x9F, 0      , 0   ,   0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 38 79. Link to vpbroadcastw
   {"vpbroadcastb",0x20,0x828200, 0x12,   0x1201, 0x1001, 0     , 0     , 0x20  , 0     , 0     , 0     },    // 0F 38 7A
   {"vpbroadcastw",0x20,0x828200, 0x12,   0x1202, 0x1002, 0     , 0     , 0x20  , 0     , 0     , 0     },    // 0F 38 7B
   {"vpbroadcast",0x20 ,0x82B200, 0x12,   0x1209, 0x1009, 0     , 0     , 0x20  , 0     , 0     , 0x1   },    // 0F 38 7C
   {"vpermt2" ,  0x20  ,0x8EC200, 0x19,   0x1209, 0x1209, 0x209 , 0     , 0x20  , 0     , 0     , 1     },    // 0F 38 7D (instruction set avx512vbmi for byte version)
   {"vpermt2",   0x20  ,0x8AB200, 0x19,   0x1209, 0x1209, 0x209 , 0     , 0x21  , 0     , 0     , 0x1   },    // 0F 38 7E
   {"vpermt2p",  0x20  ,0x8A9200, 0x19,   0x164F, 0x164F, 0x064F, 0     , 0x21  , 0     , 0     , 0x1   },    // 0F 38 7F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 80
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 81
   {"invpcid",   0x81D , 0x9200 , 0x12  , 0x1009, 0x2406, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 82
   {"vpmultishiftqb",0x23,0x8E9200,0x19 , 0x1204, 0x1204, 0x204 , 0     , 0x21  , 0     , 0     , 0     },    // 0F 38 83
   {"vscaleps",  0x80  ,0x4B8200, 0x19  , 0x164B, 0x164B, 0x603 , 0     , 0     , 0x1306, 0     , 0x100 },    // 0F 38 84
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 85
   {"vpmulhud",  0x80  ,0x4A8200, 0x19  , 0x1603, 0x1603, 0x603 , 0     , 0     , 0x1406, 0     , 0x100 },    // 0F 38 86
   {"vpmulhd",   0x80  ,0x4A8200, 0x19  , 0x1603, 0x1603, 0x603 , 0     , 0     , 0x1406, 0     , 0x100 },    // 0F 38 87
   {"vexpandp",  0x20  ,0x801200, 0x12  , 0x164F, 0x064F, 0     , 0     , 0x1030, 0     , 0     , 1     },    // 0F 38 88
   {"vpexpand",  0x20  ,0x83B200, 0x12  , 0x1209, 0x0209, 0     , 0     , 0x30  , 0     , 0     , 1     },    // 0F 38 89
   {"vcompressp",0x20  ,0x809200, 0x13,   0x024F, 0x124F, 0     , 0     , 0x1030, 0     , 0     , 1     },    // 0F 38 8A
   {"vpcompress",0x20  ,0x80B200, 0x13,   0x0209, 0x1209, 0     , 0     , 0x1030, 0     , 0     , 1     },    // 0F 38 8B
   {"vpmaskmov", 0x1C  , 0xFB200, 0x19,   0x1209, 0x1209, 0x2209, 0     , 0     , 0     , 0     , 1     },    // 0F 38 8C
   {"vperm" ,    0x23  ,0x8EC200, 0x19,   0x1209, 0x1209, 0x209 , 0     , 0x20  , 0     , 0     , 1     },    // 0F 38 8D
   {"vpmaskmov", 0x1C  , 0xFB200, 0x1A,   0x2209, 0x1209, 0x1209, 0     , 0     , 0     , 0     , 1     },    // 0F 38 8E
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 8F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x102 , 0      , 0   ,   0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 38 90. link to vpgatherd/q
   {0,           0x94  , 0      , 0   ,   0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 38 91. Link to vpgatherqd/q
   {0,           0xB6  , 0      , 0   ,   0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 38 92. Link to vpgatherdps/pd
   {0,           0xE0  , 0      , 0   ,   0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 38 93. Link to vpgatherqps/pd
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 94
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 95
   {"vfmaddsub132p",0x1A,0x8F9200,0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0     , 0     , 0x1   },    // 0F 38 96
   {"vfmsubadd132p",0x1A,0x8F9200,0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0     , 0     , 0x1   },    // 0F 38 97
   {"vfmadd132p",0x1A  ,0xCF9200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 98
   {"vfmadd132s",0x1A  ,0x8B9200, 0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 99
   {"vfmsub132p",0x1A  ,0xCF9200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 9A
   {"vfmsub132s",0x1A  ,0x8B9200, 0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 9B
   {"vfnmadd132p",0x1A ,0xCF9200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 9C
   {"vfnmadd132s",0x1A ,0x8B9200, 0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 9D
   {"vfnmsub132p",0x1A ,0xCF9200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 9E
   {"vfnmsub132s",0x1A ,0x8B9200, 0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 9F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0xD7  , 0       ,0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 38 A0. Link to vpscatterdd
   {0,           0xD8  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 38 A1. Link to vpscatterqd
   {0,           0x100 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 38 A2. Link to vpscatterdps
   {0,           0x101 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 38 A3. Link to vpscatterqps
   {"vfmadd233ps",0x80 ,0x4F8200, 0x19  , 0x124B, 0x124B, 0x24B , 0     , 0     , 0x1316, 0     , 0x100 },    // 0F 38 A4
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 A5
   {"vfmaddsub213p",0x1A,0x8F9200,0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0     , 0     , 0x1   },    // 0F 38 A6
   {"vfmsubadd213p",0x1A,0x8F9200,0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0     , 0     , 0x1   },    // 0F 38 A7
   {"vfmadd213p",0x1A  ,0xCF9200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 A8
   {"vfmadd213s",0x1A  ,0x8B9200, 0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 A9
   {"vfmsub213p",0x1A  ,0xCF9200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 AA
   {"vfmsub213s",0x1A  ,0x8B9200, 0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 AB
   {"vfnmadd213p",0x1A ,0xCF9200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 AC
   {"vfnmadd213s",0x1A ,0x8B9200, 0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 AD
   {"vfnmsub213p",0x1A ,0xCF9200, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 AE
   {"vfnmsub213s",0x1A ,0x8B9200, 0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 AF

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"(reserved)",0x00  ,0x4D2E00, 0x401E, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 B0
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 B1
   {"(reserved)",0x00  ,0x4D2E00, 0x401E, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 B2
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 B3
   {0,           0x128 , 0      , 0x19  , 0     , 0     , 0     , 0     , 0     , 0     , 0x11  , 0     },    // 0F 38 B4. Link to vpmadd52luq
   {0,           0x129 , 0      , 0x19  , 0     , 0     , 0     , 0     , 0     , 0     , 0x11  , 0     },    // 0F 38 B5. Link to vpmadd52huq
   {"vfmaddsub231p",0x1A,0x8F9200,0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0     , 0     , 0x1   },    // 0F 38 B6
   {"vfmsubadd231p",0x1A,0x8F9200,0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0     , 0     , 0x1   },    // 0F 38 B7
   {"vfmadd231p",   0x1A,0xCF9200,0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 B8
   {"vfmadd231s",   0x1A,0x8B9200,0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 B9
   {"vfmsub231p",   0x1A,0xCF9200,0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 BA
   {"vfmsub231s",   0x1A,0x8B9200,0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 BB
   {"vfnmadd231p",  0x1A,0xCF9200,0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 BC
   {"vfnmadd231s",  0x1A,0x8B9200,0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 BD
   {"vfnmsub231p",  0x1A,0xCF9200,0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x37  , 0x1304, 0     , 0x1   },    // 0F 38 BE
   {"vfnmsub231s",  0x1A,0x8B9200,0x19  , 0x144F, 0x144F, 0x04F , 0     , 0x36  , 0     , 0     , 0x1   },    // 0F 38 BF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"(reserved)",0x0  ,0x4D2E00 , 0x401E, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 C0
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 C1
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 C2
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 C3
   {"vpconflict",0x21  ,0x80B200, 0x12  , 0x1209, 0x0209, 0     , 0     , 0x31  , 0     , 0     , 0x1   },    // 0F 38 C4
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 C5
   {0,           0xB7  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 38 C6. Link to vgatherpf0dps
   {0,           0x10F , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 0F 38 C7. Link to vgatherpf0qps
   {0,           0x107 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x11  , 0     },    // 0F 38 C8
   {0,           0x108 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x11  , 0     },    // 0F 38 C9
   {0,           0x109 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x11  , 0     },    // 0F 38 CA
   {0,           0x10A , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x11  , 0     },    // 0F 38 CB
   {0,           0x10B , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x11  , 0     },    // 0F 38 CC
   {0,           0x10C , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x11  , 0     },    // 0F 38 CD
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 CE
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 CF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0xBE  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 38 D0. Link to vloadunpackld
   {0,           0xBF  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 38 D1. Link to vloadunpacklps
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 D2
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 D3
   {0,           0xC0  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 38 D4. Link to vloadunpackhd
   {0,           0xC1  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 38 D5. Link to vloadunpackhps
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 D6
   {"(reserved)",0x00  ,0x4D2E00, 0x4012, 0x609 , 0x609 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 D7
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 D8
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 D9
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 DA
   {"aesimc",    0x17  ,0x98200 , 0x19,   0x1101, 0x1101, 0x101 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 DB
   {"aesenc",    0x17  ,0x98200 , 0x19,   0x1101, 0x1101, 0x101 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 DC
   {"aesenclast",0x17  ,0x98200 , 0x19,   0x1101, 0x1101, 0x101 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 DD
   {"aesdec",    0x17  ,0x98200 , 0x19,   0x1101, 0x1101, 0x101 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 DE
   {"aesdeclast",0x17  ,0x98200 , 0x19,   0x1101, 0x1101, 0x101 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 DF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 E0
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 E1
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 E2
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 E3
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 E4
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 E5
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 E6
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 E7
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 E8
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 E9
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 EA
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 EB
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 EC
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 ED
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 EE
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 EF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"crc32",     0x16  ,0x19900 , 0x12  , 0x1009, 0x1   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 F0
   {"crc32",     0x07  ,0x19900 , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // 0F 38 F1. Link to crc32 16/32/64 bit
   {"andn",      0x1D  ,0xB1000 , 0x19  , 0x1009, 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     },    // 0F 38 F2
   {"blsi",      0xA2  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 0F 38 F3. Link to blsi etc. by reg bit
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 F4
   {"bzhi",      0xA3  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 38 F5. Link to bzhi, pdep, pext
   {"mulx",      0xD0  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 38 F6. Link to mulx, adcx, adox
   {"bextr",     0xAD  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 38 F7. Link to bextr etc.
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 F8
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 F9
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 FA
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 FB
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 FC
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 FD
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 FE
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F 38 FF

// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 39
// Reserved by Intel for future extensions, but never used
SOpcodeDef OpcodeMap3[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2012, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F 39 00

// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 3A
// or VEX encoded with mmmm = 3
// Indexed by third opcode byte
SOpcodeDef OpcodeMap4[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vpermq",    0x1C  ,0x97B200, 0x52  , 0x1204, 0x204 , 0x31  , 0     , 0x31  , 0     , 0     , 0     },    // 0F 3A 00
   {"vpermpd",   0x1C  ,0x97B200, 0x52  , 0x124C, 0x24C , 0x31  , 0     , 0x31  , 0     , 0     , 0     },    // 0F 3A 01
   {"vpblendd",  0x1C  , 0xF8200, 0x59,   0x1203, 0x1203, 0x203 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 3A 02
   {"valign",    0x20  ,0xCAB200, 0x59,   0x1209, 0x1209, 0x209 , 0x31  , 0x21  , 0x1000, 0     , 0x101 },    // 0F 3A 03
   {"vpermilps", 0x19  ,0x8F8200, 0x52,   0x124B, 0x24B , 0x31  , 0     , 0x31  , 0     , 0     , 0     },    // 0F 3A 04
   {"vpermilpd", 0x19  ,0x8FA200, 0x52,   0x124C, 0x24C , 0x31  , 0     , 0x31  , 0     , 0     , 0     },    // 0F 3A 05
   {"vperm2f128",0x19  ,0x1F8200, 0x59,   0x1550, 0x1550, 0x550 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 3A 06
   {"vpermf32x4",0x80  ,0x438200, 0x52,   0x124B, 0x24B , 0x31  , 0     , 0     , 0x1000, 0     , 0x100 },    // 0F 3A 07
   {0,           0xF9  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 3A 08. Also in AMD instruction set   
   {0,           0xFA  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 3A 09. Also in AMD instruction set
   {0,           0xFB  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 3A 0A. Also in AMD instruction set
   {0,           0xFC  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 3A 0B. Also in AMD instruction set
   {"blendps",   0x15  , 0xD8200, 0x59  , 0x124B, 0x124B, 0x24B , 0x31  , 0     , 0     , 0     , 0x2   },    // 0F 3A 0C
   {"blendpd",   0x15  , 0xD8200, 0x59  , 0x124C, 0x124C, 0x24C , 0x31  , 0     , 0     , 0     , 0x2   },    // 0F 3A 0D
   {"pblendw",   0x15  , 0xD8200, 0x59  , 0x1202, 0x1202, 0x202 , 0x31  , 0     , 0     , 0     , 0x2   },    // 0F 3A 0E
   {"palignr",   0x14  ,0x8D2200, 0x59  , 0x1201, 0x1201, 0x201 , 0x31  , 0x20  , 0     , 0     , 0x2   },    // 0F 3A 0F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 10
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 11
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 12
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 13   
   {0,           0x61  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 3A 14. Link to pextrb
   {0,           0x62  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 3A 15. Link to pextrw
   {0,           0x63  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // 0F 3A 16. Link to pextrd, pextrq
   {"extractps" ,0x15  ,0x819200, 0x53  , 0x3   , 0x144B, 0x31  , 0     , 0     , 0     , 0     , 0x2   },    // 0F 3A 17
   {0           ,0x122 , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 3A 18
   {0           ,0x11E , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 3A 19. Link to vextractf128
   {0           ,0x124 , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 3A 1A. Link to vinsertf64x4
   {0,           0xDE  , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 3A 1B. Link to vextractf64x4
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 1C
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vcvtps2ph", 0x1D  ,0x878200, 0x53,   0xF4A , 0x250 , 0x31  , 0     , 0x22  , 0     , 0     , 0     },    // 0F 3A 1D
   {0,           0x114 , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0x6   , 0     },    // 0F 3A 1E. link to vpcmpud
   {0,           0x115 , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0x6   , 0     },    // 0F 3A 1F. link to vpcmpd
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0xA5  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 0F 3A 20. Link to pinsrb
   {"insertps",  0x15  ,0x898200, 0x59  , 0x144B, 0x144B, 0x4B  , 0x31  , 0     , 0     , 0     , 0x2   },    // 0F 3A 21
   {"pinsrd/q",  0x75  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // 0F 3A 22. Link to pinsrd/q
   {0,           0x8F  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 3A 23. Link to vshuff32x4
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 24
   {"vpternlog", 0x20  ,0x88B200, 0x59  , 0x1209, 0x1209, 0x0209, 0x31  , 0x31  , 0     , 0     , 0x1   },    // 0F 3A 25
   {"vgetmantp", 0x20  , 0xC29200,0x52  , 0x124F, 0x24F , 0x31  , 0     , 0x33  , 0x1204, 0     , 0x001 },    // 0F 3A 26
   {"vgetmants", 0x20  , 0xCA9200,0x59  , 0x144F, 0x24F , 0x04F , 0x31  , 0x32  , 0x1204, 0     , 0x001 },    // 0F 3A 27
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 28
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 29
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 2A
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 2B
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 2C
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 2D
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 2E
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 2F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"kshiftr",   0x20  , 0x3C200, 0x52  , 0x1095, 0x1095, 0x11  , 0     , 0     , 0     , 0     , 1     },    // 0F 3A 30
   {"kshiftr",   0x20  , 0x3B200, 0x52  , 0x1095, 0x1095, 0x11  , 0     , 0     , 0     , 0     , 1     },    // 0F 3A 30
   {"kshiftl",   0x20  , 0x3C200, 0x52  , 0x1095, 0x1095, 0x11  , 0     , 0     , 0     , 0     , 1     },    // 0F 3A 32
   {"kshiftl",   0x20  , 0x3B200, 0x52  , 0x1095, 0x1095, 0x11  , 0     , 0     , 0     , 0     , 1     },    // 0F 3A 33
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 34
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 35
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 36
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 37
   {0,           0x125 , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 3A 38. Link to vinserti128
   {0,           0x120 , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 3A 39. Link to vextracti128
   {0,           0x127 , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 3A 3A. Link to vinserti64x4
   {0,           0xDF  , 0      , 0x53 , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 3A 3B. Link to vextracti64x4
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 3C
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 3D
   {0,           0xC6  , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 0F 3A 3E. Link to kextract and vpcmp
   {0,           0x113 , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0x6   , 0     },    // 0F 3A 3F. Link to vpcmp
   //  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"dpps",      0x15  , 0xD8200, 0x59  , 0x124B, 0x124B, 0x24B , 0x31  , 0     , 0     , 0     , 0x2   },    // 0F 3A 40
   {"dppd",      0x15  , 0x98200, 0x59  , 0x144C, 0x144C, 0x44C , 0x31  , 0     , 0     , 0     , 0x2   },    // 0F 3A 41 (No ymm version)
   {0,           0x11D , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0x2   },    // 0F 3A 42. Link to mpsadbw
   {0,           0x90  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 3A 43. Link to vshufi32x4
   {"pclmulqdq", 0x18  , 0x98200, 0x59,   0x1404, 0x1404, 0x404 , 0x31  , 0     , 0     , 0     , 0x2   },    // 0F 3A 44
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 45
   {"vperm2i128",0x1C  , 0x1FB200,0x59  , 0x1506, 0x1506, 0x506 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 3A 46
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 47
   {"vpermil2ps",0x1005, 0xFF200, 0x5C,   0x124B, 0x124B, 0x24B , 0x24B , 0     , 0x31  , 0     , 0     },    // 0F 3A 48 AMD XOP
   {"vpermil2pd",0x1005, 0xFF200, 0x5C,   0x124C, 0x124C, 0x24C , 0x24C , 0     , 0x31  , 0     , 0     },    // 0F 3A 49 AMD XOP
   {"vblendvps", 0x19  , 0xF8200, 0x5C  , 0x124B, 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 3A 4A
   {"vblendvpd", 0x19  , 0xF8200, 0x5C  , 0x124C, 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 3A 4B
   {"vpblendvb", 0x19  , 0xF8200, 0x5C  , 0x1201, 0x1201, 0x201 , 0x201 , 0     , 0     , 0     , 0     },    // 0F 3A 4C
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 4D
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 4E
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 4F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vrangep",   0x20  ,0x8EB200, 0x59,   0x124F, 0x124F, 0x24F , 0x31  , 0x23  , 0     , 0     , 1     },    // 0F 3A 50
   {"vranges",   0x20  ,0x8EB200, 0x59,   0x144F, 0x144F, 0x44F , 0x31  , 0x33  , 0     , 0     , 1     },    // 0F 3A 50
   {"vrndfxpntp",0x80  , 0x4B9200,0x52  , 0x124F, 0x24F , 0x11  , 0     , 0     , 0x1204, 0     , 0x101 },    // 0F 3A 52
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 53
   {"vfixupimmp",0x20  ,0x881200, 0x59  , 0x124F, 0x124F, 0x024F, 0x31  , 0x33  , 0     , 0     , 1     },    // 0F 3A 54
   {"vfixupimms",0x20  ,0x8C1200, 0x59  , 0x104F, 0x104F, 0x004F, 0x31  , 0x32  , 0     , 0     , 1     },    // 0F 3A 55
   {"vreducep",  0x20  ,0x86B200, 0x52  , 0x124F, 0x24f , 0     , 0x31  , 0x23  , 0     , 0     , 1     },    // 0F 3A 56
   {"vreduces",  0x20  ,0x8EB200, 0x52  , 0x144F, 0x44f , 0     , 0x31  , 0x22  , 0     , 0     , 1     },    // 0F 3A 57
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 58
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 59
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 5A
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 5B
   {"vfmaddsubps",0x1006,0xFF200,0x5C   , 0x24B , 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 3A 5C
   {"vfmaddsubpd",0x1006,0xFF200,0x5C   , 0x24C , 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 3A 5D
   {"vfmsubaddps",0x1006,0xFF200,0x5C   , 0x24B , 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 3A 5E
   {"vfmsubaddpd",0x1006,0xFF200,0x5C   , 0x24C , 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 3A 5F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pcmpestrm", 0x16  , 0x18200, 0x52  , 0x1401, 0x451 , 0x31  , 0     , 0     , 0     , 0     , 0x202 },    // 0F 3A 60
   {"pcmpestri", 0x16  , 0x18200, 0x52  , 0x1401, 0x451 , 0x31  , 0     , 0     , 0     , 0     , 0x202 },    // 0F 3A 61
   {"pcmpistrm", 0x16  , 0x18200, 0x52  , 0x1401, 0x451 , 0x31  , 0     , 0     , 0     , 0     , 0x202 },    // 0F 3A 62
   {"pcmpistri", 0x16  , 0x18200, 0x52  , 0x1401, 0x451 , 0x31  , 0     , 0     , 0     , 0     , 0x202 },    // 0F 3A 63
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 64
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 65
   {"vfpclassp", 0x20  ,0x82B200, 0x52  , 0x95  , 0x24F , 0x31  , 0     , 0x10  , 0     , 0     , 1     },    // 0F 3A 66
   {"vfpclasss", 0x20  ,0x82B200, 0x52  , 0x95  , 0x44F , 0x31  , 0     , 0x10  , 0     , 0     , 1     },    // 0F 3A 67
// 4-operand FMA instructions. First specified by Intel, then Intel changed their plans, now supported by AMD
   {"vfmaddps",  0x1006,0xFF200 , 0x5C  , 0x24B , 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 3A 68
   {"vfmaddpd",  0x1006,0xFF200 , 0x5C  , 0x24C , 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 3A 69
   {"vfmaddss",  0x1006,0xBF200 , 0x5C  , 0x44B , 0x44B , 0x44B , 0x44B , 0     , 0     , 0     , 0     },    // 0F 3A 6A
   {"vfmaddsd",  0x1006,0xBF200 , 0x5C  , 0x44C , 0x44C , 0x44C , 0x44C , 0     , 0     , 0     , 0     },    // 0F 3A 6B
   {"vfmsubps",  0x1006,0xFF200 , 0x5C  , 0x24B , 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 3A 6C
   {"vfmsubpd",  0x1006,0xFF200 , 0x5C  , 0x24C , 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 3A 6D
   {"vfmsubss",  0x1006,0xBF200 , 0x5C  , 0x44B , 0x44B , 0x44B , 0x44B , 0     , 0     , 0     , 0     },    // 0F 3A 6E
   {"vfmsubsd",  0x1006,0xBF200 , 0x5C  , 0x44C , 0x44C , 0x44C , 0x44C , 0     , 0     , 0     , 0     },    // 0F 3A 6F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 70
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 71
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 72
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 73
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 74
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 75
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 76
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 77
   {"vfnmaddps", 0x1006,0xFF200 , 0x5C  , 0x24B , 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 3A 78
   {"vfnmaddpd", 0x1006,0xFF200 , 0x5C  , 0x24C , 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 3A 79
   {"vfnmaddss", 0x1006,0xBF200 , 0x5C  , 0x44B , 0x44B , 0x44B , 0x44B , 0     , 0     , 0     , 0     },    // 0F 3A 7A
   {"vfnmaddsd", 0x1006,0xBF200 , 0x5C  , 0x44C , 0x44C , 0x44C , 0x44C , 0     , 0     , 0     , 0     },    // 0F 3A 7B
   {"vfnmsubps", 0x1006,0xFF200 , 0x5C  , 0x24B , 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 3A 7C
   {"vfnmsubpd", 0x1006,0xFF200 , 0x5C  , 0x24C , 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 3A 7D
   {"vfnmsubss", 0x1006,0xBF200 , 0x5C  , 0x44B , 0x44B , 0x44B , 0x44B , 0     , 0     , 0     , 0     },    // 0F 3A 7E
   {"vfnmsubsd", 0x1006,0xBF200 , 0x5C  , 0x44C , 0x44C , 0x44C , 0x44C , 0     , 0     , 0     , 0     },    // 0F 3A 7F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 80
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 81
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 82
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 83
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 84
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 85
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 86
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 87
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 88
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 89
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 8A
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 8B
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 8C
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 8D
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 8E
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 8F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 90
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 91
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 92
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 93
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 94
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 95
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 96
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 97
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 98
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 99
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 9A
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 9B
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 9C
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 9D
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 9E
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 9F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A A0
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A A1
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A A2
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A A3
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A A4
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A A5
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A A6
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A A7
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A A8
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A A9
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A AA
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A AB
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A AC
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A AD
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A AE
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A AF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A B0
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A B1
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A B2
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A B3
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A B4
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A B5
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A B6
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A B7
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A B8
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A B9
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A BA
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A BB
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A BC
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A BD
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A BE
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A BF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A C0
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A C1
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A C2
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A C3
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A C4
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A C5
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A C6
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A C7
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A C8
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A C9
   {0,           0xB4  , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 3A CA. Link to vcvtfxpntpd2udq etc
   {0,           0xB5  , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 3A CB. Link to vcvtfxpntdq2ps etc
   {"sha1rnds4", 0x22  , 0      , 0x52  , 0x1403, 0x0403, 0x31  , 0     , 0     , 0     , 0     , 0     },    // 0F 3A CC
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A CD
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A CE
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A CF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"(reserved)",0x00  ,0x4D2E00, 0x4052, 0x609 , 0x609 , 0x31  , 0     , 0     , 0     , 0     , 0     },    // 0F 38 D0
   {"(reserved)",0x00  ,0x4D2E00, 0x4052, 0x609 , 0x609 , 0x31  , 0     , 0     , 0     , 0     , 0     },    // 0F 38 D1
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A D2
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A D3
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A D4
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A D5
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A D6
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A D7
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A D8
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A D9
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A DA
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A DB
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A DC
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A DD
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A DE
{"aeskeygenassist",0x17,0x18200 , 0x52,   0x1101, 0x101 , 0x31  , 0     , 0     , 0     , 0     , 0x2   },    // 0F 3A DF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A E0
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A E1
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A E2
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A E3
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A E4
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A E5
{"vcvtfxpntpd2dq",0x80 ,0x42B800, 0x52,   0x1603, 0x64C , 0x31  , 0     , 0     , 0x1205, 0     , 0x100 },    // 0F 3A E6
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A E7
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A E8
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A E9
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A EA
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A EB
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A EC
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A ED
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A EE
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A EF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0xA4  , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F 3A F0. Link to rorx
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A F1
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A F2
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A F3
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A F4
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A F5
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A F6
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A F7
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A F8
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A F9
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A FA
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A FB
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A FC
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A FD
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 3A FE
   {0,           0     , 0      , 0x2052, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F 3A FF


// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 3B
// Reserved by Intel for future extensions, but never used
SOpcodeDef OpcodeMap5[1] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2000, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F 3B 00


// Tertiary opcode map for AMD 3DNow instructions (obsolete). First two bytes = 0F 0F
// Indexed by immediate byte following operands
SOpcodeDef OpcodeMap6[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 00
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 01
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 02
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 03
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 04
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 05
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 06
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 07
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 08
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 09
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 0A
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 0B
   {"PFI2FW",    0x1001, 0      , 0x52  , 0x134B, 0x302 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 0C
   {"PI2FD",     0x1001, 0      , 0x52  , 0x134B, 0x303 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 0D
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 0E
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 0F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 10
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 11
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 12
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 13
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 14
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 15
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 16
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 17
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 18
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 19
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 1A
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 1B
   {"PF2IW",     0x1002, 0      , 0x52  , 0x1302, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 1C
   {"PF2ID",     0x1001, 0      , 0x52  , 0x1303, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 1D
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 1E
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 1F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 20
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 21
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 22
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 23
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 24
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 25
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 26
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 27
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 28
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 29
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 2A
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 2B
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 2C
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 2D
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 2E
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 2F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 30
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 31
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 32
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 33
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 34
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 35
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 36
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 37
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 38
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 39
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 3A
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 3B
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 3C
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 3D
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 3E
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 3F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 40
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 41
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 42
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 43
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 44
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 45
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 46
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 47
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 48
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 49
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 4A
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 4B
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 4C
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 4D
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 4E
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 4F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 50
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 51
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 52
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 53
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 54
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 55
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 56
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 57
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 58
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 59
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 5A
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 5B
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 5C
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 5D
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 5E
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 5F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 60
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 61
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 62
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 63
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 64
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 65
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 66
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 67
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 68
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 69
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 6A
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 6B
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 6C
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 6D
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 6E
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 6F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 70
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 71
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 72
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 73
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 74
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 75
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 76
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 77
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 78
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 79
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 7A
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 7B
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 7C
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 7D
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 7E
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 7F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 80
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 81
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 82
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 83
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 84
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 85
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 86
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 87
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 88
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 89
   {"PFNACC",    0x1002, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 8A
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 8B
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 8C
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 8D
   {"PFPNACC",   0x1002, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 8E
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 8F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"PFCMPGE",   0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 90
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 91
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 92
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 93
   {"PFMIN",     0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 94
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 95
   {"PFRCP",     0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 96
   {"PFRSQRT",   0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 97
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 98
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 99
   {"PFSUB",     0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 9A
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 9B
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 9C
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 9D
   {"PFADD",     0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 9E
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op 9F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"PFCMPGT",   0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op A0
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op A1
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op A2
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op A3
   {"PFMAX",     0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op A4
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op A5
   {"PFRCPIT1",  0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op A6
   {"PFRSQIT1",  0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op A7
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op A8
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op A9
   {"PFSUBR",    0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op AA
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op AB
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op AC
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op AD
   {"PFACC",     0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op AE
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op AF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"PFCMPEQ",   0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op B0
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op B1
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op B2
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op B3
   {"PFMUL",     0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op B4
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op B5
   {"PFRCPIT2",  0x1001, 0      , 0x52  , 0x134B, 0x34B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op B6
   {"PMULHRW",   0x1001, 0      , 0x52  , 0x1302, 0x302 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op B7
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op B8
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op B9
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op BA
   {"PSWAPD",    0x1002, 0      , 0x52  , 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op BB
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op BC
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op BD
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op BE
   {"PAVGUSB",   0x1001, 0      , 0x52  , 0x1301, 0x301 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0F op BF
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x1001, 0      , 0x2052, 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F 0F op C0

// Opcode map for crc32. Opcode byte = 0F 38 F1
// Indexed by operand size (16, 32, 64)
SOpcodeDef OpcodeMap7[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"crc32",     0x16  ,0x19900 , 0x12  , 0x1003, 0x2   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 F1
   {"crc32",     0x16  ,0x19900 , 0x12  , 0x1003, 0x3   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 F1
   {"crc32",     0x16  ,0x19900 , 0x12  , 0x1004, 0x4   , 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F 38 F1

// Secondary opcode map for x87 f.p. instructions. Opcode D8
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMap8[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fadd",      0x100 , 0      , 0x11  , 0     , 0x2043, 0     , 0     , 0     , 0     , 0     , 0     },    // fadd m32
   {"fmul",      0x100 , 0      , 0x11  , 0     , 0x2043, 0     , 0     , 0     , 0     , 0     , 0     },    // fmul m32
   {"fcom",      0x100 , 0      , 0x11  , 0     , 0x2043, 0     , 0     , 0     , 0     , 0     , 0x4   },    // fcom m32
   {"fcomp",     0x100 , 0      , 0x11  , 0     , 0x2043, 0     , 0     , 0     , 0     , 0     , 0x4   },    // fcomp m32
   {"fsub",      0x100 , 0      , 0x11  , 0     , 0x2043, 0     , 0     , 0     , 0     , 0     , 0     },    // fsub m32
   {"fsubr",     0x100 , 0      , 0x11  , 0     , 0x2043, 0     , 0     , 0     , 0     , 0     , 0     },    // fsubr m32
   {"fdiv",      0x100 , 0      , 0x11  , 0     , 0x2043, 0     , 0     , 0     , 0     , 0     , 0     },    // fdiv m32
   {"fdivr",     0x100 , 0      , 0x11  , 0     , 0x2043, 0     , 0     , 0     , 0     , 0     , 0     },    // fdivr m32
   {"fadd",      0x100 , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fadd st,st(i)
   {"fmul",      0x100 , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fmul st,st(i)
   {"fcom",      0x100 , 0      , 0x11  , 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0x4   },    // fcom st,st(i)
   {"fcomp",     0x100 , 0      , 0x11  , 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0x4   },    // fcomp st,st(i)
   {"fsub",      0x100 , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fsub st,st(i)
   {"fsubr",     0x100 , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fsubr st,st(i)
   {"fdiv",      0x100 , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fdiv st,st(i)
   {"fdivr",     0x100 , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     }};   // fdivr st,st(i)

// Secondary opcode map for x87 f.p. instructions. Opcode D9
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMap9[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fld",       0x100 , 0      , 0x11  , 0     , 0x2043, 0     , 0     , 0     , 0     , 0     , 0     },    // fld m32
   {0,           0     , 0      , 0x4011, 0     , 0x2043, 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {"fst",       0x100 , 0      , 0x11  , 0x2043, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fst m32
   {"fstp",      0x100 , 0      , 0x11  , 0x2043, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fstp m32
   {"fldenv",    0x100 , 0      , 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    // fldenv m
   {"fldcw",     0x100 , 0      , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // fldcw m16
   {"fnstenv",   0x100 , 0      , 0x11  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fnstenv m
   {"fnstcw",    0x100 , 0      , 0x11  , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fnstcw m16
   {"fld",       0x100 , 0      , 0x11  , 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fld st(i)
   {"fxch",      0x100 , 0      , 0x11  , 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fxch st(i)
   {"fnop",      0x10  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0x5   , 0     },    // Link to tertiary map 0x10 fnop
   {0,           0     , 0      , 0x4011, 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {0,           0x11  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x5   , 0     },    // Link to tertiary map 0x11 fchs etc.
   {0,           0x12  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x5   , 0     },    // Link to tertiary map 0x12 fld1 etc.
   {0,           0x13  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x5   , 0     },    // Link to tertiary map 0x13 f2xm1 etc.
   {0,           0x14  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x5   , 0     }};   // Link to tertiary map 0x14 fprem etc.

// Secondary opcode map for x87 f.p. instructions. Opcode DA
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapA[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fiadd",     0x100 , 0      , 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     },    // fiadd m32
   {"fimul",     0x100 , 0      , 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     },    // fimul m32
   {"ficom",     0x100 , 0      , 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0x4   },    // ficom m32
   {"ficomp",    0x100 , 0      , 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0x4   },    // ficomp m32
   {"fisub",     0x100 , 0      , 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     },    // fisub m32
   {"fisubr",    0x100 , 0      , 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     },    // fisubr m32
   {"fidiv",     0x100 , 0      , 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     },    // fidiv m32
   {"fidivr",    0x100 , 0      , 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     },    // fidivr m32
   {"fcmovb",    0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fcmovb st,st(i)
   {"fcmove",    0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fcmovb st,st(i)
   {"fcmovbe",   0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fcmovbe st,st(i)
   {"fcmovu",    0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fcmovbe st,st(i)
   {0,           0     , 0      , 0x4011, 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {0,           0x15  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x5   , 0     },    // Link to tertiary map 0x15 fucompp
   {0,           0     , 0      , 0x4011, 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {0,           0     , 0      , 0x4011, 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     }};   // Illegal

// Secondary opcode map for x87 f.p. instructions. Opcode DB
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapB[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fild",      0x100 , 0      , 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     },    // fild m32
   {"fisttp",    0x13  , 0      , 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     },    // fisttp m32
   {"fist",      0x100 , 0      , 0x11  , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fist m32
   {"fistp",     0x100 , 0      , 0x11  , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fistp m32
   {0,           0     , 0      , 0x4011, 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {"fld",       0x100 , 0      , 0x11  , 0     , 0x2045, 0     , 0     , 0     , 0     , 0     , 0     },    // fld m80
   {0,           0     , 0      , 0x4011, 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {"fstp",      0x100 , 0      , 0x11  , 0x2045, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fst m80
   {"fcmovnb",   0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fcmovnb st,st(i)
   {"fcmovne",   0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fcmovne st,st(i)
   {"fcmovnbe",  0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fcmovnbe st,st(i)
   {"fcmovnu",   0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fcmovnu st,st(i)
   {0,           0x16  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x5   , 0     },    // Link to tertiary map 0x16 fclex etc.
   {"fucomi",    0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0x4   },    // fucomi st,st(i)
   {"fcomi",     0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0x4   },    // fcomi st,st(i)
   {0,           0     , 0      , 0x4011, 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// Secondary opcode map for x87 f.p. instructions. Opcode DC
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapC[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fadd",      0x100 , 0      , 0x11  , 0     , 0x2044, 0     , 0     , 0     , 0     , 0     , 0     },    // fadd m64
   {"fmul",      0x100 , 0      , 0x11  , 0     , 0x2044, 0     , 0     , 0     , 0     , 0     , 0     },    // fmul m64
   {"fcom",      0x100 , 0      , 0x11  , 0     , 0x2044, 0     , 0     , 0     , 0     , 0     , 0x4   },    // fcom m64
   {"fcomp",     0x100 , 0      , 0x11  , 0     , 0x2044, 0     , 0     , 0     , 0     , 0     , 0x4   },    // fcomp m64
   {"fsub",      0x100 , 0      , 0x11  , 0     , 0x2044, 0     , 0     , 0     , 0     , 0     , 0     },    // fsub m64
   {"fsubr",     0x100 , 0      , 0x11  , 0     , 0x2044, 0     , 0     , 0     , 0     , 0     , 0     },    // fsubr m64
   {"fdiv",      0x100 , 0      , 0x11  , 0     , 0x2044, 0     , 0     , 0     , 0     , 0     , 0     },    // fdiv m64
   {"fdivr",     0x100 , 0      , 0x11  , 0     , 0x2044, 0     , 0     , 0     , 0     , 0     , 0     },    // fdivr m64
   {"fadd",      0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // fadd st(i),st
   {"fmul",      0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // fmul st(i),st
   {0,           0     , 0      , 0x4011, 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {0,           0     , 0      , 0x4011, 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {"fsubr",     0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // fsubr st(i),st
   {"fsub",      0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // fsub st(i),st
   {"fdivr",     0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // fdivr st(i),st
   {"fdiv",      0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     }};   // fdiv st(i),st

// Secondary opcode map for x87 f.p. instructions. Opcode DD
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapD[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fld",       0x100 , 0      , 0x11  , 0     , 0x2044, 0     , 0     , 0     , 0     , 0     , 0     },    // fld m64
   {"fisttp",    0x13  , 0      , 0x11  , 0x2004, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fisttp m64
   {"fst",       0x100 , 0      , 0x11  , 0x2044, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fst m64
   {"fstp",      0x100 , 0      , 0x11  , 0x2044, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fstp m64
   {"frstor",    0x100 , 0      , 0x11  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // frstor 108 bytes
   {0,           0     , 0      , 0x4011, 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {"fnsave",    0x100 , 0      , 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    // fnsave 108 bytes
   {"fnstsw",    0x100 , 0      , 0x11  , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fstsw m16
   {"ffree",     0x100 , 0      , 0x11  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // ffree st(i)
   {0,           0     , 0      , 0x4011, 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {"fst",       0x100 , 0      , 0x11  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fst st(i)
   {"fstp",      0x100 , 0      , 0x11  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fstp st(i)
   {"fucom",     0x101 , 0      , 0x11  , 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fucom st(i)
   {"fucomp",    0x101 , 0      , 0x11  , 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // fucomp st(i)
   {0,           0     , 0      , 0x4011, 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {0,           0     , 0      , 0x4011, 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// Secondary opcode map for x87 f.p. instructions. Opcode DE
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapE[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fiadd",     0x100 , 0      , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // fiadd m16
   {"fimul",     0x100 , 0      , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // fimul m16
   {"ficom",     0x100 , 0      , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0x4   },    // ficom m16
   {"ficomp",    0x100 , 0      , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0x4   },    // ficomp m16
   {"fisub",     0x100 , 0      , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // fisub m16
   {"fisubr",    0x100 , 0      , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // fisubr m16
   {"fidiv",     0x100 , 0      , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // fidiv m16
   {"fidivr",    0x100 , 0      , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // fidivr m16
   {"faddp",     0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // faddp st(i),st
   {"fmulp",     0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // fmulp st(i),st
   {0,           0     , 0      , 0x4011, 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {0,           0x17  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x5   , 0     },    // Link to tertiary map 0x17 fcompp
   {"fsubrp",    0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // fsubrp st(i),st (Yes, the order is illogical here)
   {"fsubp",     0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // fsubp st(i),st
   {"fdivrp",    0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     },    // fdivrp st(i),st
   {"fdivp",     0x100 , 0      , 0x11  , 0x1040, 0xAF  , 0     , 0     , 0     , 0     , 0     , 0     }};   // fdivp st(i),st

// Secondary opcode map for x87 f.p. instructions. Opcode DF
// Indexed by reg bits and mod == 3
SOpcodeDef OpcodeMapF[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fild",      0x100 , 0      , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // fild m16
   {"fisttp",    0x13  , 0      , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // fisttp m16
   {"fist",      0x100 , 0      , 0x11  , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fist m16
   {"fistp",     0x100 , 0      , 0x11  , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fistp m16
   {"fbld",      0x100 , 0      , 0x11  , 0     , 0x2005, 0     , 0     , 0     , 0     , 0     , 0     },    // fbld m80
   {"fild",      0x100 , 0      , 0x11  , 0     , 0x2004, 0     , 0     , 0     , 0     , 0     , 0     },    // fild m64
   {"fbstp",     0x100 , 0      , 0x11  , 0x2005, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fbstp m80
   {"fistp",     0x100 , 0      , 0x11  , 0x2004, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fistp m64
   {0,           0     , 0      , 0x4011, 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {0,           0     , 0      , 0x4011, 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {0,           0     , 0      , 0x4011, 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {0,           0     , 0      , 0x4011, 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {0,           0x18  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x5   , 0     },    // Link to tertiary map 0x18 fnstsw ax
   {"fucomip",   0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0x4   },    // fucomp st,st(i)
   {"fcomip",    0x6   , 0      , 0x11  , 0xAF  , 0x1040, 0     , 0     , 0     , 0     , 0     , 0x4   },    // fcomp st,st(i)
   {0,           0     , 0      , 0x4011, 0     , 0x1040, 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// Tertiary opcode map for f.p. D9 / reg = 010
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap10[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fnop",      0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x40  },    // fnop
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // the rest is illegal

// Tertiary opcode map for f.p. D9 / reg = 100
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap11[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fchs",      0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fchs
   {"fabs",      0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fabs
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {"ftst",      0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // ftst
   {"fxam",      0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fxam
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// Tertiary opcode map for f.p. D9 / reg = 101
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap12[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fld1",      0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // fld1
   {"fldl2t",    0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fldl2e",    0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fldpi",     0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fldlg2",    0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fldln2",    0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fldz",      0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// Tertiary opcode map for f.p. D9 / reg = 110
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap13[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"f2xm1",     0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // f2xm1
   {"fyl2x",     0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fptan",     0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fpatan",    0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fxtract",   0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fprem1",    0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fdecstp",   0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fincstp",   0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 

// Tertiary opcode map for f.p. D9 / reg = 111
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap14[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fprem",     0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fyl2xp1",   0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fsqrt",     0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fsincos",   0x101 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"frndint",   0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fscale",    0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fsin",      0x101 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fcos",      0x101 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 

// Tertiary opcode map for f.p. DA / reg = 101
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap15[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {"fucompp",   0x101 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// Tertiary opcode map for f.p. DB / reg = 100
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap16[5] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {"fnclex",    0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"fninit",    0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// Tertiary opcode map for f.p. DE / reg = 011
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap17[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {"fcompp",    0x100 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   },    // 
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// Tertiary opcode map for f.p. DF / reg = 100
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap18[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fnstsw",    0x100 , 0      , 0x10  , 0xA2  , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// Secondary opcode map for IRET. Opcode byte = 0xCF
// Indexed by operand size
SOpcodeDef OpcodeMap19[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"iret",      0     , 0x102  , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x10  },    // CF
   {"iretd",     0     , 0x102  , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x10  },    // CF
   {"iretq",     0     , 0x1102 , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x10  }};   // CF

// Secondary opcode map for immediate group 1. Opcode byte = 0x80
// Indexed by reg bits = 0 - 7
SOpcodeDef OpcodeMap1A[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"add",       0     , 0x10   , 0x51  , 0x1   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 80 /0
   {"or",        0     , 0x10   , 0x51  , 0x1   , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 80 /1
   {"adc",       0     , 0x10   , 0x51  , 0x1   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 80 /2
   {"sbb",       0     , 0x10   , 0x51  , 0x1   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 80 /3
   {"and",       0     , 0x10   , 0x51  , 0x1   , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 80 /4
   {"sub",       0     , 0x10   , 0x51  , 0x1   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 80 /5
   {"xor",       0     , 0x10   , 0x51  , 0x1   , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 80 /6
   {"cmp",       0     , 0      , 0x51  , 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0x4   }};   // 80 /7

// Secondary opcode map for immediate group 1. Opcode byte = 0x81
// Indexed by reg bits = 0 - 7
SOpcodeDef OpcodeMap1B[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"add",       0     , 0x1110 , 0x91  , 0x9   , 0x28  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 81 /0
   {"or",        0     , 0x1110 , 0x91  , 0x9   , 0x39  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 81 /1
   {"adc",       0     , 0x1110 , 0x91  , 0x9   , 0x28  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 81 /2
   {"sbb",       0     , 0x1110 , 0x91  , 0x9   , 0x28  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 81 /3
   {"and",       0     , 0x1110 , 0x91  , 0x9   , 0x39  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 81 /4
   {"sub",       0     , 0x1110 , 0x91  , 0x9   , 0x28  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 81 /5
   {"xor",       0     , 0x1110 , 0x91  , 0x9   , 0x39  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // 81 /6
   {"cmp",       0     , 0x1100 , 0x91  , 0x9   , 0x28  , 0     , 0     , 0     , 0     , 0     , 0x84  }};   // 81 /7

// Secondary opcode map for immediate group 1. Opcode byte = 0x82.
// Undocumented opcode. Signed byte instructions do the same as unsigned byte instructions at 0x80
SOpcodeDef OpcodeMap1C[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"add",       0x8000, 0x10   , 0x4051, 0x1   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 82 /0
   {"or",        0x8000, 0x10   , 0x4051, 0x1   , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 82 /1
   {"adc",       0x8000, 0x10   , 0x4051, 0x1   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 82 /2
   {"sbb",       0x8000, 0x10   , 0x4051, 0x1   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 82 /3
   {"and",       0x8000, 0x10   , 0x4051, 0x1   , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 82 /4
   {"sub",       0x8000, 0x10   , 0x4051, 0x1   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 82 /5
   {"xor",       0x8000, 0x10   , 0x4051, 0x1   , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 82 /6
   {"cmp",       0x8000, 0      , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0x4   }};   // 82 /7

// Secondary opcode map for immediate group 1. Opcode byte = 0x83
// Indexed by reg bits = 0 - 7
SOpcodeDef OpcodeMap1D[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"add",       0     , 0x1110 , 0x51  , 0x9   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 83 /0
   {"or",        0     , 0x1110 , 0x51  , 0x9   , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 83 /1
   {"adc",       0     , 0x1110 , 0x51  , 0x9   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 83 /2
   {"sbb",       0     , 0x1110 , 0x51  , 0x9   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 83 /3
   {"and",       0     , 0x1110 , 0x51  , 0x9   , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 83 /4
   {"sub",       0     , 0x1110 , 0x51  , 0x9   , 0x21  , 0     , 0     , 0     , 0     , 0     , 0     },    // 83 /5
   {"xor",       0     , 0x1110 , 0x51  , 0x9   , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     },    // 83 /6
   {"cmp",       0     , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0x4   }};   // 83 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xC0
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap1E[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"rol",       0     , 0      , 0x51  , 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C0 /0 rol byte ptr r/m,ib
   {"ror",       0     , 0      , 0x51  , 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C0 /1
   {"rcl",       0     , 0      , 0x51  , 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C0 /2
   {"rcr",       0     , 0      , 0x51  , 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C0 /3
   {"shl",       0     , 0      , 0x51  , 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C0 /4
   {"shr",       0     , 0      , 0x51  , 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C0 /5
   {"sal",       0     , 0      , 0x51  , 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C0 /6
   {"sar",       0     , 0      , 0x51  , 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     }};   // C0 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xC1
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap1F[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"rol",       0     , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C1 /0 rol word ptr r/m,ib
   {"ror",       0     , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C1 /1
   {"rcl",       0     , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C1 /2
   {"rcr",       0     , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C1 /3
   {"shl",       0     , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C1 /4
   {"shr",       0     , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C1 /5
   {"sal",       0     , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C1 /6
   {"sar",       0     , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     }};   // C1 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xD0
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap20[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"rol",       0     , 0      , 0x11  , 0x1   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /0 rol byte ptr r/m,1
   {"ror",       0     , 0      , 0x11  , 0x1   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /1
   {"rcl",       0     , 0      , 0x11  , 0x1   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /2
   {"rcr",       0     , 0      , 0x11  , 0x1   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /3
   {"shl",       0     , 0      , 0x11  , 0x1   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /4
   {"shr",       0     , 0      , 0x11  , 0x1   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /5
   {"sal",       0     , 0      , 0x11  , 0x1   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /6
   {"sar",       0     , 0      , 0x11  , 0x1   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     }};   // C2 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xD1
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap21[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"rol",       0     , 0x1100 , 0x11  , 0x9   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /0 rol word ptr r/m,1
   {"ror",       0     , 0x1100 , 0x11  , 0x9   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /1
   {"rcl",       0     , 0x1100 , 0x11  , 0x9   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /2
   {"rcr",       0     , 0x1100 , 0x11  , 0x9   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /3
   {"shl",       0     , 0x1100 , 0x11  , 0x9   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /4
   {"shr",       0     , 0x1100 , 0x11  , 0x9   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /5
   {"sal",       0     , 0x1100 , 0x11  , 0x9   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /6
   {"sar",       0     , 0x1100 , 0x11  , 0x9   , 0xB1  , 0     , 0     , 0     , 0     , 0     , 0     }};   // C3 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xD2
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap22[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"rol",       0     , 0      , 0x11  , 0x1   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /0 rol byte ptr r/m,cl
   {"ror",       0     , 0      , 0x11  , 0x1   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /1
   {"rcl",       0     , 0      , 0x11  , 0x1   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /2
   {"rcr",       0     , 0      , 0x11  , 0x1   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /3
   {"shl",       0     , 0      , 0x11  , 0x1   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /4
   {"shr",       0     , 0      , 0x11  , 0x1   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /5
   {"sal",       0     , 0      , 0x11  , 0x1   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C2 /6
   {"sar",       0     , 0      , 0x11  , 0x1   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     }};   // C2 /7

// Secondary opcode map for shift group 2. Opcode byte = 0xD3
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap23[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"rol",       0     , 0x1100 , 0x11  , 0x9   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /0 rol word ptr r/m,cl
   {"ror",       0     , 0x1100 , 0x11  , 0x9   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /1
   {"rcl",       0     , 0x1100 , 0x11  , 0x9   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /2
   {"rcr",       0     , 0x1100 , 0x11  , 0x9   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /3
   {"shl",       0     , 0x1100 , 0x11  , 0x9   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /4
   {"shr",       0     , 0x1100 , 0x11  , 0x9   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /5
   {"sal",       0     , 0x1100 , 0x11  , 0x9   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     },    // C3 /6
   {"sar",       0     , 0x1100 , 0x11  , 0x9   , 0xB3  , 0     , 0     , 0     , 0     , 0     , 0     }};   // C3 /7

// Secondary opcode map for group 3. Opcode byte = 0xF6
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap24[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"test",      0     , 0      , 0x51  , 0x1   , 0x31  , 0     , 0     , 0     , 0     , 0     , 0x4   },    // test rm8,ib
   {"test",      0     , 0      , 0x4051, 0x1   , 0x31  , 0     , 0     , 0     , 0     , 0     , 0x4   },    // test rm8,ib. undocumented
   {"not",       0     , 0x1C50 , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // not rm8
   {"neg",       0     , 0x1C50 , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // neg rm8
   {"mul",       0     , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // mul rm8
   {"imul",      0     , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // imul rm8
   {"div",       0     , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // div rm8
   {"idiv",      0     , 0      , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   }};   // idiv rm8

// Secondary opcode map for group 3. Opcode byte = 0xF7
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap25[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"test",      0     , 0x1100 , 0x91  , 0x9   , 0x39  , 0     , 0     , 0     , 0     , 0     , 0x4   },    // test rm,i
   {"test",      0     , 0x1100 , 0x4091, 0x9   , 0x39  , 0     , 0     , 0     , 0     , 0     , 0x4   },    // test rm,i. undocumented
   {"not",       0     , 0x1D50 , 0x11  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // not rm
   {"neg",       0     , 0x1D50 , 0x11  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // neg rm
   {"mul",       0     , 0x1100 , 0x11  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // mul rm
   {"imul",      0     , 0x1100 , 0x11  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // imul rm
   {"div",       0     , 0x1100 , 0x11  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // div rm
   {"idiv",      0     , 0x1100 , 0x11  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   }};   // idiv rm

// Secondary opcode map for group 4. Opcode byte = 0xFE
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap26[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"inc",       0     , 0xC50  , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // inc rm8
   {"dec",       0     , 0xC50  , 0x11  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // dec rm8
   {0,           0     , 0      , 0x4011, 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal opcode

// Secondary opcode map for group 5. Opcode byte = 0xFF
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap27[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"inc",       0     , 0x1D50 , 0x11  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0x80  },    // inc rm
   {"dec",       0     , 0x1D50 , 0x11  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0x80  },    // dec rm
   {"call",      0     , 0x2182 , 0x11  , 0xC   , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // call indirect rm
   {"call",      0     , 0x1102 , 0x811 , 0x200D, 0     , 0     , 0     , 0     , 0     , 0     , 0x28  },    // call indirect far
   {"jmp",       0     , 0x2180 , 0x11  , 0xB   , 0     , 0     , 0     , 0     , 0     , 0     , 0x14  },    // jmp indirect rm
   {"jmp",       0     , 0x1100 , 0x811 , 0x200D, 0     , 0     , 0     , 0     , 0     , 0     , 0x30  },    // jmp indirect far
   {"push",      0     , 0x2102 , 0x11  , 0xA   , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   },    // push rm
   {0,           0     , 0      , 0x4011, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal opcode

// Secondary opcode map for immediate group 1A. Opcode byte = 0x8F
// Indexed by reg bits = 0 - 7. Values != 0 are discouraged
SOpcodeDef OpcodeMap28[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pop",       0     , 0x2102 , 0x11  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 8F
   {"pop",       0     , 0x2102 , 0x4011, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 8F

// Tertiary opcode map for pinsrw. Opcode byte = 0F C4
// Indexed by mod bits 0 register vs. memory operand
SOpcodeDef OpcodeMap29[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pinsrw",    0x7   ,0x892200, 0x59  , 0x1102, 0x1102, 0x2002, 0x11  , 0x1000, 0     , 0     , 0x2   },    // 0F C4 mem16
   {"pinsrw",    0x7   ,0x892200, 0x59  , 0x1102, 0x1102, 0x1009, 0x11  , 0     , 0     , 0     , 0x2   }};   // 0F C4 register

// Tertiary opcode map for group 6. Opcode byte = 0F 00
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap2A[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"sldt",      0x2   , 0x1100 , 0x11  , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"str",       0x802 , 0x100  , 0x11  , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"lldt",      0x802 , 0x2000 , 0x11  , 0     , 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"ltr",       0x802 , 0      , 0x11  , 0     , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"verr",      0x802 , 0      , 0x11  , 0     , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"verw",      0x802 , 0      , 0x11  , 0     , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0x4011, 0     , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {0,           0     , 0      , 0x4011, 0     , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// Tertiary opcode map for group 7. Opcode byte = 0F 01
// Indexed by reg bits = 0 - 7 and mod = 11b. 
SOpcodeDef OpcodeMap2B[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"sgdt",      0x802 , 0x1100 , 0x11  , 0x200D, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // mod<3, reg=0
   {"sidt",      0x802 , 0x1100 , 0x11  , 0x200D, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"lgdt",      0x802 , 0x1100 , 0x11  , 0     , 0x200D, 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"lidt",      0x802 , 0x1100 , 0x11  , 0     , 0x200D, 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"smsw",      0x2   , 0      , 0x11  , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0x133 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 9     , 0     },    // link to rstorssp
   {"lmsw",      0x802 , 0      , 0x11  , 0     , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"invlpg",    0x4   , 0      , 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    // mod<3, reg=7

   {0,           0x36  , 0      , 0x4011, 0     , 0     , 0     , 0     , 0     , 0     , 5     , 0     },    // link to quarternary map, vmcall etc.
   {0,           0x37  , 0      , 0x4011, 0     , 0     , 0     , 0     , 0     , 0     , 5     , 0     },    // link to quarternary map, monitor, mwait
   {0,           0xA9  , 0      , 0x4011, 0     , 0     , 0     , 0     , 0     , 0     , 5     , 0     },    // link to quarternary map, xgetbv, xsetbv
   {0,           0xAA  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 5     , 0     },    // link AMD virtualization
   {"smsw",      0x2   , 0x1100 , 0x11  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0x130 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 5     , 0     },    // link to incssp etc.
   {"lmsw",      0x802 , 0      , 0x11  , 0     , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0xAB  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 5     , 0     }};   // link SWAPGS and RDTSCP

// Secondary opcode map for group 8. Opcode byte = 0F BA: bt
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap2C[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x51  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {0,           0     , 0      , 0x51  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {0,           0     , 0      , 0x51  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {0,           0     , 0      , 0x51  , 0x9   , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {"bt",        0x3   , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"bts",       0x3   , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"btr",       0x3   , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"btc",       0x3   , 0x1100 , 0x51  , 0x9   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     }};   // 

// Secondary opcode map for addsub. Opcode byte = 0F D0
// Indexed by prefix = none, 66, F2, F3
SOpcodeDef OpcodeMap2D[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"addsub",    0x13  , 0xD0000, 0x4019, 0x124F, 0x124F, 0x24F , 0     , 0     , 0     , 0     , 0x2   },    // 0F D0. undefined
   {"addsubpd",  0x13  , 0xD0200, 0x19  , 0x124C, 0x124C, 0x24C , 0     , 0     , 0     , 0     , 0x2   },    // 66 0F D0. addsubpd
   {"addsubps",  0x13  , 0xD0800, 0x19  , 0x124B, 0x124B, 0x24B , 0     , 0     , 0     , 0     , 0x2   },    // F2 0F D0. addsubps
   {"addsub",    0x13  , 0xD0400, 0x4019, 0x124F, 0x124F, 0x24F , 0     , 0     , 0     , 0     , 0x2   }};   // F3 0F D0. undefined

// Secondary opcode map for group 10. Opcode byte = 0F B9
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap2E[1] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"ud1",       0     , 0      , 0x4012, 0x1009, 0x6   , 0     , 0     , 0     , 0     , 0     , 0     }};   // Invalid opcode, possibly used for emulation

// Secondary opcode map for mov group 11. Opcode byte = 0xC6
// Indexed by reg bits and mod.
SOpcodeDef OpcodeMap2F[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"mov",       0     , 0xC45  , 0x51  , 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 m /0 mov m,ib
   {"mov",       0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 m /1
   {"mov",       0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 m /2
   {"mov",       0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 m /3
   {"mov",       0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 m /4
   {"mov",       0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 m /5
   {"mov",       0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 m /6
   {"mov",       0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 m /7
   {"",          0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 r /0
   {"",          0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 r /1
   {"",          0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 r /2
   {"",          0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 r /3
   {"",          0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 r /4
   {"",          0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 r /5
   {"",          0     , 0x5    , 0x4051, 0x1   , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     },    // C6 r /6
   {"xabort",    0x1D  , 0      , 0x50  , 0     , 0x31  , 0     , 0     , 0     , 0     , 0     , 0     }};   // C6 r /7

// Secondary opcode map for mov group 11. Opcode byte = 0xC7
// Indexed by reg bits and mod.
SOpcodeDef OpcodeMap30[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"mov",       0     , 0x1D45 , 0x91  , 0x9   , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 m /0 mov m,iw
   {"mov",       0     , 0x5    , 0x4091, 0x9   , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 m /1
   {"mov",       0     , 0x5    , 0x4091, 0x9   , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 m /2
   {"mov",       0     , 0x5    , 0x4091, 0x9   , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 m /3
   {"mov",       0     , 0x5    , 0x4091, 0x9   , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 m /4
   {"mov",       0     , 0x5    , 0x4091, 0x9   , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 m /5
   {"mov",       0     , 0x5    , 0x4091, 0x9   , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 m /6
   {"mov",       0     , 0x5    , 0x4091, 0x9   , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 m /7
   {"mov",       0     , 0x1105 , 0x91  , 0x9   , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 r /0 mov r,iw
   {"",          0     , 0      , 0x91  , 0     , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 r /1
   {"",          0     , 0      , 0x91  , 0     , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 r /2
   {"",          0     , 0      , 0x91  , 0     , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 r /3
   {"",          0     , 0      , 0x91  , 0     , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 r /4
   {"",          0     , 0      , 0x91  , 0     , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 r /5
   {"",          0     , 0      , 0x91  , 0     , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     },    // C7 r /6
   {"xbegin",    0x1D  , 0x100  , 0x90  , 0     , 0x29  , 0     , 0     , 0     , 0     , 0     , 0     }};   // C7 r /7

// Secondary opcode map for group 12. Opcode byte = 0F 71
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap31[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x7   , 0x90200, 0x58  , 0x1102, 0x1102, 0x11  , 0     , 0     , 0     , 0     , 0x2   },     // Illegal
   {0,           0x7   , 0x90200, 0x58  , 0x1102, 0x1102, 0x11  , 0     , 0     , 0     , 0     , 0x2   },     // Illegal
   {"psrlw",     0x7   ,0x8D2200, 0x58  , 0x1102, 0x1102, 0x11  , 0     , 0x20  , 0     , 0     , 0x2   },     // 2
   {0,           0x7   , 0x90200, 0x58  , 0x1102, 0x1102, 0x11  , 0     , 0     , 0     , 0     , 0x2   },     // Illegal
   {"psraw",     0x7   ,0x8D2200, 0x58  , 0x1102, 0x1102, 0x11  , 0     , 0x20  , 0     , 0     , 0x2   },     // 4
   {0,           0x7   , 0x90200, 0x58  , 0x1102, 0x1102, 0x11  , 0     , 0     , 0     , 0     , 0x2   },     // Illegal
   {"psllw",     0x7   ,0x8D2200, 0x58  , 0x1102, 0x1102, 0x11  , 0     , 0x20  , 0     , 0     , 0x2   },     // 6
   {0,           0x7   , 0x90200, 0x58  , 0x1102, 0x1102, 0x11  , 0     , 0     , 0     , 0     , 0x2   }};    // Illegal

// Secondary opcode map for group 13. Opcode byte = 0F 72
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap32[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vpror",     0x20  ,0x893200, 0x58  , 0x1209, 0x209 , 0x11  , 0     , 0x21  , 0     , 0     , 0x1   },     // /0
   {"vprol",     0x20  ,0x893200, 0x58  , 0x1209, 0x209 , 0x11  , 0     , 0x21  , 0     , 0     , 0x1   },     // /1
   {"psrld",     0x12  ,0xCD3200, 0x58  , 0x1103, 0x103 , 0x11  , 0     , 0x21  , 0x1406, 0     , 0x2   },     // /2
   {0,           0x12  , 0x90200, 0x58  , 0x1103, 0x103 , 0x11  , 0     , 0     , 0     , 0     , 0x2   },     // Illegal
   {"psra",      0x12  ,0xCD3200, 0x58  , 0x1109, 0x109 , 0x11  , 0     , 0x31  , 0x1406, 0     , 0x3   },     // /4. W bit controls operand size only if EVEX
   {0,           0x12  , 0x90200, 0x58  , 0x1103, 0x103 , 0x11  , 0     , 0     , 0     , 0     , 0x2   },     // Illegal
   {"pslld",     0x12  ,0xCD3200, 0x58  , 0x1103, 0x103 , 0x11  , 0     , 0x21  , 0x1406, 0     , 0x2   },     // /6
   {0,           0x12  , 0x90200, 0x58  , 0x1103, 0x103 , 0x11  , 0     , 0     , 0     , 0     , 0x2   }};    // Illegal

// Secondary opcode map for group 14. Opcode byte = 0F 73
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap33[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x12  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // /0 Illegal
   {0,           0x12  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // /1 Illegal
   {"psrlq",     0x12  ,0x8D3200, 0x58  , 0x1104, 0x104 , 0x11  , 0     , 0x21  , 0     , 0     , 0x2   },     // /2
   {"psrldq",    0x12  , 0xDA200, 0x58  , 0x1204, 0x204 , 0x11  , 0     , 0     , 0     , 0     , 0x2   },     // /3 Not valid without 66 prefix
   {0,           0x12  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // /4 Illegal
   {0,           0x12  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // /5 Illegal
   {"psllq",     0x12  ,0x8D3200, 0x58  , 0x1104, 0x104 , 0x11  , 0     , 0x21  , 0     , 0     , 0x2   },     // /6 
   {"pslldq",    0x12  ,0x8DA200, 0x58  , 0x1204, 0x204 , 0x11  , 0     , 0     , 0     , 0     , 0x2   }};    // /7 Not valid without 66 prefix

// Secondary opcode map for group 15. Opcode byte = 0F AE 
// Indexed by reg bits = 0 - 7 and mod = 3 
// These codes are without VEX prefix. Same codes with VEX or MVEX prefix are in OpcodeMapCD
SOpcodeDef OpcodeMap34[16] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fxsave",    0x11  , 0      , 0x11  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // 0F AE /0
   {"fxrstor",   0x11  , 0      , 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0x8   },     // 0F AE /1
   {"ldmxcsr",   0x11  , 0x10000, 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0x2   },     // 0F AE /2
   {"stmxcsr",   0x11  , 0x10000, 0x11  , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     , 0x2   },     // 0F AE /3
   {0,           0     , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // Illegal
   {0,           0x134 , 0      , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     , 9     , 0     },     // 0F AE /5. Link setssbsy
   {0,           0xF3  , 0      , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     , 9     , 0     },     // 0F AE /6. Link xsaveopt
   {0,           0xF2  , 0      , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     , 9     , 0     },     // 0F AE /7. Link clflush
   {"rdfsbase",  0x10000,0x1400 , 0x11  , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // F3 0F AE m-0
   {"rdgsbase",  0x10000,0x1400 , 0x11  , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // F3 0F AE m-1
   {"wrfsbase",  0x10000,0x1400 , 0x11  , 0     , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },     // F3 0F AE m-2
   {"wrgsbase",  0x10000,0x1400 , 0x11  , 0     , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },     // F3 0F AE m-3
   {0,           0     , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // Illegal
   {"lfence",    0x12  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // m-5
   {"mfence",    0x12  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // m-6
   {0,           0xF4  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 9     , 0     }};    // m-7. Link sfence, pcommit

// Secondary opcode map for group 16. Opcode byte = 0F 18
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap35[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"prefetchnta",0x13 ,0x410000, 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0x2   , 0     , 0x2   },    // 0F 18 /0
   {"prefetcht0",0x13  ,0x410000, 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0x2   , 0     , 0x2   },    // 0F 18 /1
   {"prefetcht1",0x13  ,0x410000, 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0x2   , 0     , 0x2   },    // 0F 18 /2
   {"prefetcht2",0x13  ,0x410000, 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0x2   , 0     , 0x2   },    // 0F 18 /3
   {"vprefetchenta",0x13,0x430000,0x11  , 0     , 0x2006, 0     , 0     , 0     , 0x2   , 0     , 0     },    // 0F 18 /4
   {"vprefetche0",0x13  ,0x430000,0x11  , 0     , 0x2006, 0     , 0     , 0     , 0x2   , 0     , 0     },    // 0F 18 /5
   {"vprefetche1",0x13  ,0x430000,0x11  , 0     , 0x2006, 0     , 0     , 0     , 0x2   , 0     , 0     },    // 0F 18 /6
   {"vprefetche2",0x13  ,0x430000,0x11  , 0     , 0x2006, 0     , 0     , 0     , 0x2   , 0     , 0     }};   // 0F 18 /7

// Quarternary opcode map for group 7. 0F 01 reg = 0
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap36[6] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // illegal
   {"vmcall",    0x813 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // Intel processor only?
   {"vmlaunch",  0x813 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // Intel processor only?
   {"vmresume",  0x813 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Intel processor only?
   {"vmxoff",    0x813 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Intel processor only?
   {0,           0     , 0      , 0x4010, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// Quarternary opcode map for group 7. 0F 01 reg = 1
// Indexed by rm bits of mod/reg/rm byte
SOpcodeDef OpcodeMap37[5] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"monitor",   0x813 , 0x4    , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 1 - 0
   {"mwait",     0x813 , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 1 - 1
   {"clac",      0x81D , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 1 - 2
   {"stac",      0x81D , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 1 - 3
   {0,           0     , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // illegal

// EVEX 0F 38 1B, indexed by W bit
SOpcodeDef OpcodeMap38[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vbroadcastf32x8",0x20,0xC69200,0x12, 0x164B, 0x254B, 0     , 0     , 0x20  , 0x1011, 0     , 0x100 },    // EVEX W0 0F 38 1B
   {"vbroadcastf64x4",0x20,0xC69200,0x12, 0x164C, 0x254C, 0     , 0     , 0x20  , 0x1011, 0     , 0x100 }};   // EVEX W1 0F 38 1B

// Secondary opcode map for cbw/cwde/cdqe. Opcode byte = 0x98
// Indexed by operand size = 16, 32, 64
SOpcodeDef OpcodeMap39[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cbw",       0     , 0x100  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 98
   {"cwde",      0     , 0x100  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 98
   {"cdqe",      0x4000, 0x1000 , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   }};   // 98

// Secondary opcode map for cwd/cdq/cqo. Opcode byte = 0x99
// Indexed by operand size = 16, 32, 64
SOpcodeDef OpcodeMap3A[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cwd",       0     , 0x100  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 99
   {"cdq",       0     , 0x100  , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 99
   {"cqo",       0x4000, 0x1000 , 0x1   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   }};   // 99


// Secondary opcode map for arpl/movsxd. Opcode byte = 0x63
// Indexed by mode = 16, 32, 64
SOpcodeDef OpcodeMap3B[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"arpl",      0x8802, 0      , 0x13  , 0x2   , 0x1002, 0     , 0     , 0     , 0     , 0     , 0     },    // 63
   {"arpl",      0x8802, 0      , 0x13  , 0x2   , 0x1002, 0     , 0     , 0     , 0     , 0     , 0     },    // 63
   {"movsxd",    0x4000, 0x1000 , 0x12  , 0x1009, 0x3   , 0     , 0     , 0     , 0     , 0     , 0     }};   // 63

// Secondary opcode map for nop/pause. Opcode byte = 0x90
// Indexed by prefix = none, 66, F2, F3
SOpcodeDef OpcodeMap3C[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"nop",       0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x40  },    // 90
   {"nop",       0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x40  },    // 66 90
   {"nop",       0     , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x40  },    // F2 90
   {"pause",     0     , 0x400  , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 90. (No instset indicated because backwards compatible)

// Secondary opcode map for jcxz. Opcode byte = 0xE3
// Indexed by address size
SOpcodeDef OpcodeMap3D[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"jcxz",      0     , 0x81   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // E3
   {"jecxz",     0     , 0x81   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0     },
   {"jrcxz",     0x4000, 0x81   , 0x42  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

// Secondary opcode map for pushf/d/q. Opcode byte = 0x9C
// Indexed by operand size
SOpcodeDef OpcodeMap3E[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pushf",     0     , 0x102  , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 9C pushf
   {"pushf",     0     , 0x2102 , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   },    // 9C pushf/d/q
   {"pushf",     0     , 0x2102 , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   }};   // 9C pushf/d/q

// Secondary opcode map for poof/d/q. Opcode byte = 0x9D
// Indexed by operand size
SOpcodeDef OpcodeMap3F[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"popf",      0     , 0x102  , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 9D popf
   {"popf",      0     , 0x2102 , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   },    // 9D popf/d/q
   {"popf",      0     , 0x2102 , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x1   }};   // 9D popf/d/q

// Tertiary opcode map for movups etc. Opcode byte = 0F 10
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap40[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movups",    0x11  ,0x850000, 0x12  , 0x124B, 0x251 , 0     , 0     , 0x30  , 0     , 0     , 0x202 },    // 0F 10
   {"movupd",    0x12  ,0x852200, 0x12  , 0x124C, 0x251 , 0     , 0     , 0x30  , 0     , 0     , 0x202 },    // 66 0F 10
   {"movsd",     0x71  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 3     , 0     },    // F2 0F 10 Link for memory/register
   {"movss",     0x72  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 3     , 0     }};   // F3 0F 10 Link for memory/register

// Tertiary opcode map for movups etc. Opcode byte = 0F 11
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap41[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movups",    0x11  ,0x850000, 0x13  , 0x251 , 0x124B, 0     , 0     , 0x30  , 0     , 0     , 0x202 },    // 0F 11
   {"movupd",    0x12  ,0x852200, 0x13  , 0x251 , 0x124C, 0     , 0     , 0x30  , 0     , 0     , 0x202 },    // 66 0F 11
   {"movsd",     0x73  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 3     , 0     },    // F2 0F 11 Link for memory/register
   {"movss",     0x74  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 3     , 0     }};   // F3 0F 11 Link for memory/register

// Tertiary opcode map for movlps etc. Opcode byte = 0F 12
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap42[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x43  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // Link to quarternary map
   {"movlpd",    0x12  ,0x892200, 0x19  , 0x144C, 0x144C, 0x204C, 0     , 0     , 0     , 0     , 0x2   },    // 66 0F 12
   {"movddup",   0x70  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xB   , 0     },    // F2 0F 12
   {"movsldup",  0x13  ,0x852400, 0x12  , 0x124B, 0x24B , 0     , 0     , 0x30  , 0     , 0     , 0x2   }};   // F3 0F 12

// Quarternary opcode map for movlps and movhlps. Opcode byte = 0F 12
// Indexed by mod bits
SOpcodeDef OpcodeMap43[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movlps",    0x11  ,0x892000, 0x19  , 0x144B, 0x144B, 0x234B, 0     , 0x1000, 0     , 0     , 0x2   },    // 0F 12 (mem)
   {"movhlps",   0x11  ,0x892000, 0x19  , 0x144B, 0x144B, 0x144B, 0     , 0x00  , 0     , 0     , 0x2   }};   // 0F 12 (reg)

// Tertiary opcode map for movlps etc. Opcode byte = 0F 16
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap44[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x45  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // Link to quarternary map
   {"movhpd",    0x12  ,0x892200, 0x19  , 0x144C, 0x144C, 0x204C, 0     , 0x00  , 0     , 0     , 0x2   },    // 66 0F 16
   {0,           0x13  , 0x800  , 0x4012, 0x124C, 0x4C  , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 16
   {"movshdup",  0x13  ,0x852400, 0x12  , 0x124B, 0x24B , 0     , 0     , 0x30  , 0     , 0     , 0x2   }};   // F3 0F 16

// Quarternary opcode map for movhps and movlhps. Opcode byte = 0F 16
// Indexed by mod bits
SOpcodeDef OpcodeMap45[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movhps",    0x11  ,0x890000, 0x19  , 0x144B, 0x144B, 0x234B, 0     , 0x1000, 0     , 0     , 0x2   },    // 0F 12 (mem)
   {"movlhps",   0x11  ,0x890000, 0x19  , 0x144B, 0x144B, 0x144B, 0     , 0x0   , 0     , 0     , 0x2   }};   // 0F 12 (reg)

// Tertiary opcode map for cvtpi2ps etc. Opcode byte = 0F 2A
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap46[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cvtpi2ps",  0x11  , 0      , 0x12  , 0x124B, 0x303 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 2A
   {"cvtpi2pd",  0x12  , 0x200  , 0x12  , 0x124C, 0x303 , 0     , 0     , 0     , 0     , 0     , 0     },    // 66 0F 2A
   {"cvtsi2sd",  0x12  ,0x891800, 0x19  , 0x104C, 0x104C, 0x9   , 0     , 0x6   , 0     , 0     , 0x2   },    // F2 0F 2A
   {"cvtsi2ss",  0x12  ,0x891400, 0x19  , 0x104B, 0x104B, 0x9   , 0     , 0x6   , 0     , 0     , 0x2   }};   // F3 0F 2A

// Tertiary opcode map for cvttps2pi etc. Opcode byte = 0F 2C
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap47[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cvttps2pi", 0x11  , 0      , 0x12  , 0x1303, 0x24B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 2C
   {"cvttpd2pi", 0x12  , 0x200  , 0x12  , 0x1303, 0x24C , 0     , 0     , 0     , 0     , 0     , 0     },    // 66 0F 2C
   {"cvttsd2si", 0x12  ,0x811800, 0x12  , 0x1009, 0x4C  , 0     , 0     , 0x2   , 0     , 0     , 0x2   },    // F2 0F 2C
   {"cvttss2si", 0x12  ,0x811400, 0x12  , 0x1009, 0x4B  , 0     , 0     , 0x2   , 0     , 0     , 0x2   }};   // F3 0F 2C

// Tertiary opcode map for cvtps2pi etc. Opcode byte = 0F 2D
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap48[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cvtps2pi",  0x11  ,0x000000, 0x12  , 0x1303, 0x24B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 2D
   {"cvtpd2pi",  0x12  ,0x000200, 0x12  , 0x1303, 0x24C , 0     , 0     , 0     , 0     , 0     , 0     },    // 66 0F 2D
   {"cvtsd2si",  0x12  ,0x811800, 0x12  , 0x1009, 0x4C  , 0     , 0     , 0x6   , 0     , 0     , 0x2   },    // F2 0F 2D
   {"cvtss2si",  0x12  ,0x811400, 0x12  , 0x1009, 0x4B  , 0     , 0     , 0x6   , 0     , 0     , 0x2   }};   // F3 0F 2D

// Tertiary opcode map for cvtps2pd etc. Opcode byte = 0F 5A
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap49[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cvtps2pd",  0x12  ,0xC50000, 0x12  , 0x124C, 0xF4B , 0     , 0     , 0x33  , 0x1215, 0     , 0x2   },    //    0F 5A
   {"cvtpd2ps",  0x12  ,0xC52200, 0x12  , 0x1F4B, 0x24C , 0     , 0     , 0x37,   0x1305, 0     , 0x2   },    // 66 0F 5A
   {"cvtsd2ss",  0x12  ,0x892800, 0x19  , 0x104B, 0x4C  , 0x4C  , 0     , 0x36  , 0     , 0     , 0x2   },    // F2 0F 5A
   {"cvtss2sd",  0x12  ,0x892400, 0x19  , 0x104C, 0x104C, 0x4B  , 0     , 0x32  , 0     , 0     , 0x2   }};   // F3 0F 5A

// Tertiary opcode map for cvtdq2ps etc. Opcode byte = 0F 5B
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4A[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cvtdq2ps",  0x12  ,0x850000, 0x12  , 0x124B, 0x203 , 0     , 0     , 0x37  , 0     , 0     , 0x2   },    // 0F 5B
   {"cvtps2dq",  0x12  ,0x850200, 0x12  , 0x1203, 0x24B , 0     , 0     , 0x37  , 0     , 0     , 0x2   },    // 66 0F 5B
   {0,           0x12  ,0x800   , 0x4012, 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 5B. Illegal
   {"cvttps2dq", 0x12  ,0x852400, 0x12  , 0x1203, 0x24B , 0     , 0     , 0x37  , 0     , 0     , 0x2   }};   // F3 0F 5B

// Tertiary opcode map for ucomiss/sd etc. Opcode byte = 0F 2E
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4B[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"ucomiss",   0x11  ,0x810200, 0x12  , 0x124B, 0x4B  , 0     , 0     , 0x2   , 0     , 0     , 0x6   },    // 0F 2E. ucomiss
   {"ucomisd",   0x11  ,0x812200, 0x12  , 0x124C, 0x4C  , 0     , 0     , 0x2   , 0     , 0     , 0x6   },    // 66 0F 2E. ucomisd
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // Illegal

// Tertiary opcode map for comiss/sd etc. Opcode byte = 0F 2F
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4C[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"comiss",    0x11  ,0x812200, 0x12  , 0x124B, 0x4B  , 0     , 0     , 0x2   , 0     , 0     , 0x6   },    // 0F 2F. comiss
   {"comisd",    0x11  ,0x812200, 0x12  , 0x124C, 0x4C  , 0     , 0     , 0x2   , 0     , 0     , 0x6   },    // 66 0F 2F. comisd
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // Illegal

// Tertiary opcode map for movq/movdqa/movdqu. Opcode byte = 0F 6F
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4D[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movq",      0x7   , 0      , 0x12  , 0x1351, 0x351 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 6F
   {"movdqa",    0xB8  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 66 0F 6F. Link to movdqa and vmovdqa32
   {"vmovdqu",   0x19  ,0x864800, 0x12  , 0x1209, 0x209 , 0     , 0     , 0x20  , 0     , 0     , 0x1200},    // F2 0F 6F
   {"movdqu",    0xB9  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     }};   // F3 0F 6F. Link to movdqu and vmovdqu32

// Tertiary opcode map for movq/movdqa/movdqu. Opcode byte = 0F 7F
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4E[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movq",      0x7   , 0      , 0x13  , 0x351 , 0x1351, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7F
   {"movdqa",    0xBA  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // 66 0F 7F. Link to movdqa and vmovdqa32
   {"vmovdqu",   0x19  ,0x864800, 0x13  , 0x209 , 0x1209, 0     , 0     , 0x20  , 0     , 0     ,0x1200 },    // E/MVEX F3 0F 7F
   {"movdqu",    0xBB  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     }};   // F3 0F 7F. Link to movdqu and vmovdqu32

// Tertiary opcode map for pshufw etc. Opcode byte = 0F 70
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap4F[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pshufw",    0x7   , 0      , 0x52  , 0x1302, 0x302 , 0x31  , 0     , 0     , 0     , 0     , 0     },    // 0F 70
   {"pshufd",    0x12  ,0xC52100, 0x52  , 0x1203, 0x203 , 0x31  , 0     , 0x21  , 0x1000, 0     , 0x2   },    // 66 0F 70
   {"pshuflw",   0x12  ,0x852800, 0x52  , 0x1202, 0x202 , 0x31  , 0     , 0x20  , 0     , 0     , 0x2   },    // F2 0F 70
   {"pshufhw",   0x12  ,0x852400, 0x52  , 0x1202, 0x202 , 0x31  , 0     , 0x20  , 0     , 0     , 0x2   }};   // F3 0F 70

// Tertiary opcode map for group 9. Opcode byte = 0F C7
// Indexed by reg bits = 0 - 7. 
SOpcodeDef OpcodeMap50[8] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0. Illegal
   {0,           0x51  , 0x1010 , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // 1. Link to map: cmpxchg8b
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {0,           0xAC  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 6. Link to map: vmptrld etc
   {0,           0xAF  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     }};   // 7. Link to map: vmptrst, rdseed

// Quarternary opcode map for cmpxchg8b. Opcode byte = 0F C7 /1
// Indexed by operand size: 16, 32, 64
SOpcodeDef OpcodeMap51[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cmpxchg8b", 0x5   , 0x1C50 , 0x11  , 0x2351, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"cmpxchg8b", 0x5   , 0x1C50 , 0x11  , 0x2351, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"cmpxchg16b",0x5   , 0x1C50 , 0x11  , 0x2450, 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

// Quarternary opcode map for vmptrld etc. Opcode byte = 0F C7 /6 mem
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap52[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vmptrld",   0x813 , 0      , 0x11  , 0x2351, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F C7 /6 mem
   {"vmclear",   0x813 , 0x200  , 0x11  , 0x2351, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0x813 , 0x800  , 0x11  , 0x2351, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {"vmxon",     0x813 , 0x400  , 0x11  , 0x2351, 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

// Quarternary opcode map for movdq2q etc. Opcode byte = 0F D6
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap53[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {"movq",      0x6F  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     },    // 66: Link to movq m64,xmm / movq xmm,xmm
   {"movdq2q",   0x12  , 0x800  , 0x12  , 0x1351, 0x1450, 0     , 0     , 0     , 0     , 0     , 0     },    // F2
   {"movq2dq",   0x12  , 0x400  , 0x12  , 0x1450, 0x1351, 0     , 0     , 0     , 0     , 0     , 0     }};   // F3

// Quarternary opcode map for cvtpd2dq etc. Opcode byte = 0F E6
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap54[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {"cvttpd2dq", 0x12  ,0x852200, 0x12  , 0x1F03, 0x24C , 0     , 0     , 0x33  , 0     , 0     , 0x2   },    // 66
   {"cvtpd2dq",  0x12  ,0x852800, 0x12  , 0x1F03, 0x24C , 0     , 0     , 0x37  , 0     , 0     , 0x2   },    // F2
   {"cvtdq2pd",  0x12  ,0xC50400, 0x12  , 0x124C, 0xF03 , 0     , 0     , 0x31  , 0x1214, 0     , 0x2   }};   // F3

// Quarternary opcode map for movntq etc. Opcode byte = 0F E7
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap55[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movntq",    0x11  , 0      , 0x13  , 0x2351, 0x1351, 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"movntdq",   0x12  ,0x850200, 0x13  , 0x2250, 0x1250, 0     , 0     , 0x00  , 0     , 0     , 0x102 },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // Illegal

// Quarternary opcode map for lddqu. Opcode byte = 0F F0
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap56[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x12  , 0x1450, 0x1450, 0     , 0     , 0     , 0     , 0     , 0     },    // Illegal
   {0,           0     , 0x200  , 0x12  , 0x1450, 0x1450, 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {"lddqu",     0x13  , 0x58800, 0x12  , 0x1250, 0x251,  0     , 0     , 0     , 0     , 0     , 0x202 },    //
   {0,           0     , 0x400  , 0x12  , 0x1450, 0x1450, 0     , 0     , 0     , 0     , 0     , 0     }};   // Illegal

// Quarternary opcode map for maskmovq. Opcode byte = 0F F7
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap57[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"maskmovq",  0x7   , 0x5    , 0x12  , 0x1351, 0x1351, 0     , 0     , 0     , 0     , 0     , 0x20  },    // 
   {"maskmovdqu",0x12  , 0x18205, 0x12  , 0x1450, 0x1450, 0     , 0     , 0     , 0     , 0     , 0x22  },    //
   {0,           0     , 0      , 0x12  , 0x1450, 0x2450, 0     , 0     , 0     , 0     , 0     , 0     }};   // Illegal

// Tertiary opcode map for movd/movq. Opcode byte = 0F 6E
// Indexed by operand size 16/32/64
// First two lines are identical because operand size is determined only by REX.W prefix,
// while 66 prefix determines mmx or xmm register
// Note: VEX/EVEX version is in map B1
SOpcodeDef OpcodeMap58[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movd",      0x7   , 0x11200, 0x12  , 0x1103, 0x3   , 0     , 0     , 0x00  , 0     , 0     , 0x2   },    // 0F 6E
   {"movd",      0x7   , 0x11200, 0x12  , 0x1103, 0x3   , 0     , 0     , 0x00  , 0     , 0     , 0x2   },    // 0F 6E
   {"movq",      0x4000, 0x11200, 0x12  , 0x1104, 0x4   , 0     , 0     , 0x00  , 0     , 0     , 0x2   }};   // 0F 6E. Name varies: movd or movq, though the operand is 64 bits

// Tertiary opcode map for movd/movq. Opcode byte = 0F 7E
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMap59[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x5A  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // 0F 7E. Link to map 5A. Name depends on REX.W prefix
   {0,           0x5A  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   , 0     },    // 66 0F 7E. Link to map 5A. Name depends on REX.W prefix
   {0,           0x7   , 0      , 0x4013, 0x3   , 0x1103, 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 7E. Doesn't exist
   {0,           0x5B  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     }};   // F3 0F 7E. Link to map 5B. movq xmm,xmm/m64

// Quarternary opcode map for movd/movq. Opcode byte = 66 0F 7E
// Indexed by operand size 16/32/64
// First two lines are identical because operand size is determined only by REX.W prefix,
// while 66 prefix determines mmx or xmm register
SOpcodeDef OpcodeMap5A[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movd",      0x7   , 0x11200, 0x13  , 0x3   , 0x1103, 0     , 0     , 0     , 0     , 0     , 0x2   },    // 0F 7E
   {"movd",      0x7   , 0x11200, 0x13  , 0x3   , 0x1103, 0     , 0     , 0     , 0     , 0     , 0x2   },    // 0F 7E
   {"movq",      0x4000, 0x11200, 0x13  , 0x4   , 0x1104, 0x0   , 0     , 0     , 0     , 0     , 0x2   }};   // 0F 7E. Name varies: movd or movq, though the operand is 64 bits

// Quarternary opcode map for movq xmm,xmm/m64. Opcode byte = F3 0F 7E
// Indexed by memory vs. register operand
// Link to here from both map 59 (without VEX) and map E2 (with VEX)
SOpcodeDef OpcodeMap5B[2] = {
   {"movq",      0x12  ,0x812400, 0x12  , 0x1404, 0x4   , 0     , 0     , 0     , 0     , 0     , 0x2   },    // F3 0F 7E. movq xmm,m64
   {"movq",      0x12  ,0x812400, 0x12  , 0x1404, 0x404 , 0     , 0     , 0     , 0     , 0     , 0x2   }};   // F3 0F 7E. movq xmm,xmm

// Tertiary opcode map for haddps/pd etc. Opcode byte = 0F 7C
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap5C[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x4012, 0x124F, 0x24F , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7C
   {"haddpd",    0x13  , 0xD0A00, 0x19  , 0x124C, 0x124C, 0x24C , 0     , 0     , 0     , 0     , 0x2   },    // 66 0F 7C
   {"haddps",    0x13  , 0xD0A00, 0x19  , 0x124B, 0x124B, 0x24B , 0     , 0     , 0     , 0     , 0x2   },    // F2 0F 7C
   {0,           0     , 0      , 0x4012, 0x124F, 0x24F , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F 7C

// Tertiary opcode map for hsubps/pd etc. Opcode byte = 0F 7D
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap5D[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x4012, 0x124F, 0x24F , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7D
   {"hsubpd",    0x13  , 0xD0A00, 0x19  , 0x124C, 0x124C, 0x24C , 0     , 0     , 0     , 0     , 0x2   },    // 66 0F 7D
   {"hsubps",    0x13  , 0xD0A00, 0x19  , 0x124B, 0x124B, 0x24B , 0     , 0     , 0     , 0     , 0x2   },    // F2 0F 7D
   {0,           0     , 0      , 0x4012, 0x124F, 0x24F , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F 7D

// Tertiary opcode map for lar. Opcode byte = 0F 02
// Indexed by memory vs. register operand
SOpcodeDef OpcodeMap5E[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"lar",       0x802 , 0x1100 , 0x12  , 0x1009, 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 02 memory
   {"lar",       0x802 , 0x1100 , 0x12  , 0x1009, 0x1009, 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F 02 register

// Tertiary opcode map for lsl. Opcode byte = 0F 03
// Indexed by memory vs. register operand
SOpcodeDef OpcodeMap5F[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"lsl",       0x802 , 0x1100 , 0x12  , 0x1009, 0x2002, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 03 memory
   {"lsl",       0x802 , 0x1100 , 0x12  , 0x1009, 0x1009, 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F 03 register

// Tertiary opcode map for popcnt. Opcode byte = 0F B8
// Indexed by prefixes (none, 66, F2, F3)
SOpcodeDef OpcodeMap60[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"jmpe;Itanium only",0,0     , 0x11  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },    // 0F B8
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 66 0F B8
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F B8
   {"popcnt",    0x16  ,0x11500 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F B8

// Quarternary opcode map for pextrb. Opcode byte = 0F 3A 14
// Indexed by memory vs. register operand
SOpcodeDef OpcodeMap61[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pextrb",    0x15  ,0x81A200, 0x53  , 0x2001, 0x1401, 0x31  , 0     , 0x1000, 0     , 0     , 0x2   },    // 0F 3A 14 memory
   {"pextrb",    0x15  ,0x81A200, 0x53  , 0x1009, 0x1401, 0x31  , 0     , 0     , 0     , 0     , 0x2   }};   // 0F 3A 14 register

// Quarternary opcode map for pextrw. Opcode byte = 0F 3A 15
// Indexed by memory vs. register operand
SOpcodeDef OpcodeMap62[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pextrw",    0x15  ,0x81A200, 0x53  , 0x2002, 0x1402, 0x31  , 0     , 0x1000, 0     , 0     , 0x2   },    // 0F 3A 15 memory
   {"pextrw",    0x15  ,0x81A200, 0x53  , 0x1002, 0x1402, 0x31  , 0     , 0     , 0     , 0     , 0x2   }};   // 0F 3A 15 register

// Quarternary opcode map for pextrd/q. Opcode byte = 0F 3A 16
// Indexed by operand size (16, 32, 64)
SOpcodeDef OpcodeMap63[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pextr",     0x15  ,0x81B200, 0x53  , 0x3   , 0x1403, 0x31  , 0     , 0x1000, 0     , 0     , 3     },    // 0F 3A 16 pextrd
   {"pextr",     0x15  ,0x81B200, 0x53  , 0x3   , 0x1403, 0x31  , 0     , 0x1000, 0     , 0     , 3     },    // 0F 3A 16 pextrd
   {"pextr",     0x15  ,0x81B200, 0x53  , 0x4   , 0x1404, 0x31  , 0     , 0x1000, 0     , 0     , 3     }};   // 0F 3A 16 pextrq

// Opcode map for AMD instructions with XOP prefix and mmmmm = 01000
// Indexed by first opcode byte after XOP prefix. Has one byte immediate data
SOpcodeDef OpcodeMap64[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 00
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 01
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 02
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 03
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 04
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 05
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 06
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 07
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 08
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 09
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 0A
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 0B
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 0C
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 0D
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 0E
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 0F

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 10
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 11
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 12
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 13
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 14
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 15
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 16
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 17
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 18
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 19
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 1A
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 1B
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 1C
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 1D
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 1E
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 1F

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 20
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 21
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 22
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 23
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 24
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 25
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 26
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 27
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 28
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 29
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 2A
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 2B
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 2C
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 2D
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 2E
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 2F

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 30
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 31
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 32
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 33
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 34
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 35
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 36
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 37
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 38
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 39
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 3A
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 3B
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 3C
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 3D
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 3E
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 3F

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 40
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 41
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 42
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 43
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 44
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 45
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 46
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 47
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 48
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 49
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 4A
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 4B
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 4C
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 4D
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 4E
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 4F

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 50
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 51
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 52
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 53
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 54
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 55
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 56
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 57
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 58
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 59
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 5A
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 5B
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 5C
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 5D
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 5E
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 5F

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 60
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 61
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 62
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 63
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 64
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 65
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 66
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 67
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 68
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 69
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 6A
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 6B
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 6C
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 6D
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 6E
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 6F

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 70
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 71
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 72
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 73
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 74
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 75
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 76
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 77
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 78
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 79
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 7A
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 7B
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 7C
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 7D
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 7E
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 7F

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 80
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 81
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 82
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 83
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 84
   {"vpmacssww", 0x1005, 0xB0000, 0x5C,   0x1202, 0x1202, 0x202 , 0x1202, 0     , 0     , 0     , 0     },    // XOP(8) 85
   {"vpmacsswd", 0x1005, 0xB0000, 0x5C,   0x1203, 0x1202, 0x202 , 0x1203, 0     , 0     , 0     , 0     },    // XOP(8) 86
   {"vpmacssdql",0x1005, 0xB0000, 0x5C,   0x1204, 0x1203, 0x203 , 0x1204, 0     , 0     , 0     , 0     },    // XOP(8) 87
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 88
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 89
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 8A
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 8B
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 8C
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 8D
   {"vpmacssdd", 0x1005, 0xB0000, 0x5C,   0x1203, 0x1203, 0x203 , 0x1203, 0     , 0     , 0     , 0     },    // XOP(8) 8E
   {"vpmacssdqh",0x1005, 0xB0000, 0x5C,   0x1204, 0x1203, 0x203 , 0x1204, 0     , 0     , 0     , 0     },    // XOP(8) 8F

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 90
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 91
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 92
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 93
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 94
   {"vpmacsww",  0x1005, 0xB0000, 0x5C,   0x1202, 0x1202, 0x202 , 0x1202, 0     , 0     , 0     , 0     },    // XOP(8) 95
   {"vpmacswd",  0x1005, 0xB0000, 0x5C,   0x1203, 0x1202, 0x202 , 0x1203, 0     , 0     , 0     , 0     },    // XOP(8) 96
   {"vpmacsdql", 0x1005, 0xB0000, 0x5C,   0x1204, 0x1203, 0x203 , 0x1204, 0     , 0     , 0     , 0     },    // XOP(8) 97
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 98
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 99
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 9A
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 9B
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 9C
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) 9D
   {"vpmacsdd",  0x1005, 0xB0000, 0x5C,   0x1203, 0x1203, 0x203 , 0x1203, 0     , 0     , 0     , 0     },    // XOP(8) 9E
   {"vpmacsdqh", 0x1005, 0xB0000, 0x5C,   0x1204, 0x1203, 0x203 , 0x1204, 0     , 0     , 0     , 0     },    // XOP(8) 9F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cvtph2ps",  0x1D  , 0x70000, 0x52  , 0x124B, 0x402 , 0x31  , 0     , 0     , 0     , 0     , 0     },    // XOP(8) A0
   {"cvtps2ph",  0x1D  , 0x70000, 0x53  , 0x402,  0x124B, 0x31  , 0     , 0     , 0     , 0     , 0     },    // XOP(8) A1
   {"vpcmov",    0x1005, 0xF7000, 0x5C,   0x1201, 0x1201, 0x201 , 0x201 , 0     , 0     , 0     , 0     },    // XOP(8) A2
   {"vpperm",    0x1005, 0xB7000, 0x5C,   0x1201, 0x1201, 0x201 , 0x201 , 0     , 0     , 0     , 0     },    // XOP(8) A3
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) A4
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) A5
   {"vpmadcsswd",0x1005, 0xB0000, 0x5C,   0x1203, 0x1202, 0x202 , 0x1203, 0     , 0     , 0     , 0     },    // XOP(8) A6
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) A7
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) A8
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) A9
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) AA
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) AB
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) AC
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) AD
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) AE
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) AF

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) B0
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) B1
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) B2
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) B3
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) B4
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) B5
   {"vpmadcswd", 0x1005, 0xB0000, 0x5C,   0x1203, 0x1202, 0x202 , 0x1203, 0     , 0     , 0     , 0     },    // XOP(8) B6
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) B7
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) B8
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) B9
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) BA
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) BB
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) BC
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) BD
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) BE
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) BF

   {"vprotb",    0x1005, 0x30000, 0x52  , 0x1401, 0x401 , 0x21  , 0     , 0     , 0     , 0     , 0     },    // XOP(8) C0
   {"vprotw",    0x1005, 0x30000, 0x52  , 0x1402, 0x402 , 0x21  , 0     , 0     , 0     , 0     , 0     },    // XOP(8) C1
   {"vprotd",    0x1005, 0x30000, 0x52  , 0x1403, 0x403 , 0x21  , 0     , 0     , 0     , 0     , 0     },    // XOP(8) C2
   {"vprotq",    0x1005, 0x30000, 0x52  , 0x1404, 0x404 , 0x21  , 0     , 0     , 0     , 0     , 0     },    // XOP(8) C3
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) C4
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) C5
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) C6
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) C7
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) C8
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) C9
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) CA
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) CB
   {"vpcomb",    0x1005, 0xB0000, 0x59  , 0x1401, 0x1401, 0x401 , 0x31  , 0     , 0     , 0     , 0     },    // XOP(8) CC
   {"vpcomw",    0x1005, 0xB0000, 0x59  , 0x1402, 0x1402, 0x402 , 0x31  , 0     , 0     , 0     , 0     },    // XOP(8) CD
   {"vpcomd",    0x1005, 0xB0000, 0x59  , 0x1403, 0x1403, 0x403 , 0x31  , 0     , 0     , 0     , 0     },    // XOP(8) CE
   {"vpcomq",    0x1005, 0xB0000, 0x59  , 0x1404, 0x1404, 0x404 , 0x31  , 0     , 0     , 0     , 0     },    // XOP(8) CF

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) D0
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) D1
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) D2
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) D3
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) D4
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) D5
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) D6
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) D7
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) D8
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) D9
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) DA
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) DB
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) DC
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) DD
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) DE
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) DF

   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) E0
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) E1
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) E2
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) E3
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) E4
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) E5
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) E6
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) E7
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) E8
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) E9
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) EA
   {0,           0     , 0      , 0x2059, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(8) EB
   {"vpcomub",   0x1005, 0xB0000, 0x59  , 0x1401, 0x1401, 0x401 , 0x31  , 0     , 0     , 0     , 0     },    // XOP(8) EC
   {"vpcomuw",   0x1005, 0xB0000, 0x59  , 0x1402, 0x1402, 0x402 , 0x31  , 0     , 0     , 0     , 0     },    // XOP(8) ED
   {"vpcomud",   0x1005, 0xB0000, 0x59  , 0x1403, 0x1403, 0x403 , 0x31  , 0     , 0     , 0     , 0     },    // XOP(8) EE
   {"vpcomuq",   0x1005, 0xB0000, 0x59  , 0x1404, 0x1404, 0x404 , 0x31  , 0     , 0     , 0     , 0     }};   // XOP(8) EF


// Opcode map for AMD instructions with XOP prefix and mmmmm = 01001
// Indexed by first opcode byte after XOP prefix. Has no immediate data
SOpcodeDef OpcodeMap65[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 00
   {0,           0xD4  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // XOP(9) 01. Link blcfill etc.
   {0,           0xD5  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // XOP(9) 02. Link blcmsk  etc.
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 03
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 04
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 05
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 06
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 07
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 08
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 09
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 0A
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 0B
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 0C
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 0D
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 0E
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 0F

   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 10
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 11
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 12
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 13
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 14
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 15
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 16
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 17
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 18
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 19
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 1A
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 1B
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 1C
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 1D
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 1E
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 1F

   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 20
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 21
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 22
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 23
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 24
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 25
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 26
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 27
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 28
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 29
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 2A
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 2B
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 2C
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 2D
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 2E
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 2F

   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 30
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 31
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 32
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 33
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 34
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 35
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 36
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 37
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 38
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 39
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 3A
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 3B
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 3C
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 3D
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 3E
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 3F

   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 40
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 41
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 42
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 43
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 44
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 45
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 46
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 47
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 48
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 49
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 4A
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 4B
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 4C
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 4D
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 4E
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 4F

   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 50
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 51
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 52
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 53
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 54
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 55
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 56
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 57
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 58
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 59
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 5A
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 5B
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 5C
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 5D
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 5E
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 5F

   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 60
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 61
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 62
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 63
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 64
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 65
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 66
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 67
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 68
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 69
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 6A
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 6B
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 6C
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 6D
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 6E
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 6F

   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 70
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 71
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 72
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 73
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 74
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 75
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 76
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 77
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 78
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 79
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 7A
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 7B
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 7C
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 7D
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 7E
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 7F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"frczps",    0x11005,0x70000, 0x12  , 0x124B, 0x24B , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 80
   {"frczpd",    0x11005,0x70000, 0x12  , 0x124C, 0x24C , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 81
   {"frczss",    0x11005,0x70000, 0x12  , 0x124B, 0x4B  , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 82
   {"frczsd",    0x11005,0x70000, 0x12  , 0x124C, 0x4C  , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 83
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 84
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 85
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 86
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 87
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 88
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 89
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 8A
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 8B
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 8C
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 8D
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 8E
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 8F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vprotb",    0x1005, 0xB7000, 0x19  , 0x1401, 0x401 , 0x401 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 90
   {"vprotw",    0x1005, 0xB7000, 0x19  , 0x1402, 0x402 , 0x402 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 91
   {"vprotd",    0x1005, 0xB7000, 0x19  , 0x1403, 0x403 , 0x403 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 92
   {"vprotq",    0x1005, 0xB7000, 0x19  , 0x1404, 0x404 , 0x404 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 93
   {"vpshlb",    0x1005, 0xB7000, 0x19  , 0x1401, 0x401 , 0x401 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 94
   {"vpshlw",    0x1005, 0xB7000, 0x19  , 0x1402, 0x402 , 0x402 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 95
   {"vpshld",    0x1005, 0xB7000, 0x19  , 0x1403, 0x403 , 0x403 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 96
   {"vpshlq",    0x1005, 0xB7000, 0x19  , 0x1404, 0x404 , 0x404 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 97
   {"vpshab",    0x1005, 0xB7000, 0x19  , 0x1401, 0x401 , 0x401 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 98
   {"vpshaw",    0x1005, 0xB7000, 0x19  , 0x1402, 0x402 , 0x402 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 99
   {"vpshad",    0x1005, 0xB7000, 0x19  , 0x1403, 0x403 , 0x403 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 9A
   {"vpshaq",    0x1005, 0xB7000, 0x19  , 0x1404, 0x404 , 0x404 , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 9B
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 9C
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 9D
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 9E
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 9F

   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) A0
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) A1
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) A2
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) A3
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) A4
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) A5
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) A6
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) A7
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) A8
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) A9
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) AA
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) AB
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) AC
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) AD
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) AE
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) AF

   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) B0
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) B1
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) B2
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) B3
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) B4
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) B5
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) B6
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) B7
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) B8
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) B9
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) BA
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) BB
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) BC
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) BD
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) BE
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) BF

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) C0
   {"vphaddbw",  0x1005, 0x30000, 0x12  , 0x1402, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) C1
   {"vphaddbd",  0x1005, 0x30000, 0x12  , 0x1403, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) C2
   {"vphaddbq",  0x1005, 0x30000, 0x12  , 0x1404, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) C3
   {0,           0     , 0      , 0x2019, 0x0,    0   ,   0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) C4
   {0,           0     , 0      , 0x2019, 0x0,    0   ,   0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) C5
   {"vphaddwd",  0x1005, 0x30000, 0x12  , 0x1403, 0x402 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) C6
   {"vphaddwq",  0x1005, 0x30000, 0x12  , 0x1404, 0x402 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) C7
   {0,           0     , 0      , 0x2019, 0x0,    0   ,   0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) C8
   {0,           0     , 0      , 0x2019, 0x0,    0   ,   0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) C9
   {0,           0     , 0      , 0x2019, 0x0,    0   ,   0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) CA
   {"vphadddq",  0x1005, 0x30000, 0x12  , 0x1404, 0x403 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) CB
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) CC
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) CD
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) CE
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) CF

   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) D0
   {"vphaddubw", 0x1005, 0x30000, 0x12  , 0x1402, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) D1
   {"vphaddubd", 0x1005, 0x30000, 0x12  , 0x1403, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) D2
   {"vphaddubq", 0x1005, 0x30000, 0x12  , 0x1404, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) D3
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) D3
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) D4
   {"vphadduwd", 0x1005, 0x30000, 0x12  , 0x1403, 0x402 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) D6
   {"vphadduwq", 0x1005, 0x30000, 0x12  , 0x1404, 0x402 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) D7
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) D7
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) D8
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) D9
   {"vphaddudq", 0x1005, 0x30000, 0x12  , 0x1404, 0x403 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) DB
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) DC
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) DD
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) DE
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) DF

   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) E0
   {"vphsubbw",  0x1005, 0x30000, 0x12  , 0x1402, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) E1
   {"vphsubwd",  0x1005, 0x30000, 0x12  , 0x1403, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) E2
   {"vphsubdq",  0x1005, 0x30000, 0x12  , 0x1404, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) E3
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) E4
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) E5
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) E6
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) E7
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) E8
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) E9
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) EA
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) EB
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) EC
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) ED
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) EE
   {0,           0     , 0      , 0x2019, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // XOP(9) EF

// Opcode map for AMD instructions with XOP prefix and mmmmm = 01010
// Indexed by first opcode byte after XOP prefix. Has 4 bytes immediate data
SOpcodeDef OpcodeMap66[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 00
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 01
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 02
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 03
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 04
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 05
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 06
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 07
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 08
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 09
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 0A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 0B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 0C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 0D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 0E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 0F

   {"bextr",     0x1007, 0x11000, 0x92  , 0x1009, 0x9   , 0x33  , 0     , 0     , 0     , 0     , 0     },    // XOP(0xA) 10
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // XOP(0xA) 11


// Opcode map for AMD instructions with XOP prefix and mmmmm = 01011 or whatever (vacant)
// Indexed by first opcode byte after XOP prefix.
SOpcodeDef OpcodeMap67[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // XOP(0xB) 00


// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 24
// Indexed by third opcode byte
// AMD SSE5 instructions with three or four operands

//************************* NOTE ***********************
//  These proposed codes have never been implemented. 
//  Specifications have been changed for the sake of compatibility with Intel AVX coding scheme
// *****************************************************
SOpcodeDef OpcodeMap68[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"fmaddps",   0x21006,0x0    , 0x15  , 0x124B, 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 24 00
   {"fmaddpd",   0x21006,0x0    , 0x15  , 0x124C, 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 24 01
   {"fmaddss",   0x21006,0x0    , 0x15  , 0x104B, 0x4B  , 0x4B  , 0x4B  , 0     , 0     , 0     , 0     },    // 0F 24 02
   {"fmaddsd",   0x21006,0x0    , 0x15  , 0x104C, 0x4C  , 0x4C  , 0x4C  , 0     , 0     , 0     , 0     },    // 0F 24 03
   {"fmaddps",   0x21006,0x0    , 0x15  , 0x124B, 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 24 04
   {"fmaddpd",   0x21006,0x0    , 0x15  , 0x124C, 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 24 05
   {"fmaddss",   0x21006,0x0    , 0x15  , 0x104B, 0x4B  , 0x4B  , 0x4B  , 0     , 0     , 0     , 0     },    // 0F 24 06
   {"fmaddsd",   0x21006,0x0    , 0x15  , 0x104C, 0x4C  , 0x4C  , 0x4C  , 0     , 0     , 0     , 0     },    // 0F 24 07
   {"fmsubps",   0x21006,0x0    , 0x15  , 0x124B, 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 24 08
   {"fmsubpd",   0x21006,0x0    , 0x15  , 0x124C, 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 24 09
   {"fmsubss",   0x21006,0x0    , 0x15  , 0x104B, 0x4B  , 0x4B  , 0x4B  , 0     , 0     , 0     , 0     },    // 0F 24 0A
   {"fmsubsd",   0x21006,0x0    , 0x15  , 0x104C, 0x4C  , 0x4C  , 0x4C  , 0     , 0     , 0     , 0     },    // 0F 24 0B
   {"fmsubps",   0x21006,0x0    , 0x15  , 0x124B, 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 24 0C
   {"fmsubpd",   0x21006,0x0    , 0x15  , 0x124C, 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 24 0D
   {"fmsubss",   0x21006,0x0    , 0x15  , 0x104B, 0x4B  , 0x4B  , 0x4B  , 0     , 0     , 0     , 0     },    // 0F 24 0E
   {"fmsubsd",   0x21006,0x0    , 0x15  , 0x104C, 0x4C  , 0x4C  , 0x4C  , 0     , 0     , 0     , 0     },    // 0F 24 0F

   {"fnmaddps",  0x21006,0x0    , 0x15  , 0x124B, 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 24 10
   {"fnmaddpd",  0x21006,0x0    , 0x15  , 0x124C, 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 24 11
   {"fnmaddss",  0x21006,0x0    , 0x15  , 0x104B, 0x4B  , 0x4B  , 0x4B  , 0     , 0     , 0     , 0     },    // 0F 24 12
   {"fnmaddsd",  0x21006,0x0    , 0x15  , 0x104C, 0x4C  , 0x4C  , 0x4C  , 0     , 0     , 0     , 0     },    // 0F 24 13
   {"fnmaddps",  0x21006,0x0    , 0x15  , 0x124B, 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 24 14
   {"fnmaddpd",  0x21006,0x0    , 0x15  , 0x124C, 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 24 15
   {"fnmaddss",  0x21006,0x0    , 0x15  , 0x104B, 0x4B  , 0x4B  , 0x4B  , 0     , 0     , 0     , 0     },    // 0F 24 16
   {"fnmaddsd",  0x21006,0x0    , 0x15  , 0x104C, 0x4C  , 0x4C  , 0x4C  , 0     , 0     , 0     , 0     },    // 0F 24 17
   {"fnmsubps",  0x21006,0x0    , 0x15  , 0x124B, 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 24 18
   {"fnmsubpd",  0x21006,0x0    , 0x15  , 0x124C, 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 24 19
   {"fnmsubss",  0x21006,0x0    , 0x15  , 0x104B, 0x4B  , 0x4B  , 0x4B  , 0     , 0     , 0     , 0     },    // 0F 24 1A
   {"fnmsubsd",  0x21006,0x0    , 0x15  , 0x104C, 0x4C  , 0x4C  , 0x4C  , 0     , 0     , 0     , 0     },    // 0F 24 1B
   {"fnmsubps",  0x21006,0x0    , 0x15  , 0x124B, 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 24 1C
   {"fnmsubpd",  0x21006,0x0    , 0x15  , 0x124C, 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 24 1D
   {"fnmsubss",  0x21006,0x0    , 0x15  , 0x104B, 0x4B  , 0x4B  , 0x4B  , 0     , 0     , 0     , 0     },    // 0F 24 1E
   {"fnmsubsd",  0x21006,0x0    , 0x15  , 0x104C, 0x4C  , 0x4C  , 0x4C  , 0     , 0     , 0     , 0     },    // 0F 24 1F

   {"permps",    0x21005,0x0    , 0x15  , 0x124B, 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 24 20
   {"permpd",    0x21005,0x0    , 0x15  , 0x124C, 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 24 21
   {"pcmov",     0x21005,0x0    , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 22
   {"pperm",     0x21005,0x0    , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 23
   {"permps",    0x21005,0x0    , 0x15  , 0x124B, 0x24B , 0x24B , 0x24B , 0     , 0     , 0     , 0     },    // 0F 24 24
   {"permpd",    0x21005,0x0    , 0x15  , 0x124C, 0x24C , 0x24C , 0x24C , 0     , 0     , 0     , 0     },    // 0F 24 25
   {"pcmov",     0x21005,0x0    , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 26
   {"pperm",     0x21005,0x0    , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 27

   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 28
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 29
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 2A
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 2B
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 2C
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 2D
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 2E
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 2F

   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 30
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 31
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 32
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 33
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 34
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 35
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 36
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 37
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 38
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 39
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 3A
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 3B
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 3C
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 3D
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 3E
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 3F

   {"protb",     0x21005,0x0    , 0x14  , 0x1401, 0x401 , 0x401 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 40
   {"protw",     0x21005,0x0    , 0x14  , 0x1402, 0x402 , 0x402 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 41
   {"protd",     0x21005,0x0    , 0x14  , 0x1403, 0x403 , 0x403 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 42
   {"protq",     0x21005,0x0    , 0x14  , 0x1404, 0x404 , 0x404 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 43
   {"pshlb",     0x21005,0x0    , 0x14  , 0x1401, 0x401 , 0x401 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 44
   {"pshlw",     0x21005,0x0    , 0x14  , 0x1402, 0x402 , 0x402 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 45
   {"pshld",     0x21005,0x0    , 0x14  , 0x1403, 0x403 , 0x403 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 46
   {"pshlq",     0x21005,0x0    , 0x14  , 0x1404, 0x404 , 0x404 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 47
   {"pshab",     0x21005,0x0    , 0x14  , 0x1401, 0x401 , 0x401 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 48
   {"pshaw",     0x21005,0x0    , 0x14  , 0x1402, 0x402 , 0x402 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 49
   {"pshad",     0x21005,0x0    , 0x14  , 0x1403, 0x403 , 0x403 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 4A
   {"pshaq",     0x21005,0x0    , 0x14  , 0x1404, 0x404 , 0x404 , 0     , 0     , 0     , 0     , 0     },    // 0F 24 4B
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 4C
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 4D
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 4E
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 4F

   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 50
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 51
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 52
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 53
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 54
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 55
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 56
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 57
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 58
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 59
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 5A
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 5B
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 5C
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 5D
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 5E
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 5F

   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 60
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 61
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 62
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 63
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 64
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 65
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 66
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 67
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 68
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 69
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 6A
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 6B
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 6C
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 6D
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 6E
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 6F

   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 70
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 71
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 72
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 73
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 74
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 75
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 76
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 77
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 78
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 79
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 7A
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 7B
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 7C
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 7D
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 7E
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 7F

   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 80
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 81
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 82
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 83
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 84
   {"pmacssww",  0x21005,0x0    , 0x15  , 0x1402, 0x402 , 0x402 , 0x402 , 0     , 0     , 0     , 0     },    // 0F 24 85
   {"pmacsswd",  0x21005,0x0    , 0x15  , 0x1403, 0x402 , 0x402 , 0x402 , 0     , 0     , 0     , 0     },    // 0F 24 86
   {"pmacssdql", 0x21005,0x0    , 0x15  , 0x1404, 0x403 , 0x403 , 0x403 , 0     , 0     , 0     , 0     },    // 0F 24 87
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 88
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 89
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 8A
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 8B
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 8C
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 8D
   {"pmacssdd",  0x21005,0x0    , 0x15  , 0x1403, 0x403 , 0x403 , 0x403 , 0     , 0     , 0     , 0     },    // 0F 24 8E
   {"pmacssdqh", 0x21005,0x0    , 0x15  , 0x1404, 0x403 , 0x403 , 0x403 , 0     , 0     , 0     , 0     },    // 0F 24 8F

   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 90
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 91
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 92
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 93
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 94
   {"pmacsww",   0x21005,0x0    , 0x15  , 0x1402, 0x402 , 0x402 , 0x402 , 0     , 0     , 0     , 0     },    // 0F 24 95
   {"pmacswd",   0x21005,0x0    , 0x15  , 0x1403, 0x402 , 0x402 , 0x402 , 0     , 0     , 0     , 0     },    // 0F 24 96
   {"pmacsdql",  0x21005,0x0    , 0x15  , 0x1404, 0x403 , 0x403 , 0x403 , 0     , 0     , 0     , 0     },    // 0F 24 97
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 98
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 99
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 9A
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 9B
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 9C
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 9D
   {"pmacsdd",   0x21005,0x0    , 0x15  , 0x1403, 0x403 , 0x403 , 0x403 , 0     , 0     , 0     , 0     },    // 0F 24 9E
   {"pmacsdqh",  0x21005,0x0    , 0x15  , 0x1404, 0x403 , 0x403 , 0x403 , 0     , 0     , 0     , 0     },    // 0F 24 9F

   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 A0
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 A1
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 A2
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 A3
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 A4
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 A5
   {"pmadcsswd", 0x21005,0x0    , 0x15  , 0x1403, 0x402 , 0x402 , 0x402 , 0     , 0     , 0     , 0     },    // 0F 24 A6
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 A7
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 A8
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 A9
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 AA
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 AB
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 AC
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 AD
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 AE
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 AF

   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 B0
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 B1
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 B2
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 B3
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 B4
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 B5
   {"pmadcswd",  0x21005,0x0    , 0x15  , 0x1403, 0x402 , 0x402 , 0x402 , 0     , 0     , 0     , 0     },    // 0F 24 B6
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 B7
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 B8
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 B9
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 BA
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 BB
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 BC
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 BD
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 BE
   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     },    // 0F 24 BF

   {0,           0     , 0      , 0x15  , 0x1450, 0x450 , 0x450 , 0x450 , 0     , 0     , 0     , 0     }};   // 0F 24 C0+. Reserved for future opcodes

// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 25
// Indexed by third opcode byte
// AMD SSE5 instructions with three operands + immediate byte
// Note: These proposed codes have never been implemented. 
SOpcodeDef OpcodeMap69[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 00
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 01
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 02
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 03
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 04
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 05
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 06
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 07
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 08
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 09
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 0A
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 0B
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 0C
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 0D
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 0E
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 0F

   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 10
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 11
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 12
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 13
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 14
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 15
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 16
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 17
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 18
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 19
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 1A
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 1B
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 1C
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 1D
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 1E
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 1F

   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 20
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 21
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 22
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 23
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 24
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 25
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 26
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 27
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 28
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 29
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 2A
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 2B
   {"comps",     0x21005,0x0    , 0x54  , 0x124B, 0x24B , 0x24B , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 2C
   {"compd",     0x21005,0x0    , 0x54  , 0x124C, 0x24C , 0x24C , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 2D
   {"comss",     0x21005,0x0    , 0x54  , 0x104B, 0x4B  , 0x4B  , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 2E
   {"comsd",     0x21005,0x0    , 0x54  , 0x104C, 0x4C  , 0x4C  , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 2F

   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 30
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 31
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 32
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 33
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 34
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 35
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 36
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 37
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 38
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 39
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 3A
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 3B
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 3C
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 3D
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 3E
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 3F

   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 40
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 41
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 42
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 43
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 44
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 45
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 46
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 47
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 48
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 49
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 4A
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 4B
   {"pcomb",     0x21005,0x0    , 0x54  , 0x1401, 0x401 , 0x401 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 4C
   {"pcomw",     0x21005,0x0    , 0x54  , 0x1402, 0x402 , 0x402 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 4D
   {"pcomd",     0x21005,0x0    , 0x54  , 0x1403, 0x403 , 0x403 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 4E
   {"pcomq",     0x21005,0x0    , 0x54  , 0x1404, 0x404 , 0x404 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 4F

   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 50
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 51
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 52
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 53
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 54
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 55
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 56
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 57
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 58
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 59
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 5A
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 5B
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 5C
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 5D
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 5E
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 5F

   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 60
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 61
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 62
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 63
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 64
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 65
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 66
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 67
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 68
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 69
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 6A
   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 6B
   {"pcomub",    0x21005,0x0    , 0x54  , 0x1401, 0x401 , 0x401 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 6C
   {"pcomuw",    0x21005,0x0    , 0x54  , 0x1402, 0x402 , 0x402 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 6D
   {"pcomud",    0x21005,0x0    , 0x54  , 0x1403, 0x403 , 0x403 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 6E
   {"pcomuq",    0x21005,0x0    , 0x54  , 0x1404, 0x404 , 0x404 , 0x31  , 0     , 0     , 0     , 0     },    // 0F 25 6F

   {0,           0     , 0      , 0x54  , 0x1450, 0x450 , 0x450 , 0x31  , 0     , 0     , 0     , 0     }};   // 0F 25 70+. Reserved for future opcodes

// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 7A
// Indexed by third opcode byte
// AMD SSE5 instructions with two operands
// Note: These proposed codes have never been implemented. 
SOpcodeDef OpcodeMap6A[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 00
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 01
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 02
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 03
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 04
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 05
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 06
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 07
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 08
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 09
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 0A
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 0B
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 0C
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 0D
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 0E
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 0F

   {"frczps",    0x21005,0x0    , 0x12  , 0x124B, 0x24B , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 10
   {"frczpd",    0x21005,0x0    , 0x12  , 0x124C, 0x24C , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 11
   {"frczss",    0x21005,0x0    , 0x12  , 0x104B, 0x4B  , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 12
   {"frczsd",    0x21005,0x0    , 0x12  , 0x104C, 0x4C  , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 13
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 14
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 15
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 16
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 17
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 18
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 19
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 1A
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 1B
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 1C
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 1D
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 1E
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 1F

   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 20
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 21
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 22
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 23
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 24
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 25
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 26
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 27
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 28
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 29
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 2A
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 2B
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 2C
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 2D
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 2E
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 2F

   {"cvtph2ps",  0x21007,0x0    , 0x12  , 0x124B, 0x402 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 30
   {"cvtps2ph",  0x21007,0x0    , 0x13  , 0x402,  0x124B, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 31
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 32
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 33
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 34
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 35
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 36
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 37
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 38
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 39
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 3A
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 3B
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 3C
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 3D
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 3E
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 3F

   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 40
   {"phaddbw",   0x21005,0x0    , 0x12  , 0x1402, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 41
   {"phaddbd",   0x21005,0x0    , 0x12  , 0x1403, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 42
   {"phaddbq",   0x21005,0x0    , 0x12  , 0x1404, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 43
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 44
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 45
   {"phaddwd",   0x21005,0x0    , 0x12  , 0x1403, 0x402 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 46
   {"phaddwq",   0x21005,0x0    , 0x12  , 0x1404, 0x402 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 47
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 48
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 49
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 4A
   {"phadddq",   0x21005,0x0    , 0x12  , 0x1404, 0x403 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 4B
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 4C
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 4D
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 4E
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 4F

   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 50
   {"phaddubw",  0x21005,0x0    , 0x12  , 0x1402, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 51
   {"phaddubd",  0x21005,0x0    , 0x12  , 0x1403, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 52
   {"phaddubq",  0x21005,0x0    , 0x12  , 0x1404, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 53
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 54
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 55
   {"phadduwd",  0x21005,0x0    , 0x12  , 0x1403, 0x402 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 56
   {"phadduwq",  0x21005,0x0    , 0x12  , 0x1404, 0x402 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 57
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 58
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 59
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 5A
   {"phaddudq",  0x21005,0x0    , 0x12  , 0x1404, 0x403 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 5B
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 5C
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 5D
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 5E
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 5F

   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 60
   {"phsubbw",   0x21005,0x0    , 0x12  , 0x1402, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 61
   {"phsubwd",   0x21005,0x0    , 0x12  , 0x1403, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 62
   {"phsubdq",   0x21005,0x0    , 0x12  , 0x1404, 0x401 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 63
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 64
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 65
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 66
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 67
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 68
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 69
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 6A
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 6B
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 6C
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 6D
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7A 6E
   {0,           0     , 0      , 0x12  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F 7A 6F

// Tertiary opcode map for 3-byte opcode. First two bytes = 0F 7B
// Indexed by third opcode byte
// AMD SSE5 instructions with two operands and an immediate byte operand
// Note: These proposed codes have never been implemented. 
SOpcodeDef OpcodeMap6B[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 00
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 01
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 02
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 03
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 04
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 05
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 06
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 07
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 08
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 09
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 0A
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 0B
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 0C
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 0D
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 0E
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 0F

   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 10
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 11
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 12
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 13
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 14
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 15
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 16
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 17
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 18
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 19
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 1A
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 1B
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 1C
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 1D
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 1E
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 1F

   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 20
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 21
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 22
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 23
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 24
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 25
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 26
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 27
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 28
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 29
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 2A
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 2B
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 2C
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 2D
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 2E
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 2F

   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 30
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 31
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 32
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 33
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 34
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 35
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 36
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 37
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 38
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 39
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 3A
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 3B
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 3C
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 3D
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 3E
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 3F

   {"protb"  ,   0x21005,0x0    , 0x52  , 0x1401, 0x401 , 0x21  , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 40
   {"protw"  ,   0x21005,0x0    , 0x52  , 0x1402, 0x402 , 0x21  , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 41
   {"protd"  ,   0x21005,0x0    , 0x52  , 0x1403, 0x403 , 0x21  , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 42
   {"protq"  ,   0x21005,0x0    , 0x52  , 0x1404, 0x404 , 0x21  , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 43
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 44
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 45
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 46
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 47
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 48
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 49
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 4A
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 4B
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 4C
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 4D
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 4E
   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 7B 4F

   {0,           0     , 0      , 0x52  , 0x1450, 0x450 , 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F 7B 50+


// Tertiary opcode map for vmread, insrtw, extrq. Opcode byte = 0F 78
// Indexed by prefix = none, 66, F2, F3
SOpcodeDef OpcodeMap6C[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vmread",    0x813 , 0x1000 , 0x13  , 0x4   , 0x1004, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 78. vmread              
   {0,           0x6E  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // 66 0F 78. link to map 6E: extrq xmm,xmm (AMD SSE4a)
   {"insrtq",    0x1004, 0x800  , 0x32  , 0x1450, 0x1450, 0x11  , 0x11  , 0     , 0     , 0     , 0     },    // F2 0F 78. insrtq xmm,xmm,i,i (AMD SSE4a)
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F 78. 

// Tertiary opcode map for vmwrite, insrtw, extrq. Opcode byte = 0F 79 without VEX prefix
// Indexed by prefix = none, 66, F2, F3
SOpcodeDef OpcodeMap6D[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vmwrite",   0x813 , 0x1000 , 0x12  , 0x1004, 0x4   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 79. vmwrite
   {"extrq",     0x1004, 0x200  , 0x12  , 0x1450, 0x1450, 0     , 0     , 0     , 0     , 0     , 0     },    // 66 0F 79. link to map 6E: extrq xmm,xmm (AMD SSE4a)
   {"insrtq",    0x1004, 0x800  , 0x12  , 0x1450, 0x1450, 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 79. insrtq xmm,xmm (AMD SSE4a)
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F 79. 

// Quarternary opcode map for extrq. Opcode byte = 66 0F 78
// Indexed by reg bits = 0 - 7
SOpcodeDef OpcodeMap6E[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"extrq",     0x1004, 0x200  , 0x31  , 0x1450, 0x11  , 0x11  , 0     , 0     , 0     , 0     , 0     },    // 66 0F 78. extrq xmm,i,i (AMD SSE4a)
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // reg bits must be 0

// Submap for movq xmm/m64,xmm. Opcode byte = 66 0F D6
// Indexed by memory vs. register operand
   SOpcodeDef OpcodeMap6F[2] = {
   {"movq",      0x12  ,0x812200, 0x13  , 0x4   , 0x1450, 0     , 0     , 0     , 0     , 0     , 0x2   },    // movq m64,xmm
   {"movq",      0x12  ,0x812200, 0x13  , 0x450 , 0x1450, 0     , 0     , 0     , 0     , 0     , 0x2   }};   // movq xmm,xmm

// Submap for movddup. Opcode byte = F2 0F 12
// Indexed by VEX.L
SOpcodeDef OpcodeMap70[4] = {
   {"movddup",   0x13  , 0x00800, 0x12  , 0x124C, 0x4C  , 0     , 0     , 0     , 0     , 0     , 0     },    // no VEX prefix
   {"vmovddup",  0x19  ,0x852800, 0x12  , 0x124C, 0x4C  , 0     , 0     , 0x20  , 0     , 0     , 0     },    // VEX.L = 0
   {"vmovddup",  0x19  ,0x852800, 0x12  , 0x124C, 0x24C , 0     , 0     , 0x20  , 0     , 0     , 0     },    // VEX.L = 1
   {"vmovddup",  0x19  ,0x852800, 0x12  , 0x124C, 0x24C , 0     , 0     , 0x20  , 0     , 0     , 0     }};   // EVEX.LL = 2

// Submap for movsd. Opcode byte = F2 0F 10
// Indexed by memory/register operand
SOpcodeDef OpcodeMap71[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movsd",     0x12  ,0x812800, 0x12  , 0x104C, 0x4C  , 0     , 0     , 0x30  , 0     , 0     , 0x2   },    // F2 0F 10 mem
   {"movsd",     0x12  ,0x892800, 0x19  , 0x104C, 0x104C, 0x104C, 0     , 0x30  , 0     , 0     , 0x2   }};   // F2 0F 10 reg

// Submap for movss. Opcode byte = F3 0F 10
// Indexed by memory/register operand
SOpcodeDef OpcodeMap72[2] = {
   {"movss",     0x12  ,0x812400,  0x12  , 0x104B, 0x4B  , 0     , 0     , 0x30  , 0     , 0     , 0x2   },    // F3 0F 10 mem
   {"movss",     0x12  ,0x892400,  0x19  , 0x104B, 0x104B, 0x104B, 0     , 0x30  , 0     , 0     , 0x2   }};   // F3 0F 10 reg

// Submap for movsd. Opcode byte = F2 0F 11
// Indexed by memory/register operand
SOpcodeDef OpcodeMap73[2] = {
   {"movsd",     0x12  ,0x812800, 0x13  , 0x4C  , 0x104C, 0     , 0     , 0x30  , 0     , 0     , 0x2   },    // F2 0F 11 mem
   {"movsd",     0x12  ,0x892800, 0x19  , 0x104C, 0x104C, 0x104C, 0     , 0x10  , 0     , 0     , 0x2   }};   // F2 0F 11 reg

// Submap for movss. Opcode byte = F3 0F 11
// Indexed by memory/register operand
SOpcodeDef OpcodeMap74[2] = {
   {"movss",     0x12  ,0x812400, 0x13  , 0x4B  , 0x104B, 0     , 0     , 0x10  , 0     , 0     , 0x2   },    // F3 0F 11 mem
   {"movss",     0x12  ,0x892400, 0x19  , 0x104B, 0x104B, 0x104B, 0     , 0x30  , 0     , 0     , 0x2   }};   // F3 0F 11 reg

// Submap for pinsrd/pinsrq. Opcode byte = 0F 3A 22
// Indexed by operand size
SOpcodeDef OpcodeMap75[3] = {
   {"pinsrd",    0x15  ,0x89B200, 0x59  , 0x1403, 0x1403, 0x3   , 0x11  , 0x1000, 0     , 0     , 0x2   },    // (16 bit). 66 prefix actually is 32 bits
   {"pinsrd",    0x15  ,0x89B200, 0x59  , 0x1403, 0x1403, 0x3   , 0x11  , 0x1000, 0     , 0     , 0x2   },    // 32 bit
   {"pinsrq",    0x15  ,0x89B200, 0x59  , 0x1404, 0x1404, 0x4   , 0x11  , 0x1000, 0     , 0     , 0x2   }};   // 64 bit. REX.W prefix

// Submap for sqrtps/pd/sd/ss. Opcode byte = 0F 51
// Indexed by prefix = none, 66, F2, F3
SOpcodeDef OpcodeMap76[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"sqrtps",    0x11  ,0x852E00, 0x12  , 0x124F, 0x24F , 0     , 0     , 0x37  , 0     , 0     , 0x2   },    //    0F 51. sqrtps
   {"sqrtpd",    0x11  ,0x852E00, 0x12  , 0x124F, 0x24F , 0     , 0     , 0x37  , 0     , 0     , 0x2   },    // 66 0F 51. sqrtpd
   {"sqrtsd",    0x11  ,0x892E00, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x36  , 0     , 0     , 0x2   },    // F2 0F 51. sqrtsd
   {"sqrtss",    0x11  ,0x892E00, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0x36  , 0     , 0     , 0x2   }};   // F3 0F 51. sqrtss

// Submap for rsqrtps/ss. Opcode byte = 0F 52
// Indexed by prefix = none, 66, F2, F3
SOpcodeDef OpcodeMap77[4] = {
   {"rsqrtps",   0x11  , 0x50E00, 0x12  , 0x124F, 0x24F , 0     , 0     , 0     , 0     , 0     , 0x2   },    // 0F 52. rsqrtps
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // illegal
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // illegal
   {"rsqrtss",   0x11  , 0x90E00, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0     , 0     , 0     , 0x2   }};   // F3 0F 52. rsqrtss

// Submap for rcpps/ss. Opcode byte = 0F 53
// Indexed by prefix = none, 66, F2, F3
SOpcodeDef OpcodeMap78[4] = {
   {"rcpps",     0x11  , 0x50E00, 0x12  , 0x124F, 0x24F , 0     , 0     , 0     , 0     , 0     , 0x2   },    // 0F 53. rcpps
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // illegal
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // illegal
   {"rcpss",     0x11  , 0x90E00, 0x19  , 0x124F, 0x124F, 0x24F , 0     , 0     , 0     , 0     , 0x2   }};   // F3 0F 53. rcpss

// Submap for emms/vzeroupper/vzeroall. Opcode byte = 0F 77
// Indexed by VEX prefix and VEX.L
SOpcodeDef OpcodeMap79[3] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"emms",      0x7   , 0      , 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 77
   {"vzeroupper",0x19  , 0x10000, 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 77, L=0
   {"vzeroall",  0x19  , 0x50000, 0x2   , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // VEX 0F 77, L=1

// Submap for pmovsxbw. Opcode byte = 0F 38 20. Indexed by memory/register operand
SOpcodeDef OpcodeMap7A[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pmovsxbw",  0x15  ,0x85A200, 0x12  , 0x1202, 0xF01 , 0     , 0     , 0x2220, 0     , 0     , 0x2   },    // 0F 38 20 mem, link by VEX.L
   {"pmovsxbw",  0x15  ,0x85A200, 0x12  , 0x1202, 0xF01 , 0     , 0     , 0x2220, 0     , 0     , 0x2   }};   // 0F 38 20 reg

// Submap for pmovsxbd. Opcode byte = 0F 38 21. Indexed by memory/register operand
SOpcodeDef OpcodeMap7B[2] = {
   {0,           0x7C  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F 38 21 mem, link by VEX.L
   {"pmovsxbd",  0x15  ,0x85A200, 0x12  , 0x1203, 0x401 , 0     , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F 38 21 reg

// Submap for pmovsxbd. Opcode byte = 0F 38 21 mem. Indexed by VEX.L
SOpcodeDef OpcodeMap7C[] = {
   {"pmovsxbd",  0x15  ,0x85A200, 0x12  , 0x1203, 0x3   , 0     , 0     , 0x2420 , 0     , 0     , 0x2   },    // 0F 38 21 L0
   {"pmovsxbd",  0x15  ,0x85A200, 0x12  , 0x1203, 0x301 , 0     , 0     , 0x2420 , 0     , 0     , 0x2   },    // 0F 38 21 L1
   {"pmovsxbd",  0x15  ,0x85A200, 0x12  , 0x1203, 0x401 , 0     , 0     , 0x2420 , 0     , 0     , 0x2   }};   // 0F 38 21 L2

// Submap for pmovsxbq. Opcode byte = 0F 38 22. Indexed by memory/register operand
SOpcodeDef OpcodeMap7D[2] = {
   {"pmovsxbq",  0x7E  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F 38 22 mem, link by VEX.L
   {"pmovsxbq",  0x15  ,0x858200, 0x12  , 0x1204, 0x401 , 0     , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F 38 22 reg

// Submap for pmovsxbq. Opcode byte = 0F 38 22 mem. Indexed by VEX.L
SOpcodeDef OpcodeMap7E[] = {
   {"pmovsxbq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x2   , 0     , 0     , 0x2620, 0     , 0     , 0x2   },    // 0F 38 22 L0
   {"pmovsxbq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x3   , 0     , 0     , 0x2620, 0     , 0     , 0x2   },    // 0F 38 22 L1
   {"pmovsxbq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x4   , 0     , 0     , 0x2620, 0     , 0     , 0x2   }};   // 0F 38 22 L2

// Submap for pmovsxwd. Opcode byte = 0F 38 23. Indexed by memory/register operand
SOpcodeDef OpcodeMap7F[2] = {
   {"pmovsxwd",  0x15  ,0x85A200, 0x12  , 0x1203, 0xF02 , 0     , 0     , 0x2220, 0     , 0     , 0x2   },    // 0F 38 23 mem, link by VEX.L
   {"pmovsxwd",  0x15  ,0x85A200, 0x12  , 0x1203, 0xF02 , 0     , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F 38 23 reg

// Submap for pmovsxwq. Opcode byte = 0F 38 24. Indexed by memory/register operand
SOpcodeDef OpcodeMap80[2] = {
   {"pmovsxwq",  0x81  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F 38 24 mem, link by VEX.L
   {"pmovsxwq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x402 , 0     , 0     , 0x30  , 0     , 0     , 0x2   }};   // 0F 38 24 reg

// Submap for pmovsxwq. Opcode byte = 0F 38 24 mem. Indexed by VEX.L
SOpcodeDef OpcodeMap81[] = {
   {"pmovsxwq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x3   , 0     , 0     , 0x2420, 0     , 0     , 0x2   },    // 0F 38 24 L0
   {"pmovsxwq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x302 , 0     , 0     , 0x2420, 0     , 0     , 0x2   },    // 0F 38 24 L1
   {"pmovsxwq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x402 , 0     , 0     , 0x2420, 0     , 0     , 0x2   }};   // 0F 38 24 L1

// Submap for pmovsxdq. Opcode byte = 0F 38 25. Indexed by memory/register operand
SOpcodeDef OpcodeMap82[2] = {
   {"pmovsxdq",  0x15  ,0x85A200, 0x12  , 0x1204, 0xF03 , 0     , 0     , 0x2220, 0     , 0     , 0x2   },    // 0F 38 25 mem, link by VEX.L
   {"pmovsxdq",  0x15  ,0x85A200, 0x12  , 0x1204, 0xF03 , 0     , 0     , 0x2220, 0     , 0     , 0x2   }};   // 0F 38 25 reg

// Submap for pmovzxbw. Opcode byte = 0F 38 30. Indexed by memory/register operand
SOpcodeDef OpcodeMap83[2] = {
   {"pmovzxbw",  0x15  ,0x85A200, 0x12  , 0x1202, 0xF01 , 0     , 0     , 0x2220, 0     , 0     , 0x2   },    // 0F 38 30 L0
   {"pmovzxbw",  0x15  ,0x85A200, 0x12  , 0x1202, 0xF01 , 0     , 0     , 0x2220, 0     , 0     , 0x2   }};   // 0F 38 30 reg

// Submap for 0F 38 5A, indexed by L bit and MVEX for vector size
SOpcodeDef OpcodeMap84[] = {
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 5A, 128 bits
   {"vbroadcasti128",0x1C,0x978200,0x12 , 0x1550, 0x2451, 0     , 0     , 0x20  , 0     , 0     , 0     },    // 0F 38 5A, 256 bits
   {"vbroadcasti32x4",0x80,0xC28200,0x12, 0x1603, 0x2403, 0     , 0     , 0x20  , 0x1012, 0     , 0x100 },    // 0F 38 5A, 512 bits
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

// Submap for pmovzxbd. Opcode byte = 0F 38 31. Indexed by memory/register operand
SOpcodeDef OpcodeMap85[2] = {
   {"pmovzxbd",  0x86  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F 38 31 mem, link by VEX.L
   {"pmovzxbd",  0x15  ,0x85A200, 0x12  , 0x1203, 0x401 , 0     , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F 38 31 reg

// Submap for pmovzxbd. Opcode byte = 0F 38 31 mem. Indexed by VEX.L
SOpcodeDef OpcodeMap86[] = {
   {"pmovzxbd",  0x15  ,0x85A200, 0x12  , 0x1403, 0x3   , 0     , 0     , 0x2420 , 0     , 0     , 0x2   },    // 0F 38 31 L0
   {"pmovzxbd",  0x15  ,0x85A200, 0x12  , 0x1503, 0x301 , 0     , 0     , 0x2420 , 0     , 0     , 0x2   },    // 0F 38 31 L1
   {"pmovzxbd",  0x15  ,0x85A200, 0x12  , 0x1603, 0x401 , 0     , 0     , 0x2420 , 0     , 0     , 0x2   }};   // 0F 38 31 L2

// Submap for pmovzxbq. Opcode byte = 0F 38 32. Indexed by memory/register operand
SOpcodeDef OpcodeMap87[2] = {
   {"pmovzxbq",  0x88  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F 38 32 mem, link by VEX.L
   {"pmovzxbq",  0x15  ,0x858200, 0x12  , 0x1204, 0x401 , 0     , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F 38 32 reg

// Submap for pmovzxbq. Opcode byte = 0F 38 32 mem. Indexed by VEX.L
SOpcodeDef OpcodeMap88[] = {
   {"pmovzxbq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x2   , 0     , 0     , 0x2620, 0     , 0     , 0x2   },    // 0F 38 32 L0
   {"pmovzxbq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x3   , 0     , 0     , 0x2620, 0     , 0     , 0x2   },    // 0F 38 32 L1
   {"pmovzxbq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x4   , 0     , 0     , 0x2620, 0     , 0     , 0x2   }};   // 0F 38 32 L2

// Submap for pmovzxwd. Opcode byte = 0F 38 33. Indexed by memory/register operand
SOpcodeDef OpcodeMap89[2] = {
   {"pmovzxwd",  0x15  ,0x85A200, 0x12  , 0x1203, 0xF02 , 0     , 0     , 0x2220, 0     , 0     , 0x2   },    // 0F 38 33 mem, link by VEX.L
   {"pmovzxwd",  0x15  ,0x85A200, 0x12  , 0x1203, 0xF02 , 0     , 0     , 0x2220, 0     , 0     , 0x2   }};   // 0F 38 33 reg

// Submap for pmovzxwq. Opcode byte = 0F 38 34. Indexed by memory/register operand
SOpcodeDef OpcodeMap8A[2] = {
   {"pmovzxwq",  0x8B  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x0D  , 0     },    // 0F 38 34 mem, link by VEX.L
   {"pmovzxwq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x402 , 0     , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F 38 34 reg

// Submap for pmovzxwq. Opcode byte = 0F 38 34 mem. Indexed by VEX.L
SOpcodeDef OpcodeMap8B[] = {
   {"pmovzxwq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x3   , 0     , 0     , 0x2420, 0     , 0     , 0x2   },    // 0F 38 34 L0
   {"pmovzxwq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x302 , 0     , 0     , 0x2420, 0     , 0     , 0x2   },    // 0F 38 34 L1
   {"pmovzxwq",  0x15  ,0x85A200, 0x12  , 0x1204, 0x402 , 0     , 0     , 0x2420, 0     , 0     , 0x2   }};   // 0F 38 34 L1

// Submap for pmovzxwq. Opcode byte = 0F 38 35. Indexed by memory/register operand
SOpcodeDef OpcodeMap8C[2] = {
   {"pmovzxdq",  0x15  ,0x85A200, 0x12  , 0x1204, 0xF03 , 0     , 0     , 0x2220, 0     , 0     , 0x2   },    // 0F 38 35 mem
   {"pmovzxdq",  0x15  ,0x85A200, 0x12  , 0x1204, 0xF03 , 0     , 0     , 0x2220, 0     , 0     , 0x2   }};   // 0F 38 35 reg

// submap for 0F 38 14. Indexed by VEX prefix
SOpcodeDef OpcodeMap8D[] = {
   {"blendvps",  0x15  , 0x8200 , 0x12  , 0x124B, 0x24B , 0xAE  , 0     , 0     , 0     , 0     , 0     },    //      0F 38 14
   {"vprorv",    0x20  ,0x883200, 0x19  , 0x1209, 0x1209, 0x0209, 0     , 0x31  , 0     , 0     , 0x1   }};   //  VEX 0F 38 14

// submap for 0F 38 15. Indexed by VEX prefix
SOpcodeDef OpcodeMap8E[] = {
   {"blendvpd",  0x15  , 0x8200 , 0x12  , 0x124C, 0x24C , 0xAE  , 0     , 0     , 0     , 0     , 0     },    //     0F 38 15
   {"vprolv",    0x20  ,0x883200, 0x19  , 0x1209, 0x1209, 0x0209, 0     , 0x31  , 0     , 0     , 0x1   }};   // VEX 0F 38 15

// submap for 0F 3A 23. Index by W bit
SOpcodeDef OpcodeMap8F[] = {
   {"vshuff32x4",0x20  ,0x88B200, 0x59  , 0x124F, 0x124F, 0x024F, 0x31  , 0x31  , 0     , 0     , 0     },    // 0F 3A 23. W0
   {"vshuff64x2",0x20  ,0x88B200, 0x59  , 0x124F, 0x124F, 0x024F, 0x31  , 0x31  , 0     , 0     , 0     }};   // 0F 3A 23. W1

// submap for 0F 3A 43. Index by W bit
SOpcodeDef OpcodeMap90[] = {
   {"vshufi32x4",0x20  ,0x88B200, 0x59  , 0x1209, 0x1209, 0x0209, 0x31  , 0x31  , 0     , 0     , 0     },    // 0F 3A 43. W0
   {"vshufi64x2",0x20  ,0x88B200, 0x59  , 0x1209, 0x1209, 0x0209, 0x31  , 0x31  , 0     , 0     , 0     }};   // 0F 3A 43. W1

// submap for 0F 38 2A. Index by 66 F2 F3 prefix
SOpcodeDef OpcodeMap91[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 2A
   {"movntdqa",  0x15  ,0x85A200, 0x12  , 0x1250, 0x2250, 0     , 0     , 0x00  , 0     , 0     , 0x102 },    // 66 0F 38 2A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 38 2A
   {"vpbroadcastmb2q",0x20,0x823400,0x12, 0x1204, 0x1095, 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F 38 2A. W1

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
// Submap for xlat. Opcode byte = D7. Indexed by assembly syntax: 0=MASM, 1=NASM/YASM, 2=GAS
SOpcodeDef OpcodeMap92[3] = {
   {"xlat",      0     , 0x5    , 0x1   , 0     , 0x20C0, 0     , 0     , 0     , 0     , 0     , 0x8   },    // D7
   {"xlatb",     0     , 0x5    , 0x1   , 0     , 0x20C0, 0     , 0     , 0     , 0     , 0     , 0x8   },    // D7
   {"xlat",      0     , 0x5    , 0x1   , 0     , 0x20C0, 0     , 0     , 0     , 0     , 0     , 0x8   }};   // D7

// Submap for pmovmskb, Opcode 0F D7, Indexed by VEX prefix and VEX.L bit
SOpcodeDef OpcodeMap93[3] = {
   {"pmovmskb",  0x7   , 0x51200, 0x12  , 0x1009, 0x1150, 0     , 0     , 0     , 0     , 0     , 0x2   },    // 0F D7
   {"pmovmskb",  0x7   , 0x51200, 0x12  , 0x100A, 0x1150, 0     , 0     , 0     , 0     , 0     , 0x2   },    // 0F D7, VEX    (32/64 bit dest. depending on mode)
   {"pmovmskb",  0x7   , 0x51200, 0x12  , 0x100A, 0x1150, 0     , 0     , 0     , 0     , 0     , 0x2   }};   // 0F D7, VEX.L  (32/64 bit dest. depending on mode)

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
// Submap for vpgatherq, Opcode 0F 38 91, Indexed by VEX/EVEX
SOpcodeDef OpcodeMap94[2] = {
   {0,           0x105 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 
   {0,           0x106 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};   // 

// Submap for vgatherqps/pd, Opcode 0F 38 93, Indexed by VEX.W bit
SOpcodeDef OpcodeMap95[2] = {
   {"vgatherqps",0x1C  , 0xE9200, 0x1E,   0xF4B , 0x224B, 0xF4B , 0     , 0     , 0     , 0     , 0     },    // 0F 38 93, W0
   {"vgatherqpd",0x1C  , 0xE9200, 0x1E,   0x24C , 0x224C, 0x24C , 0     , 0     , 0     , 0     , 0     }};   // 0F 38 93, W1

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
// Submap for psllw, Opcode 0F F1, Indexed by VEX.L bit
SOpcodeDef OpcodeMap96[] = {
   {"psllw",     0x7   ,0x8D2200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F F1
   {"psllw",     0x7   ,0x8D2200, 0x19  , 0x1202, 0x1202, 0x402 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F F1, L1
   {"psllw",     0x20  ,0x8D2200, 0x19  , 0x1202, 0x1202, 0x402 , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F F1, L2

// Submap for pslld, Opcode 0F F2, Indexed by VEX.L bit
SOpcodeDef OpcodeMap97[] = {
   {"pslld",     0x7   ,0x8D2200, 0x19  , 0x1103, 0x1103, 0x103 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F F2
   {"pslld",     0x7   ,0x8D2200, 0x19  , 0x1203, 0x1203, 0x403 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F F2, L1
   {"pslld",     0x20  ,0x8D2200, 0x19  , 0x1203, 0x1203, 0x403 , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F F2, L1

// Submap for psllq, Opcode 0F F3, Indexed by VEX.L bit
SOpcodeDef OpcodeMap98[] = {
   {"psllq",     0x7   ,0x8D2200, 0x19  , 0x1104, 0x1104, 0x104 , 0     , 0x30  , 0     , 0     , 0x2   },    // 0F F3
   {"psllq",     0x7   ,0x8D2200, 0x19  , 0x1204, 0x1204, 0x404 , 0     , 0x30  , 0     , 0     , 0x2   },    // 0F F3, L1
   {"psllq",     0x20  ,0x8D2200, 0x19  , 0x1204, 0x1204, 0x404 , 0     , 0x30  , 0     , 0     , 0x2   }};   // 0F F3, L1

// Submap for psrlw, Opcode 0F D1, Indexed by VEX.L bit
SOpcodeDef OpcodeMap99[] = {
   {"psrlw",     0x7   ,0x8D2200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F D1
   {"psrlw",     0x7   ,0x8D2200, 0x19  , 0x1202, 0x1202, 0x402 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F D1
   {"psrlw",     0x7   ,0x8D2200, 0x19  , 0x1202, 0x1202, 0x402 , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F D1

// Submap for psrld, Opcode 0F D2, Indexed by VEX.L bit
SOpcodeDef OpcodeMap9A[] = {
   {"psrld",     0x7   ,0x8D2200, 0x19  , 0x1103, 0x1103, 0x103 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F D2
   {"psrld",     0x7   ,0x8D2200, 0x19  , 0x1203, 0x1203, 0x403 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F D2
   {"psrld",     0x7   ,0x8D2200, 0x19  , 0x1203, 0x1203, 0x403 , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F D2

// Submap for psrlq, Opcode 0F D3, Indexed by VEX.L bit
SOpcodeDef OpcodeMap9B[] = {
   {"psrlq",     0x7   ,0x8D2200, 0x19  , 0x1104, 0x1104, 0x104 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F D3
   {"psrlq",     0x7   ,0x8D2200, 0x19  , 0x1204, 0x1204, 0x404 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F D3
   {"psrlq",     0x7   ,0x8D2200, 0x19  , 0x1204, 0x1204, 0x404 , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F D3

// Submap for psraw, Opcode 0F E1, Indexed by VEX.L bit
SOpcodeDef OpcodeMap9C[] = {
   {"psraw",     0x7   ,0x8D2200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F E1
   {"psraw",     0x7   ,0x8D2200, 0x19  , 0x1202, 0x1202, 0x402 , 0     , 0x20  , 0     , 0     , 0x2   },    // 0F E1
   {"psraw",     0x7   ,0x8D2200, 0x19  , 0x1202, 0x1202, 0x402 , 0     , 0x20  , 0     , 0     , 0x2   }};   // 0F E1

// Submap for psrad, Opcode 0F E2, Indexed by VEX.L bit
SOpcodeDef OpcodeMap9D[] = {
   {"psra",      0x7   ,0x8D3200, 0x19  , 0x1109, 0x1109, 0x109 , 0     , 0x21  , 0     , 0     , 0x3   },    // 0F E2
   {"psra",      0x7   ,0x8D3200, 0x19  , 0x1109, 0x1109, 0x409 , 0     , 0x21  , 0     , 0     , 0x3   },    // 0F E2. w bit specifies operand size only if EVEX
   {"psra",      0x7   ,0x8D3200, 0x19  , 0x1109, 0x1109, 0x409 , 0     , 0x21  , 0     , 0     , 0x3   }};   // 0F E2. w bit specifies operand size only if EVEX

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
// Submap for vpbroadcastb, Opcode 0F 38 78, Indexed by memory/register
SOpcodeDef OpcodeMap9E[2] = {
   {"vpbroadcastb",0x1C,0x878200, 0x12,   0x1201, 0x1   , 0     , 0     , 0x20  , 0     , 0     , 0     },    // 0F 38 78 mem
   {"vpbroadcastb",0x1C,0x878200, 0x12,   0x1201, 0x401 , 0     , 0     , 0x20  , 0     , 0     , 0     }};   // 0F 38 78 reg

// Submap for vpbroadcastw, Opcode 0F 38 79, Indexed by memory/register
SOpcodeDef OpcodeMap9F[2] = {
   {"vpbroadcastw",0x1C,0x878200, 0x12,   0x1201, 0x2   , 0     , 0     , 0x20  , 0     , 0     , 0     },    // 0F 38 79 mem
   {"vpbroadcastw",0x1C,0x878200, 0x12,   0x1201, 0x402 , 0     , 0     , 0x20  , 0     , 0     , 0     }};   // 0F 38 79 reg

// Submap for vpbroadcastd, Opcode 0F 38 58, Indexed by memory/register
SOpcodeDef OpcodeMapA0[2] = {
   {"vpbroadcastd",0x1C,0xC78200, 0x12,   0x1201, 0x3   , 0     , 0     , 0x20  , 0x100A, 0     , 0     },    // 0F 38 58 mem
   {"vpbroadcastd",0x1C,0x878200, 0x12,   0x1201, 0x403 , 0     , 0     , 0x20  , 0     , 0     , 0     }};   // 0F 38 58 reg

// Submap for vpbroadcastq, Opcode 0F 38 59, Indexed by memory/register
SOpcodeDef OpcodeMapA1[2] = {
   {"vpbroadcastq",0x1C,0xC7B200, 0x12,   0x1201, 0x4   , 0     , 0     , 0x20  , 0x100B, 0     , 0     },    // 0F 38 59 mem. (manuals disagree on W bit?)
   {"vpbroadcastq",0x1C,0x879200, 0x12,   0x1201, 0x404 , 0     , 0     , 0x20  , 0     , 0     , 0     }};   // 0F 38 59 reg

// Submap for 0F 38 F3. Indexed by reg bit
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
SOpcodeDef OpcodeMapA2[8] = {
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },   // 0F 38 F3 /0
   {"blsr"  ,    0x1D   , 0xB1000, 0x18 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },   // 0F 38 F3 /3
   {"blsmsk",    0x1D   , 0xB1000, 0x18 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },   // 0F 38 F3 /3
   {"blsi"  ,    0x1D   , 0xB1000, 0x18 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },   // 0F 38 F3 /3
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },   // 0F 38 F3 /4
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },   // 0F 38 F3 /5
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },   // 0F 38 F3 /6
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};  // 0F 38 F3 /7

// Submap for 0F 38 F5. Indexed by prefixes
SOpcodeDef OpcodeMapA3[4] = {
   {"bzhi"  ,    0x1D   , 0xB1000, 0x1B  , 0x1009, 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     },   // 0F 38 F5
   {"wruss" ,    0      , 0x1200 , 0x13  , 0x2009, 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },   // 66 0F 38 F5
   {"pdep"  ,    0x1D   , 0xB1000, 0x19  , 0x1009, 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     },   // F2 0F 38 F5
   {"pext"  ,    0x1D   , 0xB1000, 0x19  , 0x1009, 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     }};  // F3 0F 38 F5

// Submap for 0F 3A F0. Indexed by prefixes
SOpcodeDef OpcodeMapA4[4] = {
   {0       ,    0      , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },   // 0F 3A F0
   {0       ,    0      , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },   // 66 0F 3A F0
   {"rorx"  ,    0x1D   , 0x31000, 0x52  , 0x1009, 0x9   , 0x31  , 0     , 0     , 0     , 0     , 0     },   // F2 0F 3A F0
   {0       ,    0      , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};  // F3 0F 3A F0

// Quarternary opcode map for pinsrb. Opcode byte = 0F 3A 20
// Indexed by memory vs. register operand
SOpcodeDef OpcodeMapA5[2] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pinsrb",    0x15  ,0x89A200, 0x59  , 0x1401, 0x1401, 0x2001, 0x31  , 0x1000, 0     , 0     , 0x2   },   // 0F 3A 20 memory 8
   {"pinsrb",    0x15  ,0x89A200, 0x59  , 0x1401, 0x1401, 0x1003, 0x31  , 0     , 0     , 0     , 0x2   }};  // 0F 3A 20 register 32

// Opcode map for VIA instructions. Opcode byte = 0F A6 ..
// Indexed by mod and reg fields of mod/reg/rm byte
SOpcodeDef OpcodeMapA6[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A6, mod<3, reg=0
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A6, mod<3, reg=1
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A6, mod<3, reg=2
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A6, mod<3, reg=3
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A6, mod<3, reg=4
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A6, mod<3, reg=5
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A6, mod<3, reg=6
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A6, mod<3, reg=7
   {"rep montmul;VIA",0x2001,0x8021,0x10, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // F3 0F A6 /0
   {"rep xsha1;VIA",0x2001,0x8021,0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // F3 0F A6 /1
   {"rep xsha256;VIA",0x2001,0x8021,0x10, 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // F3 0F A6 /2
   {0,           0     , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }}; // 0F A6, mod=3, reg=3

// Opcode map for VIA instructions. Opcode byte = 0F A7 ..
// Indexed by mod and reg fields of mod/reg/rm byte
SOpcodeDef OpcodeMapA7[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A7, mod<3, reg=0
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A7, mod<3, reg=1
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A7, mod<3, reg=2
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A7, mod<3, reg=3
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A7, mod<3, reg=4
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A7, mod<3, reg=5
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A7, mod<3, reg=6
   {0,           0     , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A7, mod<3, reg=7
   {0,           0xA8  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },  // 0F A7, mod=3, reg=0. Link to XSTORE
   {"rep xcryptecb;VIA",0x2001,0x8021,0x10,0    , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // F3 0F A7 /1
   {"rep xcryptcbc;VIA",0x2001,0x8021,0x10,0    , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // F3 0F A7 /2
   {"rep xcryptctr;VIA",0x2001,0x8021,0x10,0    , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // F3 0F A7 /3
   {"rep xcryptcfb;VIA",0x2001,0x8021,0x10,0    , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // F3 0F A7 /4
   {"rep xcryptofb;VIA",0x2001,0x8021,0x10,0    , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // F3 0F A7 /5
   {0,           0     , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }}; // 0F A7, mod=3, reg=6

// Opcode map for VIA XSTORE instruction. Opcode byte = 0F A7 /0
// Indexed by prefixes
SOpcodeDef OpcodeMapA8[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"xstore;VIA",0x2001, 1      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F A7 /0
   {0,           0     , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 66 0F A7 /0
   {0,           0     , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // F2 0F A7 /0
   {"rep xstore;VIA",0x2001,0x21, 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   }}; // F3 0F A7 /0

// Opcode map for XGETBV, XSETBV instruction. Opcode byte = 0F 01 /2
// Indexed by rm bits
SOpcodeDef OpcodeMapA9[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"xgetbv",    0x16   , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // 0F 01 D0
   {"xsetbv",    0x16   , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F 01 D1
   {0       ,    0      , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F 01 D2
   {0       ,    0      , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F 01 D3
   {0       ,    0      , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F 01 D4
   {"xend"  ,    0x1D   , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F 01 D5
   {"xtest" ,    0x1D   , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F 01 D6
   {0       ,    0      , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }}; // 0F 01 D7

// Opcode map for AMD virtualization instructions 0F 1F 11/011/xxx
// Indexed by rm bits
SOpcodeDef OpcodeMapAA[] = {
   {"vmrun" ,   0x1804  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // 0F 01 D8
   {"vmmcall",  0x1804  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // 0F 01 D9
   {"vmload",   0x1804  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // 0F 01 DA
   {"vmsave",   0x1804  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // 0F 01 DB
   {"stgi"  ,   0x1804  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // 0F 01 DC
   {"clgi"  ,   0x1804  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // 0F 01 DD
   {"skinit",   0x1804  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // 0F 01 DE
   {0       ,   0x1804  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   }}; // 0F 01 DF

// Opcode map for swapgs and RDTSCP instructions 0F 1F 11/111/xxx
// Indexed by rm bits
SOpcodeDef OpcodeMapAB[] = {
   {"swapgs",    0x800  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // 0F 01 F8. instruction set unknown
   {"rdtscp",    0x19   , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x8   },  // 0F 01 F9. AMD SSE4.A and Intel AVX?
   {0       ,    0      , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F 01 FA
   {0       ,    0      , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F 01 FB
   {"clzero",    0      , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F 01 FC. AMD
   {0       ,    0      , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F 01 FD
   {0       ,    0      , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },  // 0F 01 FE
   {0       ,    0      , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }}; // 0F 01 FF

// Opcode map for 0F C7 /6
// Indexed by mem/reg
SOpcodeDef OpcodeMapAC[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0x52  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // 0F C7 /6 mem link to vmptrld etc
   {"rdrand",    0x1D  , 0x1100 , 0x11  , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 0F C7 /6 reg

// Submap for 0F 38 F7, indexed by prefixes
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
SOpcodeDef OpcodeMapAD[] = {
   {"bextr",     0x1D  , 0xB3000, 0x1B  , 0x1009, 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     },    // 0F 38 F7
   {"shlx",      0x1D  , 0xB3200, 0x1B  , 0x1009, 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     },    // 66 0F 38 F7
   {"shrx",      0x1D  , 0xB3800, 0x1B  , 0x1009, 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     },    // F2 0F 38 F7
   {"sarx",      0x1D  , 0xB3400, 0x1B  , 0x1009, 0x9   , 0x1009, 0     , 0     , 0     , 0     , 0     }};   // F3 0F 38 F7

// Submap for 0F BC, indexed by prefixes
SOpcodeDef OpcodeMapAE[4] = {
   {"bsf"   ,    0x3    , 0x1100 , 0x12 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F BC
   {"bsf"   ,    0x3    , 0x1100 , 0x12 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 66 0F BC
   {"tzcnti",    0x20   ,0x31800 , 0x12 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F BC
   {"tzcnt" ,    0x1D   ,0x11500 , 0x12 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F BC, or 66 F3 0F BC. Does not work for F3 66 0F BC!

// Submap for 0F C7 /7, Indexed by mem/reg
SOpcodeDef OpcodeMapAF[] = {
   {"vmptrst",   0x813  , 0      , 0x11  , 0x2351, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F C7 /7 mem
   {0       ,    0x138  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 9     , 0     }};   // 0F C7 /7 reg. link to rdseed, rdpid


// Shortcut opcode map for VEX prefix and mmmm = 0000
// Indexed by first opcode byte after VEX prefix. With or without mod/reg/rm byte, and any number of immediate bytes
SOpcodeDef OpcodeMapB0[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 00
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 01
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 02
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 03
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 04
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 05
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 06
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 07
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 08
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 09
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 10
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 11
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 12
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 13
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 14
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 15
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 16
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 17
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 18
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 19
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 1A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 1B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 1C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 1D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 1E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 1F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 20
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 21
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 22
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 23
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 24
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 25
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 26
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 27
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 28
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 29
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 2A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 2B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 2C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 2D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 2E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 2F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 30
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 31
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 32
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 33
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 34
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 35
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 36
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 37
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 38
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 39
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 3A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 3B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 3C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 3D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 3E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 3F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 40
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 41
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 42
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 43
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 44
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 45
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 46
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 47
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 48
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 49
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 4A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 4B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 4C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 4D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 4E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 4F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 50
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 51
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 52
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 53
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 54
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 55
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 56
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 57
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 58
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 59
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 5A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 5B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 5C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 5D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 5E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 5F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 60
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 61
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 62
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 63
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 64
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 65
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 66
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 67
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 68
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 69
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 6A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 6B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 6C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 6D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 6E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 6F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 70
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 71
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 72
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 73
   {"jkzd",      0x20  , 0xB0080, 0x44  , 0x95  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // VEX 74
   {"jknzd",     0x20  , 0xB0080, 0x44  , 0x95  , 0x81  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // VEX 75
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 76
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 77
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 78
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 79
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 7A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 7B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 7C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 7D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 7E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // VEX 7F

// Shortcut opcode map for VEX or EVEX prefix and mmmm = 0001
// Important: if VEX prefix is optional then use OpcodeMap1 instead. Don't put the same code in both maps!
// Indexed by first opcode byte after VEX prefix. With or without mod/reg/rm byte, and any number of immediate bytes
SOpcodeDef OpcodeMapB1[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 00
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 01
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 02
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 03
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 04
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 05
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 06
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 07
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 08
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 09
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 0A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 0B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 0C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 0D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 0E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 0F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 10
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 11
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 12
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 13
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 14
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 15
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 16
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 17
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 18
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 19
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 1A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 1B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 1C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 1D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 1E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 1F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 20
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 21
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 22
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 23
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 24
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 25
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 26
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 27
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 28
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 29
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 2A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 2B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 2C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 2D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 2E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 2F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 30
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 31
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 32
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 33
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 34
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 35
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 36
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 37
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 38
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 39
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 3A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 3B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 3C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 3D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 3E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 3F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 40
   {"kand",      0x20  , 0xE5200, 0x19  , 0x1095, 0x1095, 0x1095, 0     , 0     , 0     , 0     , 1     },    // VEX 0F 41
   {"kandn",     0x20  , 0xE5200, 0x19  , 0x1095, 0x1095, 0x1095, 0     , 0     , 0     , 0     , 1     },    // VEX 0F 42
   {"kandnr",    0x80  , 0x30000, 0x12  , 0x95  , 0x1095, 0     , 0     , 0     , 0     , 0     , 1     },    // VEX 0F 43
   {"knot",      0x20  , 0xE5200, 0x12  , 0x1095, 0x1095, 0     , 0     , 0     , 0     , 0     , 1     },    // VEX 0F 44
   {"kor",       0x20  , 0xE5200, 0x19  , 0x1095, 0x1095, 0x1095, 0     , 0     , 0     , 0     , 1     },    // VEX 0F 45
   {"kxnor",     0x20  , 0xE5200, 0x19  , 0x1095, 0x1095, 0x1095, 0     , 0     , 0     , 0     , 1     },    // VEX 0F 46
   {"kxor",      0x20  , 0xE5200, 0x19  , 0x1095, 0x1095, 0x1095, 0     , 0     , 0     , 0     , 1     },    // VEX 0F 47
   {"kmerge2l1h",0x80  , 0x30000, 0x12  , 0x95  , 0x1095, 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 48
   {"kmerge2l1l",0x80  , 0x30000, 0x12  , 0x95  , 0x1095, 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 49
   {"kadd",      0x20  , 0xE5200, 0x19  , 0x1095, 0x1095, 0x1095, 0     , 0     , 0     , 0     , 1     },    // VEX 0F 41
   {0,           0xF0  , 0      , 0x19  , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // VEX 0F 4B. Link to kunpckbw
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 4C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 4D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 4E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 4F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 50
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 51
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 52
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 53
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 54
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 55
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 56
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 57
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 58
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 59
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 5A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 5B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 5C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 5D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 5E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 5F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 60
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 61
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 62
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 63
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 64
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 65
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 66
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 67
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 68
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 69
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 6A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 6B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 6C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 6D
   {"vmov",      0x7   ,0x813200, 0x12  , 0x1409, 0x9   , 0     , 0     , 0x00  , 0     , 0     , 0x1   },    // VEX 0F 6E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 6F
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 70
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 71
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 72
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 73
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 74
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 75
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 76
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 77
   {0,           0xDA  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // EVEX 0F 78. Link to vcvttpd2udq
   {0,           0xD6  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // EVEX 0F 79. Link to vcvtps etc
   {0,           0xDC  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // E/MVEX 0F 7A. Link to vcvtudq2pd
   {0,           0xDD  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // EVEX 0F 7B. Link to vcvtusi2sd etc
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 7C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 7D
   {0,           0xE2  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // VEX 0F 7E. Link to movq
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 7F
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 80
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 81
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 82
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 83
   {"jkzd",      0x20  , 0xB0080, 0x84  , 0x95  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // VEX 0F 84
   {"jknzd",     0x20  , 0xB0080, 0x84  , 0x95  , 0x82  , 0     , 0     , 0     , 0     , 0     , 0x80  },    // VEX 0F 85
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 86
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 87
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 88
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 89
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 8A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 8B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 8C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 8D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 8E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 8F
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0xEB  , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 3     ,       },    // VEX 0F 90. Link to kmov
   {"kmov",      0x20  , 0x35200, 0x13  , 0x2009, 0x1095, 0     , 0     , 0     , 0     , 0     , 1     },    // VEX 0F 91. Name without w in KNC syntax, but code identical
   {0,           0xEC  , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // VEX 0F 92. Link to kmov r, k
   {0,           0xEE  , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     },    // VEX 0F 93. Link to kmov k, r
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 94
   {"kconcath",  0x80  , 0xB0000, 0x19  , 0x1004, 0x1095, 0x1095, 0     , 0     , 0     , 0     , 0     },    // VEX 0F 95
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 96
   {"kconcatl",  0x80  , 0xB0000, 0x19  , 0x1004, 0x1095, 0x1095, 0     , 0     , 0     , 0     , 0     },    // VEX 0F 97
   {"kortest",   0x20  , 0x25200, 0x12  , 0x95  , 0x1095, 0     , 0     , 0     , 0     , 0     , 1     },    // VEX 0F 98
   {"ktest",     0x20  , 0x25200, 0x12  , 0x1095, 0x1095, 0     , 0     , 0     , 0     , 0     , 1     },    // VEX 0F 99
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 9A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 9B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 9C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 9D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 9E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 9F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F A0
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F A1
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F A2
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F A3
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F A4
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F A5
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F A6
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F A7
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F A8
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F A9
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F AA
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F AB
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F AC
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F AD
   {0,           0xCD  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x4   , 0     },    // VEX 0F AE. Link
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // VEX 0F AF


// Shortcut opcode map for EVEX F2 0F 38
// Indexed by first opcode byte after EVEX prefix
SOpcodeDef OpcodeMapB2[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 00
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 01
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 02
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 03
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 04
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 05
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 06
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 07
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 08
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 09
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 0A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 0B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 0C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 0D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 0E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 0F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 10
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 11
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 12
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 13
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 14
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 15
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 16
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 17
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 18
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 19
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 1A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 1B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 1C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 1D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 1E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 1F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 20
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 21
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 22
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 23
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 24
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 25
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 26
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 27
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 28
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 29
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 2A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 2B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 2C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 2D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 2E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 2F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 30
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 31
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 32
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 33
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 34
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 35
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 36
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 37
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 38
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 39
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 3A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 3B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 3C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 3D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 3E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 3F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 40
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 41
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 42
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 43
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 44
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 45
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 46
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 47
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 48
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 49
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 4A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 4B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 4C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 4D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 4E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 4F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 50
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 51
   {"vp4dpwssd" ,0x24  ,0x8F9800, 0x19  , 0x164B, 0x164B, 0x244B, 0     , 0x20  , 0     , 0     , 0     },    // EVEX F2 0F 38 52
   {"vp4dpwssds",0x24  ,0x8F9800, 0x19  , 0x164B, 0x164B, 0x244B, 0     , 0x20  , 0     , 0     , 0     },    // EVEX F2 0F 38 53
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 54
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 55
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 56
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 57
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 58
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 59
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 5A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 5B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 5C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 5D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 5E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 5F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 60
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 61
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 62
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 63
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 64
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 65
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 66
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 67
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 68
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 69
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 6A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 6B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 6C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 6D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 6E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 6F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 70
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 71
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 72
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 73
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 74
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 75
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 76
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 77
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 78
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 79
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 7A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 7B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 7C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 7D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 7E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 7F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 80
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 81
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 82
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 83
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 84
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 85
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 86
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 87
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 88
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 89
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 8A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 8B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 8C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 8D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 8E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 8F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 90
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 91
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 92
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 93
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 94
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 95
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 96
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 97
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 98
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 99
   {"v4fmaddps", 0x24  ,0x8F9800, 0x19  , 0x164B, 0x164B, 0x264B, 0     , 0x20  , 0     , 0     , 0     },    // EVEX F2 0F 38 9A
   {"v4fmaddss", 0x24  ,0x8F9800, 0x19  , 0x144B, 0x144B, 0x244B, 0     , 0x20  , 0     , 0     , 0     },    // EVEX F2 0F 38 9B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 9C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 9D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 9E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 9F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 A0
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 A1
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 A2
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 A3
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 A4
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 A5
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 A6
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 A7
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 A8
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 A9
   {"v4fnmaddps",0x24  ,0x8F9800, 0x19  , 0x164B, 0x164B, 0x264B, 0     , 0x20  , 0     , 0     , 0     },    // EVEX F2 0F 38 AA
   {"v4fnmaddss",0x24  ,0x8F9800, 0x19  , 0x144B, 0x144B, 0x244B, 0     , 0x20  , 0     , 0     , 0     },    // EVEX F2 0F 38 AB
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 AC
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 AD
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F2 0F 38 AE
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // EVEX F2 0F 38 AF


// Shortcut opcode map for EVEX F3 0F 38
// Indexed by first opcode byte after EVEX prefix
SOpcodeDef OpcodeMapB3[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 00
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 01
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 02
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 03
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 04
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 05
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 06
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 07
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 08
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 09
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 0A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 0B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 0C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 0D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 0E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 0F

   {"vpmovuswb", 0x20  ,0x820400, 0x13  , 0x0F01, 0x1202, 0     , 0     , 0x2220, 0     , 0     , 0x800 },    // EVEX F3 0F 38 10
   {"vpmovusdb", 0x20  ,0x820400, 0x13  , 0x0401, 0x1203, 0     , 0     , 0x2430, 0     , 0     , 0x800 },    // EVEX F3 0F 38 11
   {"vpmovusqb", 0x20  ,0x820400, 0x13  , 0x0401, 0x1204, 0     , 0     , 0x2630, 0     , 0     , 0x800 },    // EVEX F3 0F 38 12
   {"vpmovusdw", 0x20  ,0x820400, 0x13  , 0x0F02, 0x1203, 0     , 0     , 0x2220, 0     , 0     , 0x800 },    // EVEX F3 0F 38 13
   {"vpmovusqw", 0x20  ,0x820400, 0x13  , 0x0402, 0x1204, 0     , 0     , 0x2430, 0     , 0     , 0x800 },    // EVEX F3 0F 38 14
   {"vpmovusqd", 0x20  ,0x820400, 0x13  , 0x0F03, 0x1204, 0     , 0     , 0x2220, 0     , 0     , 0x800 },    // EVEX F3 0F 38 15
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 16
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 17
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 18
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 19
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 1A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 1B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 1C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 1D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 1E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 1F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vpmovswb",  0x20  ,0x820400, 0x13  , 0x0F01, 0x1202, 0     , 0     , 0x2220, 0     , 0     , 0x800 },    // EVEX F3 0F 38 20
   {"vpmovsdb",  0x20  ,0x820400, 0x13  , 0x0401, 0x1203, 0     , 0     , 0x2430, 0     , 0     , 0x800 },    // EVEX F3 0F 38 21
   {"vpmovsqb",  0x20  ,0x820400, 0x13  , 0x0401, 0x1204, 0     , 0     , 0x2630, 0     , 0     , 0x800 },    // EVEX F3 0F 38 22
   {"vpmovsdw",  0x20  ,0x820400, 0x13  , 0x0F02, 0x1203, 0     , 0     , 0x2220, 0     , 0     , 0x800 },    // EVEX F3 0F 38 23
   {"vpmovsqw",  0x20  ,0x820400, 0x13  , 0x0402, 0x1204, 0     , 0     , 0x2430, 0     , 0     , 0x800 },    // EVEX F3 0F 38 24
   {"vpmovsqd",  0x20  ,0x820400, 0x13  , 0x0F03, 0x1204, 0     , 0     , 0x2220, 0     , 0     , 0     },    // EVEX F3 0F 38 25
   {"vptestnm",  0x20  ,0x8EC200, 0x19  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0     , 0     , 1     },    // EVEX F3 0F 38 26
   {"vptestnm",  0x20  ,0x8EB200, 0x19  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0     , 0     , 1     },    // EVEX F3 0F 38 27
   {"vpmovm2",   0x20  ,0x86C400, 0x12  , 0x1201, 0x95  , 0     , 0     , 0     , 0     , 0     , 1     },    // EVEX F3 0F 38 28
   {0,           0x12E , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // EVEX F3 0F 38 29
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 2A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 2B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 2C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 2D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 2E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 2F

   {"vpmovwb",   0x20  ,0x820400, 0x13  , 0x0F01, 0x1202, 0     , 0     , 0x2220, 0     , 0     , 0x800 },    // EVEX F3 0F 38 30
   {"vpmovdb",   0x20  ,0x820400, 0x13  , 0x0401, 0x1203, 0     , 0     , 0x2420, 0     , 0     , 0x800 },    // EVEX F3 0F 38 31
   {"vpmovqb",   0x20  ,0x820400, 0x13  , 0x0401, 0x1204, 0     , 0     , 0x2620, 0     , 0     , 0x800 },    // EVEX F3 0F 38 32
   {"vpmovdw",   0x20  ,0x820400, 0x13  , 0x0F02, 0x1203, 0     , 0     , 0x2220, 0     , 0     , 0x800 },    // EVEX F3 0F 38 33
   {"vpmovqw",   0x20  ,0x820400, 0x13  , 0x0402, 0x1204, 0     , 0     , 0x2420, 0     , 0     , 0x800 },    // EVEX F3 0F 38 34
   {"vpmovqd",   0x20  ,0x820400, 0x13  , 0x0F03, 0x1204, 0     , 0     , 0x2220, 0     , 0     , 0x800 },    // EVEX F3 0F 38 35
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 36
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 37
   {"vpmovm2",   0x20  ,0x86B400, 0x12  , 0x1201, 0x95  , 0     , 0     , 0     , 0     , 0     , 1     },    // EVEX F3 0F 38 38
   {0,           0x12F , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // EVEX F3 0F 38 39
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 3A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 3B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 3C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 3D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 3E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 3F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 40
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 41
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 42
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 43
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 44
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 45
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 46
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 47
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 48
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 49
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 4A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 4B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 4C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 4D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 4E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 4F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 50
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 51
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 52
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 53
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 54
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 55
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 56
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 57
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 58
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 59
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 5A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 5B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 5C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 5D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 5E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 5F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 60
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 61
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 62
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 63
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 64
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 65
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 66
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 67
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 68
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 69
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 6A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 6B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 6C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 6D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 6E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 6F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 70
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 71
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 72
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 73
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 74
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 75
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 76
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 77
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 78
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 79
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 7A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 7B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 7C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 7D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 7E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 7F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 80
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 81
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 82
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 83
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 84
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 85
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 86
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 87
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 88
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 89
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 8A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 8B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 8C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 8D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 8E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 8F

   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 90
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 91
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 92
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 93
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 94
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 95
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 96
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 97
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 98
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 99
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 9A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 9B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 9C
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 9D
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 9E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 9F

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 A0
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 A1
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 A2
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 A3
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 A4
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 A5
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 A6
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 A7
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 A8
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 A9
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 AA
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 AB
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 AC
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 AD
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 AE
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // EVEX F3 0F 38 AF



// Submap for vcvtfxpntpd2udq etc. Opcode byte = 0F 3A CA
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMapB4[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
{"vcvtfxpntudq2ps",0x80 ,0x420000, 0x52 , 0x164B, 0x603 , 0x31  , 0     , 0     , 0x1206, 0     , 0x100 },    //    0F 3A CA
{"vcvtfxpntps2udq",0x80 ,0x420200, 0x52 , 0x1603, 0x64B , 0x31  , 0     , 0     , 0x1204, 0     , 0x100 },    // 66 0F 3A CA
{"vcvtfxpntpd2udq",0x80 ,0x423800, 0x52 , 0x1603, 0x64C , 0x31  , 0     , 0     , 0x1205, 0     , 0x100 },    // F2 0F 3A CA
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F 3A CA

// Submap for vcvtfxpntdq2ps etc. Opcode byte = 0F 3A CB
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMapB5[] = {
{"vcvtfxpntdq2ps",0x80  ,0x420000, 0x52 , 0x164B, 0x603 , 0x31  , 0     , 0     , 0x1206, 0     , 0x100 },    //    0F 3B CB
{"vcvtfxpntps2dq",0x80  ,0x420200, 0x52 , 0x1603, 0x64B , 0x31  , 0     , 0     , 0x1204, 0     , 0x100 },    // 66 0F 3B CB
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 3B CB
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F 3B CB

// Submap for vgatherdps. Opcode byte = 0F 38 92
// Indexed by MVEX prefix
SOpcodeDef OpcodeMapB6[] = {
   {"vgatherdp", 0xCC  ,0       ,  0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // VEX 0F 38 92. link vgatherdps
   {"vgatherdp", 0xCB  ,0       ,  0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};   // EVEX/MVEX 0F 38 92. link vgatherdps, has k register as mask

// Submap for E/MVEX 0F 38 C6 vgatherpf.. Indexed by W bit
SOpcodeDef OpcodeMapB7[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0       ,    0x10D  , 0      , 0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     },    // E/MVEX 0F 38 C6. W0
   {0       ,    0x10E  , 0      , 0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0x2   , 0     }};   // E/MVEX 0F 38 C6. W1

// Submap for movdqa. Opcode byte = 66 0F 6F
// Indexed by E/MVEX prefix
SOpcodeDef OpcodeMapB8[] = {
   {"movdqa",    0x12  , 0x52100, 0x12  , 0x1250, 0x250 , 0     , 0     , 0     , 0     , 0     , 0x102 },    // 66 0F 6F
   {"vmovdqa",   0x19  ,0xC53100, 0x12  , 0x1209, 0x209 , 0     , 0     , 0x30  , 0x140A, 0     , 0x1100}};   // E/MVEX.66 0F 6F

// Submap for movdqu. Opcode byte = F3 0F 6F
// Indexed by E/MVEX prefix
SOpcodeDef OpcodeMapB9[] = {
   {"movdqu",    0x12  , 0x50400, 0x12  , 0x1250, 0x251 , 0     , 0     , 0     , 0     , 0     , 0x202 },    // F3 0F 6F
   {"vmovdqu",   0x20  ,0x853400, 0x12  , 0x1209, 0x209 , 0     , 0     , 0x30  , 0     , 0     , 0x1200}};   // F3 0F 6F

// Submap for movdqa. Opcode byte = 66 0F 7F
// Indexed by E/MVEX prefix
SOpcodeDef OpcodeMapBA[] = {
   {"movdqa",    0x12  , 0x52100, 0x13  , 0x250 , 0x1250, 0     , 0     , 0     , 0     , 0     , 0x102 },    // 66 0F 7F
   {"vmovdqa",   0x19  ,0xC53100, 0x13  , 0x203 , 0x1203, 0     , 0     , 0x30  , 0x140E, 0     , 0x1100}};   // E/MVEX.66.W0 0F 7F

// Submap for movdqu. Opcode byte = F3 0F 7F
// Indexed by MVEX.W prefix
SOpcodeDef OpcodeMapBB[] = {
   {"movdqu",    0x12  , 0x50400, 0x13  , 0x251 , 0x1250, 0     , 0     , 0     , 0     , 0     , 0x202 },    // F3 0F 7F
   {"vmovdqu",   0x20  ,0x853400, 0x13  , 0x209 , 0x1209, 0     , 0     , 0x30  , 0     , 0     ,0x1200 }};   // E/MVEX F3 0F 7F

// Submap for vmovaps. Opcode byte = 0F 29
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMapBC[] = {
   {"mova",      0x11  ,0xC52200, 0x13 , 0x24F  , 0x124F, 0     , 0     , 0x30  , 0x140C, 0     , 0x103 },    //    0F 29. movaps
   {"mova",      0x11  ,0xC52200, 0x13 , 0x24F  , 0x124F, 0     , 0     , 0x30  , 0x140C, 0     , 0x103 },    // 66 0F 29. movapd
   {0       ,    0xBD  , 0      , 0    ,  0     , 0     , 0     , 0     , 0     , 0     , 0xF   , 0     },    // F2 0F 29. link to vmovnraps
   {0       ,    0xBD  , 0      , 0    ,  0     , 0     , 0     , 0     , 0     , 0     , 0xF   , 0     }};   // F3 0F 29. link to vmovnraps

// Submap for vmovnraps. Opcode byte = F2/F3 0F 29
// Indexed by MVEX.E bit
SOpcodeDef OpcodeMapBD[] = {
   {"vmovnrap",   0x80  ,0x411C00, 0x13  ,0x224F , 0x124F, 0    , 0     , 0     , 0x180C, 0     , 0x101 },    // F2/F3 0F 29
   {"vmovnrngoap",0x80  ,0x411C00, 0x13  ,0x224F , 0x124F, 0    , 0     , 0     , 0x180C, 0     , 0x101 }};   // F2/F3 0F 29, MVEX.E

// Submap for vloadunpackld. Opcode byte = 0F 38 D0
// Indexed by prefix: none/66
SOpcodeDef OpcodeMapBE[] = {
   {"vloadunpackl",0x80, 0x423200,0x12  , 0x1609, 0x2609, 0     , 0     , 0     , 0x100A, 0     , 0x101 },    // 0F 38 D0
   {"vpackstorel" ,0x80, 0x423200,0x13  , 0x2609, 0x1609, 0     , 0     , 0     , 0x100E, 0     , 0x101 }};   // 66 0F 38 D0

// Submap for vloadunpacklps. Opcode byte = 0F 38 D1
// Indexed by prefix: none/66
SOpcodeDef OpcodeMapBF[] = {
   {"vloadunpacklp",0x80,0x421200,0x12  , 0x164F, 0x264F, 0     , 0     , 0     , 0x1008, 0     , 0x101 },    // 0F 38 D1
   {"vpackstorelp" ,0x80,0x421200,0x13  , 0x264F, 0x164F, 0     , 0     , 0     , 0x100C, 0     , 0x101 }};   // 66 0F 38 D1

// Submap for vloadunpackhd. Opcode byte = 0F 38 D4
// Indexed by prefix: none/66
SOpcodeDef OpcodeMapC0[] = {
   {"vloadunpackh",0x80, 0x423200,0x12  , 0x1609, 0x2609, 0     , 0     , 0     , 0x100A, 0     , 0x101 },    // 0F 38 D4
   {"vpackstoreh" ,0x80, 0x423200,0x13  , 0x2609, 0x1609, 0     , 0     , 0     , 0x100E, 0     , 0x101 }};   // 66 0F 38 D4

// Submap for vloadunpackhps. Opcode byte = 0F 38 D5
// Indexed by prefix: none/66
SOpcodeDef OpcodeMapC1[] = {
   {"vloadunpackhp",0x80,0x421200,0x12  , 0x164F, 0x264F, 0     , 0     , 0     , 0x1008, 0     , 0x101 },    // 0F 38 D5
   {"vpackstorehp" ,0x80,0x421200,0x13  , 0x264F, 0x164F, 0     , 0     , 0     , 0x100C, 0     , 0x101 }};   // 66 0F 38 D5

// Submap for pand. Opcode byte = 0F DB
// Indexed by E/MVEX prefix
SOpcodeDef OpcodeMapC2[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pand",      0x7   , 0xD0200, 0x19  , 0x1150, 0x1150, 0x150 , 0     , 0     , 0     , 0     , 0x2   },    // 0F DB
   {"vpand",     0x20  ,0xC93200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1406, 0     , 0x1   }};   // MVEX 0F DB

// Submap for pandn. Opcode byte = 0F DF
// Indexed by MVEX prefix
SOpcodeDef OpcodeMapC3[] = {
   {"pandn",     0x7   , 0xD0200, 0x19  , 0x1150, 0x1150, 0x150 , 0     , 0     , 0     , 0     , 0x2   },    // 0F DF
   {"vpandn",    0x20  ,0xC93200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1406, 0     , 0x1   }};   // MVEX 0F DF

// Submap for por. Opcode byte = 0F EB
// Indexed by E/MVEX prefix
SOpcodeDef OpcodeMapC4[] = {
   {"por",       0x7   , 0xD0200, 0x19  , 0x1150, 0x1150, 0x150 , 0     , 0     , 0     , 0     , 0x2   },    // 0F EB
   {"vpor",      0x20  ,0xC93200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1406, 0     , 0x1   }};   // MVEX 0F EB

// Submap for pxor. Opcode byte = 0F EF
// Indexed by MVEX prefix
SOpcodeDef OpcodeMapC5[] = {
   {"pxor",      0x7   , 0xD0200, 0x19  , 0x1150, 0x1150, 0x150 , 0     , 0     , 0     , 0     , 0x2   },    // 0F EF
   {"vpxor",     0x20  ,0xC93200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1406, 0     , 0x1   }};   // MVEX 0F EF

// Submap for vpcmpd. Opcode byte = 0F 3A 3E
// Indexed by VEX / EVEX
SOpcodeDef OpcodeMapC6[] = {
   {"kextract",  0x80  , 0x38200, 0x52  , 0x1095, 0x1004, 0x11  , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 3A 3E
   {0,           0x112 , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0x6   , 0     }};   // EVEX 0F 3A 3F. Link to vpcmp

// Submap for pcmpeqd. Opcode byte = 0F 76
// Indexed by E/MVEX prefix
SOpcodeDef OpcodeMapC7[] = {
   {"pcmpeqd",   0x7   , 0xD0200, 0x19  , 0x1103, 0x1103, 0x103 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 76
   {"vpcmpeqd",  0x20  ,0xCBA200, 0x19  , 0x95  , 0x1203, 0x203 , 0     , 0x11  , 0x1406, 0     , 0x000 }};   // E/MVEX 0F 76

// Submap for pcmpgtd. Opcode byte = 0F 66
// Indexed by E/MVEX prefix
SOpcodeDef OpcodeMapC8[] = {
   {"pcmpgtd",   0x7   , 0xD0200, 0x19  , 0x1103, 0x1103, 0x103 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 66
   {"vpcmpgtd",  0x20  ,0xCBA200, 0x19  , 0x95  , 0x1203, 0x203 , 0     , 0x11  , 0x1406, 0     , 0x000 }};   // E/MVEX 0F 66

// Opcode map for EVEX 66 0F 79. Indexed by W bit
SOpcodeDef OpcodeMapC9[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
{"vcvtps2uqq",   0x20  ,0x840200, 0x12  , 0x204 , 0xF4B , 0     , 0     , 0x27  , 0     , 0     , 0     },    // EVEX 66 0F 79. W = 0
{"vcvtpd2uqq",   0x20  ,0x841200, 0x12  , 0x204 , 0x24C , 0     , 0     , 0x27  , 0     , 0     , 0     }};   // EVEX 66 0F 79. W = 1

// Opcode map for 0F 50. Indexed by prefix
SOpcodeDef OpcodeMapCA[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movmskps",  0x11  , 0x52000, 0x12  , 0x100A, 0x124B, 0     , 0     , 0     , 0     , 0     , 2     },    // 0F 50. movmskps
   {"movmskpd",  0x11  , 0x52200, 0x12  , 0x100A, 0x124C, 0     , 0     , 0     , 0     , 0     , 2     },    // 66 0F 50. movmskpd
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

// Submap for EVEX vgatherdps. Opcode byte = 0F 38 92
// Indexed by VEX.W bit
SOpcodeDef OpcodeMapCB[] = {
   {"vgatherdps",0x20  ,0xC39200,  0x1E,  0x24F , 0x224F, 0     , 0     , 0x1090, 0x3048, 0     , 0     },    // EVEX/MVEX 0F 38 92 has k register as mask
   {"vgatherdpd",0x20  ,0xC39200,  0x1E,  0x24F , 0x2F4F, 0     , 0     , 0x1090, 0x3048, 0     , 0     }};   // EVEX/MVEX 0F 38 92 has k register as mask

// Submap for vgatherdps. Opcode byte = 0F 38 92
// Indexed by VEX.W bit
SOpcodeDef OpcodeMapCC[] = {
   {"vgatherdps",0x1C  ,0x0E9200,  0x1E,  0x24B , 0x224B, 0x24B , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 38 92
   {"vgatherdpd",0x1C  ,0x0E9200,  0x1E,  0x24C , 0x2F4C, 0x24C , 0     , 0     , 0     , 0     , 0     }};   // VEX 0F 38 92

// Submap for opcodes VEX/MVEX 0F AE
// Indexed by reg bits = 0 - 7 and mod < 3 to mod = 3 
// These codes are with VEX or MVEX prefix. Same codes without prefix are in OpcodeMap34
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
SOpcodeDef OpcodeMapCD[] = {
   {"fxsave",    0x11  , 0      , 0x11  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE /0
   {"fxrstor",   0x11  , 0      , 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0x8   },     // VEX 0F AE /1
   {"vldmxcsr",  0x11  , 0x10000, 0x11  , 0     , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE /2
   {"vstmxcsr",  0x11  , 0x10000, 0x11  , 0x2003, 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE /3
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE /4
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE /5
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE /6
   {0       ,    0xCF   , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 9     , 0     },     // VEX 0F AE /7. Link
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE reg /0
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE reg /1
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE reg /2
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE reg /3
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE reg /4
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 0F AE reg /5
   {0       ,    0xCE   , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 9     , 0     },     // VEX 0F AE reg /6. Link
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};    // VEX 0F AE reg /7

// Submap for opcodes VEX/MVEX 0F AE /6
// Indexed by prefixes 66 F2 F3
SOpcodeDef OpcodeMapCE[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX    0F AE /6
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 66 0F AE /6
   {"spflt" ,    0x80   , 0x33400, 0x11 , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX F2 0F AE /6
   {"delay" ,    0x80   , 0x33400, 0x11 , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     }};    // VEX F3 0F AE /6

// Submap for opcodes VEX/MVEX 0F AE /7
// Indexed by prefixes 66 F2 F3
SOpcodeDef OpcodeMapCF[] = {
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX    0F AE /7
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // VEX 66 0F AE /7
   {"clevict0",  0x80   ,0x430800, 0x11 , 0x2006, 0     , 0     , 0     , 0     , 2     , 0     , 0     },     // VEX F2 0F AE /7
   {"clevict1",  0x80   ,0x430400, 0x11 , 0x2006, 0     , 0     , 0     , 0     , 2     , 0     , 0     }};    // VEX F3 0F AE /7

// Submap for opcodes 0F 38 F6
// Indexed by prefixes 66 F2 F3
SOpcodeDef OpcodeMapD0[] = {
   {"wrss"  ,    0      , 0x1000 , 0x13 , 0x2009, 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },     //    0F 38 F6
   {"adcx"  ,    0x1D   , 0x1200 , 0x12 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },     // 66 0F 38 F6
   {"mulx"  ,    0x1D   , 0xB1000, 0x19 , 0x1009, 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     },     // F2 0F 38 F6
   {"adox"  ,    0x1D   , 0x1400 , 0x12 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     }};    // F3 0F 38 F6

SOpcodeDef OpcodeMapD1[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"prefetch",  0x1001 ,  0     , 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0D /0 AMD only. Works as NOP on Intel
   {"prefetchw", 0x1D   ,  0     , 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0D /1
   {"prefetchwt1",0x22  ,  0     , 0x11  , 0     , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 0D /2
   {0       ,    0      ,  0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   //

// Tertiary opcode map for movnt. Opcode byte = 0F 2B
// Indexed by prefix = none, 66, F2, F3
SOpcodeDef OpcodeMapD2[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"movntps",   0x11  ,0x852000, 0x13  , 0x224B, 0x124B, 0     , 0     , 0x00  , 0     , 0     , 0x102 },    // 0F 2B. movntps
   {"movntpd",   0x12  ,0x852200, 0x13  , 0x224C, 0x124C, 0     , 0     , 0x00  , 0     , 0     , 0x102 },    // 66 0F 2B. movntpd
   {"movntsd",   0x1004, 0x800  , 0x13  , 0x204C, 0x104C, 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 2B. movntsd (AMD only)
   {"movntss",   0x1004, 0x400  , 0x13  , 0x204B, 0x104B, 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F 2B. movntss (AMD only)

// opcode map for bsr and lzcnt. Opcode byte = 0F BD
// Indexed by prefix = none, 66, F2, F3
SOpcodeDef OpcodeMapD3[4] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"bsr",       0x3   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F BD. bsr
   {"bsr",       0x3   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F BD. not allowed
   {"bsr",       0x3   , 0x1100 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F BD. not allowed
   {"lzcnt",     0x1D  ,0x11500 , 0x12  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F BD. AMD SSE4a, Intel LZCNT

// Opcode map for blcfill etc. Opcode byte = XOP(9) 01, indexed by reg bits
SOpcodeDef OpcodeMapD4[] = {
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 01 /0
   {"blcfill",   0x1007, 0x11000, 0x18  , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 01 /1
   {"blsfill",   0x1007 , 0x11000, 0x18 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 01 /2
   {"blcs"  ,    0x1007 , 0x11000, 0x18 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 01 /3
   {"tzmsk" ,    0x1007 , 0x11000, 0x18 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 01 /4
   {"blcic" ,    0x1007 , 0x11000, 0x18 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 01 /5
   {"blsic" ,    0x1007 , 0x11000, 0x18 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 01 /6
   {"t1mskc",    0x1007 , 0x11000, 0x18 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     }};   // XOP(9) 01 /7

// Opcode map for blcmsk etc. Opcode byte = XOP(9) 02, indexed by reg bits
SOpcodeDef OpcodeMapD5[] = {
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 02 /0
   {"blcmsk",    0x1007 , 0x11000, 0x18 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 02 /1
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 02 /2
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 02 /3
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 02 /4
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 02 /5
   {"blci"  ,    0x1007 , 0x11000, 0x18 , 0x1009, 0x9   , 0     , 0     , 0     , 0     , 0     , 0     },    // XOP(9) 02 /6
   {0       ,    0      , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // XOP(9) 02 /7

// Opcode map for EVEX 0F 79. Indexed by 66,F2,F3 prefix
SOpcodeDef OpcodeMapD6[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0xD9  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // EVEX 0F 79. Link to vcvtps/pd2udq
   {0,           0xC9  , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // EVEX 66 0F 79. Link to 
   {"vcvtsd2usi",0x20  ,0x803800, 0x12  , 0x1009, 0x4C  , 0     , 0     , 0x6   , 0     , 0     , 0     },    // EVEX F2 0F 79
   {"vcvtss2usi",0x20  ,0x803800, 0x12  , 0x1009, 0x4B  , 0     , 0     , 0x6   , 0     , 0     , 0     }};   // EVEX F3 0F 79

// Opcode map for 0F 38 A0. Indexed by VEX.W bit
SOpcodeDef OpcodeMapD7[] = {
   {"vpscatterdd",0x20 , 0xC3B200,0x1E  , 0x2209, 0x1209, 0     , 0     , 0x1090, 0x304E, 0     , 0x000 },    // W0 0F 38 A0
   {"vpscatterdq",0x20 , 0xC3B200,0x1E  , 0x2F09, 0x1209, 0     , 0     , 0x1090, 0x304E, 0     , 0x000 }};   // W1 0F 38 A0

// Opcode map for 0F 38 A1. Indexed by VEX.W bit
SOpcodeDef OpcodeMapD8[] = {
   {"vpscatterqd",0x20 , 0xC3B200,0x1E  , 0x2209, 0x1F09, 0     , 0     , 0x1090, 0x304E, 0     , 0x000 },    // W0 0F 38 A0
   {"vpscatterqq",0x20 , 0xC3B200,0x1E  , 0x2209, 0x1209, 0     , 0     , 0x1090, 0x304E, 0     , 0x000 }};   // W1 0F 38 A0

// Opcode map for EVEX 0F 79, pp0. Indexed by W bit
SOpcodeDef OpcodeMapD9[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
{"vcvtps2udq",   0x20  ,0x840000, 0x12  , 0x203 , 0x24B , 0     , 0     , 0x37  , 0     , 0     , 0     },    // EVEX 0F 79. W = 0
{"vcvtpd2udq",   0x20  ,0x841000, 0x12  , 0xF03 , 0x24C , 0     , 0     , 0x37  , 0     , 0     , 0     }};   // EVEX 0F 79. W = 1

// Opcode map for EVEX 0F 78. Indexed by 66,F2,F3 prefix
SOpcodeDef OpcodeMapDA[] = {
   {0,           0xDB  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // EVEX 0F 78. Link to vcvttpd2udq
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 66 EVEX 0F 78
   {"vcvttsd2usi",0x20 ,0x803800, 0x12  , 0x1009, 0x4C  , 0     , 0     , 0x2   , 0     , 0     , 0     },    // F2 EVEX 0F 78
   {"vcvttss2usi",0x20 ,0x803400, 0x12  , 0x1009, 0x4B  , 0     , 0     , 0x2   , 0     , 0     , 0     }};   // F3 EVEX 0F 78

// Opcode map for EVEX 0F 78. Indexed by W bit
SOpcodeDef OpcodeMapDB[] = {
   {"vcvttps2udq",0x20 ,0x841000, 0x12  , 0x1203, 0x24B , 0     , 0     , 0x37  , 0     , 0     , 0     },    // VEX 0F 78
   {"vcvttpd2udq",0x20 ,0x841000, 0x12  , 0x1F03, 0x24C , 0     , 0     , 0x37  , 0     , 0     , 0     }};   // VEX 0F 78

// Opcode map for EVEX 0F 7A. Indexed by 66,F2,F3 prefix
SOpcodeDef OpcodeMapDC[] = {
   {0           , 0    , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX 0F 7A
   {0           , 0    , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 66 EVEX 0F 7A
   {0           ,0x11B , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // F2 EVEX 0F 7A. Link to vcvtudq2ps
   {0           ,0x11C , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};   // F3 E/MVEX 0F 7A. Link to vcvtudq2pd

// Opcode map for EVEX 0F 7B. Indexed by 66,F2,F3 prefix
SOpcodeDef OpcodeMapDD[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    //    EVEX 0F 7B
   {0,           0x11A , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 66 EVEX 0F 7B. Link to vcvtps/pd2qq
   {"vcvtusi2sd",0x20  ,0x8C3800, 0x19  , 0x104C, 0x104C, 9     , 0     , 0x06  , 0     , 0     , 0     },    // F2 EVEX 0F 7B
   {"vcvtusi2ss",0x20  ,0x8C3400, 0x19  , 0x104B, 0x104B, 9     , 0     , 0x06  , 0     , 0     , 0     }};   // F3 EVEX 0F 7B

// Opcode map for 0F 3A 1B. Indexed by W bit
SOpcodeDef OpcodeMapDE[] = {
   {"vextractf32x8",0x20,0x801200,0x53  , 0x54B , 0x124B, 0x31  , 0     , 0x30  , 0     , 0     , 0     },    // 0F 3A 1B. W0
   {"vextractf64x4",0x20,0x801200,0x53  , 0x54B , 0x124B, 0x31  , 0     , 0x30  , 0     , 0     , 0     }};   // 0F 3A 1B. W1

// Opcode map for 0F 3A 3B. Indexed by W bit
SOpcodeDef OpcodeMapDF[] = {
   {"vextracti32x8",0x20,0x800200, 0x53 , 0x504 , 0x1204, 0x31  , 0     , 0x20  , 0     , 0     , 0     },    // 0F 3A 3B
   {"vextracti64x4",0x20,0x801200, 0x53 , 0x504 , 0x1204, 0x31  , 0     , 0x20  , 0     , 0     , 0     }};   // 0F 3A 3B

// Opcode map for 0F 38 93. Indexed by EVEX present
SOpcodeDef OpcodeMapE0[] = {
   {"vgatherqp", 0x95  , 0      , 0   ,   0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 0F 38 93. Link to vpgatherqps/pd
   {"vgatherqp", 0xE1  , 0      , 0    ,  0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};   // EVEX/MVEX 0F 38 92 has k register as mask. Link by vector size

// Opcode map for 0F 38 93. Indexed by W bit
SOpcodeDef OpcodeMapE1[] = {
   {"vgatherqps",0x20  ,0xC39200, 0x1E ,  0xF4F , 0x224F, 0     , 0     , 0x1090, 0x3048, 0     , 0     },    // EVEX/MVEX 0F 38 92. W0
   {"vgatherqpd",0x20  ,0xC39200, 0x1E ,  0x24F , 0x224F, 0     , 0     , 0x1090, 0x3048, 0     , 0     }};   // EVEX/MVEX 0F 38 92. W1

// map for movd/movq. Opcode byte = 0F 7E
// Indexed by prefix: none/66/F2/F3
SOpcodeDef OpcodeMapE2[] = {
   {"vmov",      0x7   ,0x813200, 0x13  , 0x9   , 0x1409, 0     , 0     , 0x00  , 0     , 0     , 0x1   },    //    VEX 0F 7E
   {"vmov",      0x7   ,0x813200, 0x13  , 0x9   , 0x1409, 0     , 0     , 0x00  , 0     , 0     , 0x1   },    // 66 VEX 0F 7E
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 VEX 0F 7E
   {0,           0x5B  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0x3   , 0     }};   // F3 0F 7E. Link to map 5B. movq xmm,xmm/m64

// map for 0F 38 29
// Indexed by EVEX
SOpcodeDef OpcodeMapE3[] = {
   {"pcmpeqq",   0x16  , 0xD8200, 0x19  , 0x1204, 0x1204, 0x204 , 0     , 0     , 0     , 0     , 0x2   },    //      0F 38 29
   {"vpcmpeqq",  0x20  ,0x8FB200, 0x19  , 0x95  , 0x1204, 0x204 , 0     , 0x11  , 0     , 0     , 0     }};   // EVEX 0F 38 29

// map for 0F 38 37
// Indexed by EVEX
SOpcodeDef OpcodeMapE4[] = {
   {"pcmpgtq",   0x16  , 0xD8200, 0x19  , 0x1204, 0x1204, 0x204 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 38 37
   {"vpcmpgtq",  0x20  ,0x8FB200, 0x19  , 0x95  , 0x1204, 0x204 , 0     , 0x11  , 0     , 0     , 0     }};   // EVEX 0F 38 37

// Submap for 0F 38 1A, indexed by VEX.W bit
SOpcodeDef OpcodeMapE5[] = {
   {0       ,    0xF8   , 0      , 0    , 0     , 0     , 0     , 0     , 0     , 0     , 0xE   , 0     },    // VEX 0F 38 1A /W0, link to vbroadcastf128 vbroadcastf32x4
   {"vbroadcastf64x2",0x20,0xC6B200,0x12, 0x124C, 0x244C, 0     , 0     , 0x20  , 0x1010, 0     , 0x100 }};   // 0F 38 1A, 512 bits

// Map for 0F 38 39. Indexed by EVEX present
SOpcodeDef OpcodeMapE6[] = {
   {"pminsd",    0x15  ,0x4D8200, 0x19  , 0x1203, 0x1203, 0x203 , 0     , 0     , 0x1406, 0     , 0x2   },    //      0F 38 39
   {0,           0x12D , 0      , 0x19  , 0     , 0     , 0     , 0     , 0     , 0     , 0x9   , 0     }};   // EVEX 0F 38 39

// Map for 0F 38 3B. Indexed by EVEX present
SOpcodeDef OpcodeMapE7[] = {
   {"pminud",    0x15  ,0x4D8200, 0x19  , 0x1203, 0x1203, 0x203 , 0     , 0     , 0x1406, 0     , 0x2   },    // 0F 38 3B
   {"vpminu",    0x15  ,0xCDB200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1406, 0     , 0x1   }};   // 0F 38 3B

// Map for 0F 38 3D. Indexed by EVEX present
SOpcodeDef OpcodeMapE8[] = {
   {"pmaxsd",    0x15  ,0x4D8200, 0x19  , 0x1203, 0x1203, 0x203 , 0     , 0     , 0x1406, 0     , 0x2   },    // 0F 38 3D
   {"vpmaxs",    0x15  ,0xCDB200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1406, 0     , 0x1   }};   // 0F 38 3D

// Map for 0F 38 3F. Indexed by EVEX present
SOpcodeDef OpcodeMapE9[] = {
   {"pmaxud",    0x15  ,0x4D8200, 0x19  , 0x1203, 0x1203, 0x203 , 0     , 0     , 0x1406, 0     , 0x2   },    // 0F 38 3F
   {"vpmaxu",    0x15  ,0xCDB200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1406, 0     , 0x1   }};   // 0F 38 3F

// Map for 0F 38 10. Indexed by VEX prefix type
SOpcodeDef OpcodeMapEA[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pblendvb",  0x15  , 0x8200 , 0x12  , 0x1401, 0x401 , 0xAE  , 0     , 0     , 0     , 0     , 0     },    // 0F 38 10
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX 0F 38 10
   {"vpsrlvw",   0x20  ,0x8FC200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x20  , 0     , 0     , 0     },    // EVEX 0F 38 10
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // MVEX 0F 38 10

// Map for VEX 0F 90. Indexed by memory/register
SOpcodeDef OpcodeMapEB[] = {
   {"kmov",      0x20  , 0x35200, 0x12  , 0x1095, 0x2009, 0     , 0     , 0     , 0     , 0     , 1     },    // VEX 0F 90. Name without w in KNC syntax, but code identical
   {"kmov",      0x20  , 0x35200, 0x12  , 0x1095, 0x95  , 0     , 0     , 0     , 0     , 0     , 1     }};   // VEX 0F 90. Name without w in KNC syntax, but code identical

// Map for VEX 0F 92. indexed by prefix 0 66 F2 F3
// The coding with F2 is different from other k instructions. Allow coding with 66 instead in case this is an error in the manual
SOpcodeDef OpcodeMapEC[] = {
   {"kmov",      0x20  , 0x35200, 0x12  , 0x95  , 0x1003, 0     , 0     , 0     , 0     , 0     , 1     },    // VEX 0F 92. Name without w in KNC syntax, but code identical
   {"kmov",      0x20  , 0x35200, 0x12  , 0x95  , 0x1003, 0     , 0     , 0     , 0     , 0     , 1     },    // 66 VEX 0F 92
   {0,           0xED  , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};   // F2 VEX 0F 92

// Map for VEX 0F 92. indexed by VEX.W bit
SOpcodeDef OpcodeMapED[] = {
   {"kmovd",    0x20  , 0x35200, 0x12  , 0x95  , 0x1003, 0     , 0     , 0     , 0     , 0     , 0     },    // F2 VEX 0F 92. W0
   {"kmovq",    0x20  , 0x35200, 0x12  , 0x95  , 0x1004, 0     , 0     , 0     , 0     , 0     , 0     }};   // F2 VEX 0F 92. W1

// Map for VEX 0F 93. indexed by prefix 0 66 F2 F3
// The coding with F2 is different from other k instructions. Allow coding with 66 instead in case this is an error in the manual
SOpcodeDef OpcodeMapEE[] = {
   {"kmov",      0x20  , 0x35200, 0x12  , 0x1003, 0x1095, 0     , 0     , 0     , 0     , 0     , 1     },    // VEX 0F 93. Name without w in KNC syntax, but code identical
   {"kmov",      0x20  , 0x35200, 0x12  , 0x1003, 0x1095, 0     , 0     , 0     , 0     , 0     , 1     },    // 66 VEX 0F 93
   {0,           0xEF  , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};   // F2 VEX 0F 93

// Map for VEX 0F 93. indexed by VEX.W bit
SOpcodeDef OpcodeMapEF[] = {
   {"kmovd",     0x20  , 0x35200, 0x12  , 0x1003, 0x1095, 0     , 0     , 0     , 0     , 0     , 0     },    // F2 VEX 0F 93 W0
   {"kmovq",     0x20  , 0x35200, 0x12  , 0x1004, 0x1095, 0     , 0     , 0     , 0     , 0     , 0     }};   // F2 VEX 0F 93 W1

// Map for VEX 0F 4B. indexed by prefix 0 66 F2 F3
SOpcodeDef OpcodeMapF0[] = {
   {0,           0xF1  , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }, 
   {"kunpckbw",  0x20  ,0x1E3200, 0x19  , 0x1095, 0x1095, 0x1095, 0     , 0     , 0     , 0     , 0     },    // 66 VEX 0F 4B
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }}; 

// Map for VEX 0F 4B. indexed by VEX.W bit
SOpcodeDef OpcodeMapF1[] = {
   {"kunpckwd",  0x20  ,0x1E3200, 0x19  , 0x1095, 0x1095, 0x1095, 0     , 0     , 0     , 0     , 0     },    // VEX 0F 4B
   {"kunpckdq",  0x20  ,0x1E3200, 0x19  , 0x1095, 0x1095, 0x1095, 0     , 0     , 0     , 0     , 0     }};

// Map for 0F AE /7. Indexed by 66 prefix
SOpcodeDef OpcodeMapF2[] = {
   {"clflush",   0x12  , 0      , 0x11  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // 0F AE /7
   {"clflushopt",0x22  , 0x200  , 0x11  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     , 0     }};    // 66 0F AE /7

// Map for 0F AE /6. Indexed by 66 prefix
SOpcodeDef OpcodeMapF3[] = {
   {"xsaveopt",  0x19  , 0x2000 , 0x11  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // 0F AE /6
   {"clwb    ",  0x22  , 0x200  , 0x11  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // 66 0F AE /6
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // F2 0F AE /6
   {"clrssbsy",  0     , 0x400  , 0x11  , 0     , 0x2004, 0     , 0     , 0     , 0     , 0     , 0     }};    // F3 0F AE /6

// Map for 0F AE reg /7. Indexed by 66 prefix
SOpcodeDef OpcodeMapF4[] = {
   {"sfence",    0x12  , 0      , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },     // m-7
   {"pcommit",   0x22  , 0x200  , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

// Opcode map for floating point cmpps/pd instructions. First two bytes = 0F C2
// Indexed by VEX prefix type
SOpcodeDef OpcodeMapF5[] = {
   {0,           0xF6  , 0      , 0x52  , 0     , 0     , 0     , 0     , 0     , 0     , 0x6   , 0     },    //      0F C2. Link to cmpps etc.
   {0,           0xF6  , 0      , 0x52  , 0     , 0     , 0     , 0     , 0     , 0     , 0x6   , 0     },    // VEX  0F C2. Link to cmpps etc.
   {0,           0xF7  , 0      , 0x52  , 0     , 0     , 0     , 0     , 0     , 0     , 0x6   , 0     },    // EVEX 0F C2. Link to cmpps etc.
   {0,           0xF7  , 0      , 0x52  , 0     , 0     , 0     , 0     , 0     , 0     , 0x6   , 0     }};   // MVEX 0F C2. Link to cmpps etc.

SOpcodeDef OpcodeMapF6[] = {
// Opcode map for floating point cmpps/pd instructions. First two bytes = 0F C2
// Indexed by immediate byte following operands = 0 - 7
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cmpeq",     0x12  ,0xCD2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 00: cmpeqps/pd
   {"cmplt",     0x12  ,0xCD2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 01: 
   {"cmple",     0x12  ,0xCD2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 02: 
   {"cmpunord",  0x12  ,0xCD2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 03: 
   {"cmpneq",    0x12  ,0xCD2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 04: 
   {"cmpnlt",    0x12  ,0xCD2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 05: 
   {"cmpnle",    0x12  ,0xCD2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 06: 
   {"cmpord",    0x12  ,0xCD2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 07: 
// imm > 7 only with VEX
   {"vcmpeq_uq", 0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 08: 
   {"vcmpnge_us",0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 09: 
   {"vcmpngt_us",0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0A: 
   {"vcmpfalse_oq",0x19,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0B: 
   {"vcmpneq_oq",0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0C: 
   {"vcmpge_os", 0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0D: 
   {"vcmpgt_os", 0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0E: 
   {"vcmptrue_uq",0x19 ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0F: 
   {"vcmpeq_os", 0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 10: 
   {"vcmplt_oq", 0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 11: 
   {"vcmple_oq", 0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 12: 
   {"vcmpunord_s",0x19 ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 13: 
   {"vcmpneq_us",0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 14: 
   {"vcmpnlt_uq",0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 15: 
   {"vcmpnle_uq",0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 16: 
   {"vcmpord_s", 0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 17: 
   {"vcmpeq_us", 0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 18: 
   {"vcmpnge_uq",0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 19: 
   {"vcmpngt_uq",0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1A: 
   {"vcmpfalse_os",0x19,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1B: 
   {"vcmpneq_os",0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1C: 
   {"vcmpge_oq", 0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1D: 
   {"vcmpgt_oq", 0x19  ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1E: 
   {"vcmptrue_us",0x19 ,0x8E2E00, 0x59  , 0x124F, 0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1F: 
   {"vcmp",      0x19  ,0x8E2200, 0x4059, 0x124F, 0x24F , 0x24F , 0x31  , 0x13  , 0     , 0     , 0x3   }};   // 0F C2 op > 1F: cmpps/pd, imm


SOpcodeDef OpcodeMapF7[] = {
// Opcode map for floating point cmpps/pd instructions. EVEX 0F C2
// Indexed by immediate byte following operands = 0 - 7
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"cmpeq",     0x12  ,0xCD2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 00: cmpeqps/pd
   {"cmplt",     0x12  ,0xCD2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 01: 
   {"cmple",     0x12  ,0xCD2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 02: 
   {"cmpunord",  0x12  ,0xCD2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 03: 
   {"cmpneq",    0x12  ,0xCD2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 04: 
   {"cmpnlt",    0x12  ,0xCD2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 05: 
   {"cmpnle",    0x12  ,0xCD2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 06: 
   {"cmpord",    0x12  ,0xCD2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0x1204, 0     , 0x3   },    // 0F C2 op 07: 
// imm > 7 only with EVEX prefix, not with MVEX
   {"vcmpeq_uq", 0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 08: 
   {"vcmpnge_us",0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 09: 
   {"vcmpngt_us",0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0A: 
   {"vcmpfalse_oq",0x19,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0B: 
   {"vcmpneq_oq",0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0C: 
   {"vcmpge_os", 0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0D: 
   {"vcmpgt_os", 0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0E: 
   {"vcmptrue_uq",0x19 ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 0F: 
   {"vcmpeq_os", 0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 10: 
   {"vcmplt_oq", 0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 11: 
   {"vcmple_oq", 0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 12: 
   {"vcmpunord_s",0x19 ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 13: 
   {"vcmpneq_us",0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 14: 
   {"vcmpnlt_uq",0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 15: 
   {"vcmpnle_uq",0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 16: 
   {"vcmpord_s", 0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 17: 
   {"vcmpeq_us", 0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 18: 
   {"vcmpnge_uq",0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 19: 
   {"vcmpngt_uq",0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1A: 
   {"vcmpfalse_os",0x19,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1B: 
   {"vcmpneq_os",0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1C: 
   {"vcmpge_oq", 0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1D: 
   {"vcmpgt_oq", 0x19  ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1E: 
   {"vcmptrue_us",0x19 ,0x8E2E00, 0x59  , 0x95,   0x24F , 0x24F , 0     , 0x13  , 0     , 0     , 0x1   },    // 0F C2 op 1F: 
   {"vcmp",      0x19  ,0x8E2200, 0x4059, 0x95,   0x24F , 0x24F , 0x31  , 0x13  , 0     , 0     , 0x3   }};   // 0F C2 op > 1F: cmpps/pd, imm

// Submap for 0F 38 1A / W0, indexed by EVEX
SOpcodeDef OpcodeMapF8[] = {
   {"vbroadcastf128" ,0x19,0x878200,0x12, 0x154B, 0x244B, 0     , 0     , 0x20  , 0     , 0     , 0     },    // VEX  0F 38 1A
   {"vbroadcastf32x4",0x10,0xC6B200,0x12, 0x124B, 0x244B, 0     , 0     , 0x20  , 0x1010, 0     , 0x100 }};   // EVEX 0F 38 1A

// Map for 0F 3A 08. Indexed by EVEX present
SOpcodeDef OpcodeMapF9[] = {
   {"roundps",   0x15  , 0x58200, 0x52  , 0x124B, 0x24B , 0x31  , 0     , 0     , 0     , 0     , 0x2   },    // 0F 3A 08. Also in AMD instruction set   
   {"vrndscaleps",0x20 ,0x858200, 0x52  , 0x124B, 0x24B , 0x31  , 0     , 0x33  , 0     , 0     , 0     }};   // EVEX 0F 3A 08

// Map for 0F 3A 09. Indexed by EVEX present
SOpcodeDef OpcodeMapFA[] = {
   {"roundpd",   0x15  ,0x858200, 0x52  , 0x124C, 0x24C , 0x31  , 0     , 0     , 0     , 0     , 0x2   },    // 0F 3A 09. Also in AMD instruction set
   {"vrndscalepd",0x20 ,0x85A200, 0x52  , 0x124C, 0x24C , 0x31  , 0     , 0x33  , 0     , 0     , 0     }};   // EVEX 0F 3A 09

// Map for 0F 3A 0A. Indexed by EVEX present
SOpcodeDef OpcodeMapFB[] = {
   {"roundss",   0x15  , 0x98200, 0x59  , 0x104B, 0x104B, 0x4B  , 0x31  , 0     , 0     , 0     , 0x2   },    // 0F 3A 0A. Also in AMD instruction set
   {"vrndscaless",0x20 ,0x8DB200, 0x59  , 0x104B, 0x004B, 0x4B  , 0x31  , 0x32  , 0     , 0     , 0     }};   // EVEX 0F 3A 08

// Map for 0F 3A 0B. Indexed by EVEX present
SOpcodeDef OpcodeMapFC[] = {
   {"roundsd",   0x15  , 0x98200, 0x59  , 0x104C, 0x104C, 0x4C  , 0x31  , 0     , 0     , 0     , 0x2   },    // 0F 3A 0B. Also in AMD instruction set
   {"vrndscalesd",0x20 ,0x8DB200, 0x59  , 0x104C, 0x004C, 0x4C  , 0x31  , 0x32  , 0     , 0     , 0     }};   // EVEX 0F 3A 08

// Map for 0F 38 2C. Indexed by EVEX present
SOpcodeDef OpcodeMapFD[] = {
   {"vmaskmovps",0x19  , 0xF8200, 0x19,   0x124B, 0x124B, 0x224B, 0     , 0     , 0     , 0     , 0     },    // 0F 38 2C
   {"vscalefp"  ,0x20  ,0x899200, 0x19,   0x124F, 0x124F, 0x024F, 0     , 0x37  , 0     , 0     , 0x1   }};   // EVEX 0F 38 2C

// Map for 0F 38 2D. Indexed by EVEX present
SOpcodeDef OpcodeMapFE[] = {
   {"vmaskmovpd",0x19  , 0xF8200, 0x19,   0x124C, 0x124C, 0x224C, 0     , 0     , 0     , 0     , 0     },    // 0F 38 2D
   {"vscalefs"  ,0x20  ,0x899200, 0x19,   0x144F, 0x144F, 0x044F, 0     , 0x36  , 0     , 0     , 0x1   }};   // EVEX 0F 38 2D

// Map for 0F 38 3A. Indexed by 66 F2 F3 prefixes
SOpcodeDef OpcodeMapFF[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F 38 3A
   {"pminuw",    0x15  ,0x8D8200, 0x19  , 0x1202, 0x1202, 0x202 , 0     , 0x20  , 0     , 0     , 0x2   },    // 66 0F 38 3A
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 38 3A
   {"vpbroadcastmw2d",0x20,0x860400,0x12, 0x1203, 0x1095, 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F 38 2A

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
// Map for 0F 38 A". Indexed by W bit
SOpcodeDef OpcodeMap100[] = {
   {"vscatterdps",0x20 ,0xC39200, 0x1E  , 0x224B, 0x1209, 0     , 0     , 0x1090, 0x304C, 0     , 0x000 },    // 0F 38 A2. W0
   {"vscatterdpd",0x20 ,0xC39200, 0x1E  , 0x2F4C, 0x1209, 0     , 0     , 0x1090, 0x304C, 0     , 0x000 }};   // 0F 38 A2. W1

// Map for 0F 38 A3. Indexed by W bit
SOpcodeDef OpcodeMap101[] = {
   {"vscatterqps",0x20 ,0xC39200, 0x1E  , 0x224B, 0x1F09, 0     , 0     , 0x1090, 0x304C, 0     , 0x000 },    // 0F 38 A3. W0
   {"vscatterqpd",0x20 ,0xC39200, 0x1E  , 0x224C, 0x1209, 0     , 0     , 0x1090, 0x304C, 0     , 0x000 }};   // 0F 38 A3. W1


// Submap for vpgatherd. Opcode byte = 0F 38 90
// Indexed by VEX/EVEX prefix
SOpcodeDef OpcodeMap102[] = {
   {0,           0x103 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     },    // 
   {0,           0x104 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};   // 

// Submap for vpgatherd. Opcode byte = 0F 38 90
// Indexed by VEX.W bit
SOpcodeDef OpcodeMap103[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vpgatherdd",0x1C  ,0xCEB200, 0x1E,   0x203 , 0x2203, 0x203 , 0     , 0     , 0     , 0     , 0     },    // 0F 38 90
   {"vpgatherdq",0x1C  ,0xCEB200, 0x1E,   0x204 , 0x2F04, 0x204 , 0     , 0     , 0x100A, 0     , 0     }};   // 0F 38 90

// Submap for vpgatherd. Opcode byte = 0F 38 90
// Indexed by EVEX.W bit
SOpcodeDef OpcodeMap104[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vpgatherdd",0x1C  ,0xCEB200, 0x1E,   0x203 , 0x2203, 0     , 0     , 0x1090, 0x100A, 0     , 0     },    // EVEX 0F 38 90
   {"vpgatherdq",0x1C  ,0xCEB200, 0x1E,   0x204 , 0x2F04, 0     , 0     , 0x1090, 0x100A, 0     , 0     }};   // EVEX 0F 38 90

// Submap for vpgatherq, Opcode 0F 38 91, Indexed by VEX.W bit
SOpcodeDef OpcodeMap105[] = {
   {"vpgatherqd",0x1C  ,0x8EB200, 0x1E,   0xF03 , 0x2203, 0xF03 , 0     , 0     , 0     , 0     , 0     },    // 0F 38 91, W0
   {"vpgatherqq",0x1C  ,0x8EB200, 0x1E,   0x204 , 0x2204, 0x204 , 0     , 0     , 0     , 0     , 0     }};   // 0F 38 91, W1

// Submap for vpgatherq, Opcode 0F 38 91, Indexed by EVEX.W bit
SOpcodeDef OpcodeMap106[] = {
   {"vpgatherqd",0x1C  ,0x8EB200, 0x1E,   0xF03 , 0x2203, 0     , 0     , 0x1090, 0     , 0     , 0     },    // EVEX 0F 38 91, W0
   {"vpgatherqq",0x1C  ,0x8EB200, 0x1E,   0x204 , 0x2204, 0     , 0     , 0x1090, 0     , 0     , 0     }};   // EVEX 0F 38 91, W1

// Map for 0F 38 C8. Indexed by VEX prefix type
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
SOpcodeDef OpcodeMap107[] = {
   {"sha1nexte", 0x22  , 0      , 0x12  , 0x1203, 0x0203, 0     , 0     , 0     , 0     , 0     , 0     },    // no VEX
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX
   {"vexp2p",    0x21  ,0x809200, 0x12  , 0x124F, 0x024F, 0     , 0     , 0x33  , 0     , 0     , 0x1   },    // EVEX 0F 38 C8
   {"vexp223ps", 0x80  ,0x428200, 0x12  , 0x164B, 0x603 , 0     , 0     , 0     , 0x1201, 0     , 0x100 }};   // MVEX 0F 38 C8

// Map for 0F 38 C9. Indexed by VEX prefix type
SOpcodeDef OpcodeMap108[] = {
   {"sha1msg1",  0x22  , 0      , 0x12  , 0x1203, 0x0203, 0     , 0     , 0     , 0     , 0     , 0     },    // no VEX
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX 0F 38 C9
   {"vlog2ps",   0x80  ,0x428200, 0x12  , 0x164B, 0x603 , 0     , 0     , 0     , 0x1201, 0     , 0x100 }};   // MVEX 0F 38 C9

// Map for 0F 38 CA. Indexed by VEX prefix type
SOpcodeDef OpcodeMap109[] = {
   {"sha1msg2",  0x22  , 0      , 0x12  , 0x1203, 0x0203, 0     , 0     , 0     , 0     , 0     , 0     },    // no VEX
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX
   {"vrcp28p",   0x21  ,0x809200, 0x12  , 0x124F, 0x024F, 0     , 0     , 0x33  , 0     , 0     , 0x1   },    // EVEX 0F 38 CA
   {"vrcp23ps",  0x80  ,0x428200, 0x12  , 0x164B, 0x603 , 0     , 0     , 0     , 0x1201, 0     , 0x100 }};   // MVEX 0F 38 CA

// Map for 0F 38 CB. Indexed by VEX prefix type
SOpcodeDef OpcodeMap10A[] = {
   {"sha256rnds2",0x22 , 0      , 0x12  , 0x1203, 0x0203, 0xAE  , 0     , 0     , 0     , 0     , 0     },    // no VEX
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX
   {"vrcp28s",   0x21  ,0x809200, 0x12  , 0x104F, 0x004F, 0     , 0     , 0x32  , 0     , 0     , 0x1   },    // EVEX 0F 38 CB
   {"vrsqrt23ps",0x80  ,0x428200, 0x12  , 0x164B, 0x603 , 0     , 0     , 0     , 0x1201, 0     , 0x100 }};   // MVEX 0F 38 CB

// Map for 0F 38 CC. Indexed by VEX prefix type
SOpcodeDef OpcodeMap10B[] = {
   {"sha256msg1",0x22  , 0      , 0x12  , 0x1203, 0x0203, 0     , 0     , 0     , 0     , 0     , 0     },    // no VEX
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX
   {"vrsqrt28p", 0x21  ,0x809200, 0x12  , 0x124F, 0x024F, 0     , 0     , 0x33  , 0     , 0     , 0x1   },    // EVEX 0F 38 CC
   {"vaddsetsps",0x80  ,0x4A8200, 0x19,   0x164B, 0x164B, 0x64B , 0     , 0     , 0x3304, 0     , 0x100 }};   // MVEX 0F 38 CC

// Map for 0F 38 CD. Indexed by VEX prefix type
SOpcodeDef OpcodeMap10C[] = {
   {"sha256msg2",0x22  , 0      , 0x12  , 0x1203, 0x0203, 0     , 0     , 0     , 0     , 0     , 0     },    // no VEX
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX
   {"vrsqrt28s", 0x21  ,0x809200, 0x12  , 0x104F, 0x004F, 0     , 0     , 0x32  , 0     , 0     , 0x1   },    // EVEX 0F 38 CD
   {"vpaddsetsd",0x80  ,0x4A8200, 0x19  , 0x1603, 0x1603, 0x603 , 0     , 0     , 0x3406, 0     , 0x100 }};   // MVEX 0F 38 CD

// Submap for MVEX 0F 38 C6. W0
// Indexed by reg bits
SOpcodeDef OpcodeMap10D[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
{"vgatherpf0hintdp",0x80,0x439200, 0x1E,  0     , 0x264B, 0     , 0     , 0     , 0x1048, 0     , 0x101 },    // MVEX 0F 38 C6 /0
{"vgatherpf0dps",0x21,0xC38200,    0x1E,  0     , 0x224B, 0     , 0     , 0x1010, 0x1048, 0     , 0x000 },    // MVEX 0F 38 C6 /1
{"vgatherpf1dps",0x21,0xC38200,    0x1E,  0     , 0x224B, 0     , 0     , 0x1010, 0x1048, 0     , 0x000 },    // MVEX 0F 38 C6 /2
   {0       ,    0      , 0      , 0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
{"vscatterpf0hintdp",0x80,0x43B200,0x1E,  0     , 0x264B, 0     , 0     , 0     , 0x1048, 0     , 0x101 },    // MVEX 0F 38 C6 /4
{"vscatterpf0dps",0x21,0xC38200,   0x1E,  0     , 0x224B, 0     , 0     , 0x1010, 0x1048, 0     , 0x000 },    // MVEX 0F 38 C6 /5
{"vscatterpf1dps",0x21,0xC38200,   0x1E,  0     , 0x224B, 0     , 0     , 0x1010, 0x1048, 0     , 0x000 },    // MVEX 0F 38 C6 /6
   {0       ,    0      , 0      , 0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 

// Submap for MVEX 0F 38 C6. W1
// Indexed by reg bits
SOpcodeDef OpcodeMap10E[] = {
{"vgatherpf0hintdpd",0x80,0x439200,0x1E,  0     , 0x264C, 0     , 0     , 0     , 0x1048, 0     , 0x100 },    // MVEX 0F 38 C6 /0
{"vgatherpf0dpd",0x21,0xC3A200,    0x1E,  0     , 0x2F4C, 0     , 0     , 0x1010, 0x1048, 0     , 0x000 },    // MVEX 0F 38 C6 /1
{"vgatherpf1dpd",0x21,0xC3A200,    0x1E,  0     , 0x2F4C, 0     , 0     , 0x1010, 0x1048, 0     , 0x000 },    // MVEX 0F 38 C6 /2
   {0       ,    0      , 0      , 0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
{"vscatterpf0hintdp",0x80,0x43B200,0x1E,  0     , 0x264c, 0     , 0     , 0     , 0x1048, 0     , 0x101 },    // MVEX 0F 38 C6 /4
{"vscatterpf0dpd",0x21,0xC3A200,   0x1E,  0     , 0x2F4C, 0     , 0     , 0x1010, 0x1048, 0     , 0x000 },    // MVEX 0F 38 C6 /5
{"vscatterpf1dpd",0x21,0xC3A200,   0x1E,  0     , 0x2F4C, 0     , 0     , 0x1010, 0x1048, 0     , 0x100 },    // MVEX 0F 38 C6 /6
   {0       ,    0      , 0      , 0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 

// Submap for 0F 38 C7 vgatherpf.. Indexed by reg bits
SOpcodeDef OpcodeMap10F[] = {
   {0       ,    0      , 0      , 0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
{"vgatherpf0qp", 0x21   ,0xC39200, 0x1E,  0     , 0x224F, 0     , 0     , 0x1010, 0     , 0     , 0x1   },    // 0F 38 C7 /1
{"vgatherpf1qp", 0x21   ,0xC39200, 0x1E,  0     , 0x224F, 0     , 0     , 0x1010, 0     , 0     , 0x1   },    // 0F 38 C7 /2
   {0       ,    0      , 0      , 0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0       ,    0      , 0      , 0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
{"vscatterpf0qp",0x21   ,0xC39200, 0x1E,  0     , 0x224F, 0     , 0     , 0x1010, 0     , 0     , 0x1   },    // 0F 38 C7 /5
{"vscatterpf1qp",0x21   ,0xC39200, 0x1E,  0     , 0x224F, 0     , 0     , 0x1010, 0     , 0     , 0x1   },    // 0F 38 C7 /6
   {0       ,    0      , 0      , 0   ,  0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // 

//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
// Submap for 0F 1A. Indexed by 66 F2 F3 prefix
SOpcodeDef OpcodeMap110[] = {
   {"bndldx",    0x22   , 0      , 0x12,  0x98  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    //    0F 1A
   {"bndmov",    0x22   , 0x200  , 0x12,  0x1098, 0x98  , 0     , 0     , 0     , 0     , 0     , 0     },    // 66 0F 1A
   {"bndcu" ,    0x22   , 0x800  , 0x12,  0x98  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 1A
   {"bndcl" ,    0x22   , 0x400  , 0x12,  0x98  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F 1A

// Submap for 0F 1B. Indexed by 66 F2 F3 prefix
SOpcodeDef OpcodeMap111[] = {
   {"bndstx",    0x22   , 0      , 0x13,  0x2006, 0x98  , 0     , 0     , 0     , 0     , 0     , 0     },    //    0F 1B
   {"bndmov",    0x22   , 0x200  , 0x13,  0x98  , 0x1098, 0     , 0     , 0     , 0     , 0     , 0     },    // 66 0F 1B
   {"bndcn" ,    0x22   , 0x800  , 0x12,  0x98  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 1B
   {"bndmk" ,    0x22   , 0x400  , 0x12,  0x98  , 0x2006, 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F 1B

// Submap for 0F 3A 3E. Indexed by immediate byte. VCMPUB/W
SOpcodeDef OpcodeMap112[] = {
   {"vpcmpequ",  0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3E / 0
   {"vpcmpltu",  0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3E / 1
   {"vpcmpleu",  0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3E / 2
   {"vpcmpu",    0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0x31  , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3E / 3 = true
   {"vpcmpnequ", 0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3E / 4
   {"vpcmpnltu", 0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3E / 5
   {"vpcmpnleu", 0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3E / 6
   {"vpcmpu",    0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0x31  , 0x10  , 0     , 0     , 0x01  }};   // 0F 3A 3E / >= 7 = false

// Submap for 0F 3A 3F. Indexed by immediate byte. VCMPB/W
SOpcodeDef OpcodeMap113[] = {
   {"vpcmpeq",   0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3F / 0
   {"vpcmplt",   0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3F / 1
   {"vpcmple",   0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3F / 2
   {"vpcmp",     0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0x31  , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3F / 3 = true
   {"vpcmpneq",  0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3F / 4
   {"vpcmpnlt",  0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3F / 5
   {"vpcmpnle",  0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x10  , 0     , 0     , 0x01  },    // 0F 3A 3F / 6
   {"vpcmp",     0x20  , 0x8FC200,0x59  , 0x95  , 0x1209, 0x209 , 0x31  , 0x10  , 0     , 0     , 0x01  }};   // 0F 3A 3F / >= 7 = false

// Submap for 0F 3A 1E. Indexed by immediate byte. VCMPUD/Q
SOpcodeDef OpcodeMap114[] = {
   {"vpcmpequ",  0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1E / 0
   {"vpcmpltu",  0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1E / 1
   {"vpcmpleu",  0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1E / 2
   {"vpcmpu",    0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0x31  , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1E / 3 = true
   {"vpcmpnequ", 0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1E / 4
   {"vpcmpnltu", 0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1E / 5
   {"vpcmpnleu", 0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1E / 6
   {"vpcmpu",    0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0x31  , 0x11  , 0x1406, 0     , 0x01  }};   // 0F 3A 1E / >= 7 = false

// Submap for 0F 3A 1F. Indexed by immediate byte. VCMPD/Q
SOpcodeDef OpcodeMap115[] = {
   {"vpcmpeq",   0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1F / 0
   {"vpcmplt",   0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1F / 1
   {"vpcmple",   0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1F / 2
   {"vpcmp",     0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0x31  , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1F / 3 = true
   {"vpcmpneq",  0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1F / 4
   {"vpcmpnlt",  0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1F / 5
   {"vpcmpnle",  0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0     , 0x11  , 0x1406, 0     , 0x01  },    // 0F 3A 1F / 6
   {"vpcmp",     0x20  , 0xCBB200,0x59  , 0x95  , 0x1209, 0x209 , 0x31  , 0x11  , 0x1406, 0     , 0x01  }};   // 0F 3A 1F / >= 7 = false

// Submap for pcmpeqb. Opcode byte = 0F 74
// Indexed by E/MVEX prefix
SOpcodeDef OpcodeMap116[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"pcmpeqb",   0x7   , 0xD0200, 0x19  , 0x1101, 0x1101, 0x101 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 74
   {"vpcmpeqb",  0x20  ,0x8FA200, 0x19  , 0x95  , 0x1201, 0x201 , 0     , 0x10  , 0     , 0     , 0     }};   // E/MVEX 0F 76

// Submap for pcmpeqw. Opcode byte = 0F 75
// Indexed by E/MVEX prefix
SOpcodeDef OpcodeMap117[] = {
   {"pcmpeqw",   0x7   , 0xD0200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 75
   {"vpcmpeqw",  0x20  ,0x8FA200, 0x19  , 0x95  , 0x1202, 0x202 , 0     , 0x10  , 0     , 0     , 0     }};   // E/MVEX 0F 76

// Submap for pcmpgtb. Opcode byte = 0F 64
// Indexed by EVEX prefix
SOpcodeDef OpcodeMap118[] = {
   {"pcmpgtb",   0x7   , 0xD0200, 0x19  , 0x1101, 0x1101, 0x101 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 64
   {"vpcmpgtb",  0x20  ,0x8BA200, 0x19  , 0x95  , 0x1203, 0x203 , 0     , 0x10  , 0x1406, 0     , 0x000 }};   // E/MVEX 0F 64


// Submap for pcmpgtw. Opcode byte = 0F 65
// Indexed by EVEX prefix
SOpcodeDef OpcodeMap119[] = {
   {"pcmpgtw",   0x7   , 0xD0200, 0x19  , 0x1102, 0x1102, 0x102 , 0     , 0     , 0     , 0     , 0x2   },    // 0F 65
   {"vpcmpgtw",  0x20  ,0x8BA200, 0x19  , 0x95  , 0x1203, 0x203 , 0     , 0x10  , 0x1406, 0     , 0x000 }};   // E/MVEX 0F 65


// Opcode map for EVEX 66 0F 7B. Indexed by W bit
SOpcodeDef OpcodeMap11A[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {"vcvtps2qq", 0x20  ,0x840200, 0x12  , 0x204 , 0xF4B , 0     , 0     , 0x27  , 0     , 0     , 0     },    // EVEX 66 0F 7B. W = 0
   {"vcvtpd2qq", 0x20  ,0x841200, 0x12  , 0x204 , 0x24C , 0     , 0     , 0x27  , 0     , 0     , 0     }};   // EVEX 66 0F 7B. W = 1

// Opcode map for EVEX F2 0F 7A. Indexed by W bit
SOpcodeDef OpcodeMap11B[] = {
   {"vcvtudq2ps",0x20  ,0xC28800, 0x12  , 0x124B, 0x203 , 0     , 0     , 0x37  , 0x1214, 0     , 0     },    // F2 EVEX 0F 7A W0
   {"vcvtuqq2ps",0x20  ,0x869800, 0x12  , 0x1F4B, 0x204 , 0     , 0     , 0x27  , 0     , 0     , 0     }};   // F2 EVEX 0F 7A W0

// Opcode map for EVEX F3 0F 7A. Indexed by W bit
SOpcodeDef OpcodeMap11C[] = {
   {"vcvtudq2pd",0x20  ,0xC28400, 0x12  , 0x124C, 0xF03 , 0     , 0     , 0x31  , 0x1214, 0     , 0     },    // F3 E/MVEX 0F 7A W0
   {"vcvtuqq2pd",0x20  ,0x869800, 0x12  , 0x124C, 0x204 , 0     , 0     , 0x27  , 0     , 0     , 0     }};   // F2 EVEX 0F 7A W0

// Opcode map for 0F 3A 42. Indexed by EVEX
SOpcodeDef OpcodeMap11D[] = {
   {"mpsadbw",   0x15  , 0xD8200, 0x59  , 0x1202, 0x1202, 0x201 , 0x31  , 0     , 0     , 0     , 0x2   },    // 0F 3A 42
   {"vdbpsadbw", 0x20  ,0x8E8200, 0x59  , 0x1202, 0x1202, 0x201 , 0x31  , 0x20  , 0     , 0     , 0     }};   // EVEX 0F 3A 42

// Opcode map for 0F 3A 19. Indexed by EVEX
SOpcodeDef OpcodeMap11E[] = {
   {"vextractf128" ,0x19,0x978200,0x53,   0x450 , 0x1550, 0x31  , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 19
   {0              ,0x11F,0      ,0x53,   0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};   // EVEX 0F 3A 19

// Opcode map for EVEX 0F 3A 19. Indexed by W bit
SOpcodeDef OpcodeMap11F[] = {
   {"vextractf32x4",0x20,0x868200,0x53,   0x44B , 0x124B, 0x31  , 0     , 0x20  , 0     , 0     , 0     },    // EVEX W0 0F 3A 19
   {"vextractf64x2",0x20,0x869200,0x53,   0x44C , 0x124C, 0x31  , 0     , 0x20  , 0     , 0     , 0     }};   // EVEX W1 0F 3A 19

// Opcode map for EVEX 0F 3A 39. Indexed by EVEX
SOpcodeDef OpcodeMap120[] = {
   {"vextracti128",0x1C, 0x978200,0x53  , 0x406 , 0x1506, 0x31  , 0     , 0     , 0     , 0     , 0     },    // 0F 3A 39
   {0             ,0x121, 0      ,0x53  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};   // EVEX 0F 3A 39

// Opcode map for EVEX 0F 3A 39. Indexed by W bit
SOpcodeDef OpcodeMap121[] = {
   {"vextracti32x4",0x20,0x868200,0x53  , 0x406 , 0x1203, 0x31  , 0     , 0x20  , 0     , 0     , 0     },    // 0F 3A 39
   {"vextracti64x2",0x20,0x869200,0x53  , 0x406 , 0x1203, 0x31  , 0     , 0x20  , 0     , 0     , 0     }};   // 0F 3A 39

// Opcode map for 0F 3A 18. Indexed by EVEX
SOpcodeDef OpcodeMap122[] = {
   {"vinsertf128",0x19 ,0x9F8200, 0x59  , 0x1250, 0x1250, 0x450 , 0x31  , 0x30  , 0     , 0     , 0     },    // 0F 3A 18
   {0,           0x123 , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};

// Opcode map for EVEX 0F 3A 18. Indexed by W bit
SOpcodeDef OpcodeMap123[] = {
   {"vinsertf32x4",0x20,0x8AB200, 0x59  , 0x1250, 0x1250, 0x44B , 0x31  , 0x20  , 0     , 0     , 0     },    // EVEX 0F 3A 18. W0
   {"vinsertf64x2",0x20,0x8AB200, 0x59  , 0x1250, 0x1250, 0x44C , 0x31  , 0x20  , 0     , 0     , 0     }};   // EVEX 0F 3A 18. W0

// Opcode map for EVEX 0F 3A 1A. Indexed by W bit
SOpcodeDef OpcodeMap124[] = {
   {"vinsertf32x8",0x20,0x8AB200, 0x59  , 0x1250, 0x1250, 0x54B , 0x31  , 0x30  , 0     , 0     , 0     },    // 0F 3A 1A
   {"vinsertf64x4",0x20,0x8AB200, 0x59  , 0x1250, 0x1250, 0x54C , 0x31  , 0x30  , 0     , 0     , 0     }};   // 0F 3A 1A

// Opcode map for 0F 3A 38. Indexed by EVEX
SOpcodeDef OpcodeMap125[] = {
   {"vinserti128",0x1C ,0x9F8200, 0x59  , 0x1206, 0x1206, 0x406 , 0x31  , 0x30  , 0     , 0     , 0     },    // 0F 3A 38
   {0,           0x126 , 0      , 0x59  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};

// Opcode map for EVEX 0F 3A 38. Indexed by W bit
SOpcodeDef OpcodeMap126[] = {
   {"vinserti32x4",0x1C ,0x8AB200, 0x59  , 0x1203, 0x1203, 0x403 , 0x31  , 0x20  , 0     , 0     , 0     },   // EVEX 0F 3A 38. W0
   {"vinserti64x2",0x20 ,0x8AB200, 0x59  , 0x1204, 0x1204, 0x404 , 0x31  , 0x20  , 0     , 0     , 0     }};  // EVEX 0F 3A 38. W1

// Opcode map for EVEX 0F 3A 3A. Indexed by W bit
SOpcodeDef OpcodeMap127[] = {
   {"vinserti32x8",0x20,0x8AB200, 0x59  , 0x1203, 0x1203, 0x503 , 0x31   , 0x20  , 0     , 0     , 0     },    // EVEX 0F 3A 3A. W0
   {"vinserti64x4",0x20,0x8AB200, 0x59  , 0x1204, 0x1204, 0x504 , 0x31   , 0x20  , 0     , 0     , 0     }};   // EVEX 0F 3A 3A. W1

// Opcode map for 0F 38 B4. Indexed by VEX prefix type
SOpcodeDef OpcodeMap128[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX  0F 38 B4
   {"vpmadd52luq",0x23 ,0x8EB200, 0x19  , 0x1204, 0x1204, 0x204 , 0     , 0x21  , 0     , 0     , 0     },    // EVEX 0F 38 B4
   {"vpmadd233d",0x80  ,0x4A8200, 0x19  , 0x1603, 0x1603, 0x603 , 0     , 0     , 0x1406, 0     , 0x100 }};   // MVEX 0F 38 B4

// Opcode map for 0F 38 B5. Indexed by VEX prefix type
SOpcodeDef OpcodeMap129[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // VEX  0F 38 B5
   {"vpmadd52huq",0x23 ,0x8EB200, 0x19  , 0x1204, 0x1204, 0x204 , 0     , 0x21  , 0     , 0     , 0     },    // EVEX 0F 38 B5
   {"vpmadd231d",0x80  ,0x4A8200, 0x19  , 0x1603, 0x1603, 0x603 , 0     , 0     , 0x1406, 0     , 0x100 }};   // MVEX 0F 38 B5

// 0F 38 19 indexed by VEX / EVEX
SOpcodeDef OpcodeMap12A[] = {
   {"vbroadcastsd",0x19,0xC7A200, 0x12  , 0x124C, 0x04C , 0     , 0     , 0x20  , 0x1049, 0     , 0     },    // VEX  0F 38 19
   {0,           0x12B , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};   // EVEX 0F 38 19

// EVEX 0F 38 19 indexed by W bit
SOpcodeDef OpcodeMap12B[] = {
   {"vbroadcastf32x2",0x20,0xC6B200, 0x12,0x124C, 0x04B , 0     , 0     , 0x20  , 0x1049, 0     , 0     },    // EVEX W0 0F 38 19
   {"vbroadcastsd",0x20,0xC6B200, 0x12  , 0x124C, 0x04C , 0     , 0     , 0x20  , 0x1049, 0     , 0     }};    // EVEX W1 0F 38 19

// Opcode map for 0F 38 38. Indexed by prefix
SOpcodeDef OpcodeMap12C[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    //    0F 38 38
   {"pminsb",    0x15  ,0x8DA200, 0x19  , 0x1201, 0x1201, 0x201 , 0     , 0x20  , 0     , 0     , 0x2   },    // 66 0F 38 38
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 38 38
   // moved to map B2. this map can be removed
//   {"vpmovm2",   0x20  ,0x86B400, 0x12  , 0x1209, 0x95  , 0     , 0     , 0     , 0     , 0     , 1     }};   // F3 0F 38 38
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

// Opcode map for EVEX 0F 38 39. Indexed by prefix
SOpcodeDef OpcodeMap12D[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    //    0F 38 39
   {"vpmins",    0x15  ,0xCDB200, 0x19  , 0x1209, 0x1209, 0x209 , 0     , 0x31  , 0x1406, 0     , 0x1   },    // 66 0F 38 39
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F 38 39
   // this entry has been replaced by a link from map B2. this may be removed
 //  {0,           0x12F , 0      , 0x12  , 0     , 0     , 0     , 0     , 0     , 0     , 0xC   , 0     }};   // F3 0F 38 39
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

// Opcode map for EVEX 0F 38 29. Indexed by W bit
SOpcodeDef OpcodeMap12E[] = {
   {"vpmovb2m",  0x20  ,0x86C400, 0x12  , 0x95  , 0x1209, 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 29 W0
   {"vpmovw2m",  0x20  ,0x86C400, 0x12  , 0x95  , 0x1209, 0     , 0     , 0     , 0     , 0     , 0     }};   // EVEX F3 0F 38 29 W1

// Opcode map for EVEX 0F 38 39. Indexed by W bit
SOpcodeDef OpcodeMap12F[] = {
   {"vpmovd2m",  0x20  ,0x86B400, 0x12  , 0x95  , 0x1209, 0     , 0     , 0     , 0     , 0     , 0     },    // EVEX F3 0F 38 39 W0
   {"vpmovq2m",  0x20  ,0x86B400, 0x12  , 0x95  , 0x1209, 0     , 0     , 0     , 0     , 0     , 0     }};   // EVEX F3 0F 38 39 W1

// Opcode map for 0F 01, mod = 11b, reg = 5
// Indexed by rm bits
SOpcodeDef OpcodeMap130[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 0
   {0,           0x131 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 9     , 0     },    // rm = 1. link to incssp
   {0,           0x132 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 9     , 0     },    // rm = 2. link to savessp
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 3
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 4
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 5
   {"rdpkru",    0     , 0x000  , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 6
   {"wrpkru",    0     , 0x000  , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // rm = 7

// Opcode map for 0F 01, mod = 11b, reg = 5, rm = 1. Indexed by prefix
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
SOpcodeDef OpcodeMap131[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 66
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2
   {"incssp",    0     , 0x400  , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3

// Opcode map for 0F 01, mod = 11b, reg = 5, rm = 2. Indexed by prefix
SOpcodeDef OpcodeMap132[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 66
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2
   {"savessp",   0     , 0x400  , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3

// Opcode map for 0F 01, mod != 11b, reg = 5. Indexed by prefix
SOpcodeDef OpcodeMap133[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    //
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 66
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2
   {"rstorssp",  0     , 0x400  , 0x11  , 0     , 0x2004, 0     , 0     , 0     , 0     , 0     , 0     }};   // F3

// Opcode map for 0F AE /5. Link by prefix
SOpcodeDef OpcodeMap134[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 66
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2
   {"setssbsy",  0     , 0x400  , 0x11  , 0     , 0x2004, 0     , 0     , 0     , 0     , 0     , 0     }};   // F3

// Opcode map for 0F 1E. Hint instructions. Link by prefix
SOpcodeDef OpcodeMap135[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 66
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2
   {0,           0x136 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 4     , 0     }};   // F3

// Opcode map for F3 0F 1E. Hint instructions. Link by mod / reg
SOpcodeDef OpcodeMap136[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    //  mod < 3
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // mod = 3, reg = 0 
   {"rdssp",     0     , 0x1400 , 0x11  , 0     , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     },    // mod = 3, reg = 1 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // mod = 3, reg = 2 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // mod = 3, reg = 3 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // mod = 3, reg = 4 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // mod = 3, reg = 5 
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // mod = 3, reg = 6 
   {0,           0x137 , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 5     , 0     }};   // mod = 3, reg = 7 

// Opcode map for F3 0F 1E. mod = 3, reg = 7. Link by rm
SOpcodeDef OpcodeMap137[] = {
//  name         instset prefix   format  dest.   source1 source2 source3 EVEX    MVEX    link    options
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 0
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 1
   {"endbr64",   0     , 0x400  , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 2
   {"endbr32",   0     , 0x400  , 0x10  , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 3
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 4
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 5
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // rm = 6
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // rm = 7

// Submap for 0F C7 reg /7, Indexed by prefixes
SOpcodeDef OpcodeMap138[] = {
   {"rdseed",    0x1D  , 0x1100 , 0x11  , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 0F C7 reg /7
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // 66 0F C7 reg /7
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     },    // F2 0F C7 reg /7
   {"rdpid",     0x1D  , 0x1500 , 0x11  , 0x1009, 0     , 0     , 0     , 0     , 0     , 0     , 0     }};   // F3 0F C7 reg /7


SOpcodeDef OpcodeMap139[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

SOpcodeDef OpcodeMap13A[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

SOpcodeDef OpcodeMap13B[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

SOpcodeDef OpcodeMap13C[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

SOpcodeDef OpcodeMap13D[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

SOpcodeDef OpcodeMap13E[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};

SOpcodeDef OpcodeMap13F[] = {
   {0,           0     , 0      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     }};


/************** Make pointers to all opcode maps ***************************/
const SOpcodeDef * const OpcodeTables[] = {    
   OpcodeMap0,  OpcodeMap1,  OpcodeMap2,  OpcodeMap3, 
   OpcodeMap4,  OpcodeMap5,  OpcodeMap6,  OpcodeMap7,
   OpcodeMap8,  OpcodeMap9,  OpcodeMapA,  OpcodeMapB, 
   OpcodeMapC,  OpcodeMapD,  OpcodeMapE,  OpcodeMapF, 
   OpcodeMap10, OpcodeMap11, OpcodeMap12, OpcodeMap13,
   OpcodeMap14, OpcodeMap15, OpcodeMap16, OpcodeMap17,
   OpcodeMap18, OpcodeMap19, OpcodeMap1A, OpcodeMap1B,
   OpcodeMap1C, OpcodeMap1D, OpcodeMap1E, OpcodeMap1F,
   OpcodeMap20, OpcodeMap21, OpcodeMap22, OpcodeMap23,
   OpcodeMap24, OpcodeMap25, OpcodeMap26, OpcodeMap27,
   OpcodeMap28, OpcodeMap29, OpcodeMap2A, OpcodeMap2B,
   OpcodeMap2C, OpcodeMap2D, OpcodeMap2E, OpcodeMap2F,
   OpcodeMap30, OpcodeMap31, OpcodeMap32, OpcodeMap33,
   OpcodeMap34, OpcodeMap35, OpcodeMap36, OpcodeMap37,
   OpcodeMap38, OpcodeMap39, OpcodeMap3A, OpcodeMap3B,
   OpcodeMap3C, OpcodeMap3D, OpcodeMap3E, OpcodeMap3F,
   OpcodeMap40, OpcodeMap41, OpcodeMap42, OpcodeMap43,
   OpcodeMap44, OpcodeMap45, OpcodeMap46, OpcodeMap47,
   OpcodeMap48, OpcodeMap49, OpcodeMap4A, OpcodeMap4B, 
   OpcodeMap4C, OpcodeMap4D, OpcodeMap4E, OpcodeMap4F,
   OpcodeMap50, OpcodeMap51, OpcodeMap52, OpcodeMap53,
   OpcodeMap54, OpcodeMap55, OpcodeMap56, OpcodeMap57,
   OpcodeMap58, OpcodeMap59, OpcodeMap5A, OpcodeMap5B,
   OpcodeMap5C, OpcodeMap5D, OpcodeMap5E, OpcodeMap5F,
   OpcodeMap60, OpcodeMap61, OpcodeMap62, OpcodeMap63,
   OpcodeMap64, OpcodeMap65, OpcodeMap66, OpcodeMap67,
   OpcodeMap68, OpcodeMap69, OpcodeMap6A, OpcodeMap6B,
   OpcodeMap6C, OpcodeMap6D, OpcodeMap6E, OpcodeMap6F, 
   OpcodeMap70, OpcodeMap71, OpcodeMap72, OpcodeMap73,
   OpcodeMap74, OpcodeMap75, OpcodeMap76, OpcodeMap77, 
   OpcodeMap78, OpcodeMap79, OpcodeMap7A, OpcodeMap7B, 
   OpcodeMap7C, OpcodeMap7D, OpcodeMap7E, OpcodeMap7F, 
   OpcodeMap80, OpcodeMap81, OpcodeMap82, OpcodeMap83, 
   OpcodeMap84, OpcodeMap85, OpcodeMap86, OpcodeMap87,
   OpcodeMap88, OpcodeMap89, OpcodeMap8A, OpcodeMap8B,
   OpcodeMap8C, OpcodeMap8D, OpcodeMap8E, OpcodeMap8F, 
   OpcodeMap90, OpcodeMap91, OpcodeMap92, OpcodeMap93, 
   OpcodeMap94, OpcodeMap95, OpcodeMap96, OpcodeMap97,
   OpcodeMap98, OpcodeMap99, OpcodeMap9A, OpcodeMap9B,
   OpcodeMap9C, OpcodeMap9D, OpcodeMap9E, OpcodeMap9F, 
   OpcodeMapA0, OpcodeMapA1, OpcodeMapA2, OpcodeMapA3, 
   OpcodeMapA4, OpcodeMapA5, OpcodeMapA6, OpcodeMapA7,
   OpcodeMapA8, OpcodeMapA9, OpcodeMapAA, OpcodeMapAB,
   OpcodeMapAC, OpcodeMapAD, OpcodeMapAE, OpcodeMapAF,
   OpcodeMapB0, OpcodeMapB1, OpcodeMapB2, OpcodeMapB3,
   OpcodeMapB4, OpcodeMapB5, OpcodeMapB6, OpcodeMapB7,
   OpcodeMapB8, OpcodeMapB9, OpcodeMapBA, OpcodeMapBB,
   OpcodeMapBC, OpcodeMapBD, OpcodeMapBE, OpcodeMapBF,
   OpcodeMapC0, OpcodeMapC1, OpcodeMapC2, OpcodeMapC3,
   OpcodeMapC4, OpcodeMapC5, OpcodeMapC6, OpcodeMapC7,
   OpcodeMapC8, OpcodeMapC9, OpcodeMapCA, OpcodeMapCB,
   OpcodeMapCC, OpcodeMapCD, OpcodeMapCE, OpcodeMapCF,
   OpcodeMapD0, OpcodeMapD1, OpcodeMapD2, OpcodeMapD3,
   OpcodeMapD4, OpcodeMapD5, OpcodeMapD6, OpcodeMapD7,
   OpcodeMapD8, OpcodeMapD9, OpcodeMapDA, OpcodeMapDB,
   OpcodeMapDC, OpcodeMapDD, OpcodeMapDE, OpcodeMapDF,
   OpcodeMapE0, OpcodeMapE1, OpcodeMapE2, OpcodeMapE3,
   OpcodeMapE4, OpcodeMapE5, OpcodeMapE6, OpcodeMapE7,
   OpcodeMapE8, OpcodeMapE9, OpcodeMapEA, OpcodeMapEB,
   OpcodeMapEC, OpcodeMapED, OpcodeMapEE, OpcodeMapEF,
   OpcodeMapF0, OpcodeMapF1, OpcodeMapF2, OpcodeMapF3,
   OpcodeMapF4, OpcodeMapF5, OpcodeMapF6, OpcodeMapF7,
   OpcodeMapF8, OpcodeMapF9, OpcodeMapFA, OpcodeMapFB,
   OpcodeMapFC, OpcodeMapFD, OpcodeMapFE, OpcodeMapFF,
   OpcodeMap100, OpcodeMap101, OpcodeMap102, OpcodeMap103,
   OpcodeMap104, OpcodeMap105, OpcodeMap106, OpcodeMap107,
   OpcodeMap108, OpcodeMap109, OpcodeMap10A, OpcodeMap10B,
   OpcodeMap10C, OpcodeMap10D, OpcodeMap10E, OpcodeMap10F,
   OpcodeMap110, OpcodeMap111, OpcodeMap112, OpcodeMap113,
   OpcodeMap114, OpcodeMap115, OpcodeMap116, OpcodeMap117,
   OpcodeMap118, OpcodeMap119, OpcodeMap11A, OpcodeMap11B,
   OpcodeMap11C, OpcodeMap11D, OpcodeMap11E, OpcodeMap11F,
   OpcodeMap120, OpcodeMap121, OpcodeMap122, OpcodeMap123,
   OpcodeMap124, OpcodeMap125, OpcodeMap126, OpcodeMap127,
   OpcodeMap128, OpcodeMap129, OpcodeMap12A, OpcodeMap12B,
   OpcodeMap12C, OpcodeMap12D, OpcodeMap12E, OpcodeMap12F,
   OpcodeMap130, OpcodeMap131, OpcodeMap132, OpcodeMap133,
   OpcodeMap134, OpcodeMap135, OpcodeMap136, OpcodeMap137,
   OpcodeMap138, OpcodeMap139, OpcodeMap13A, OpcodeMap13B,
   OpcodeMap13C, OpcodeMap13D, OpcodeMap13E, OpcodeMap13F,
};

// size of each table pointed to by OpcodeTables[]
const uint32 OpcodeTableLength[] = {    
   TableSize(OpcodeMap0),  TableSize(OpcodeMap1),  TableSize(OpcodeMap2),  TableSize(OpcodeMap3), 
   TableSize(OpcodeMap4),  TableSize(OpcodeMap5),  TableSize(OpcodeMap6),  TableSize(OpcodeMap7), 
   TableSize(OpcodeMap8),  TableSize(OpcodeMap9),  TableSize(OpcodeMapA),  TableSize(OpcodeMapB), 
   TableSize(OpcodeMapC),  TableSize(OpcodeMapD),  TableSize(OpcodeMapE),  TableSize(OpcodeMapF),
   TableSize(OpcodeMap10), TableSize(OpcodeMap11), TableSize(OpcodeMap12), TableSize(OpcodeMap13),
   TableSize(OpcodeMap14), TableSize(OpcodeMap15), TableSize(OpcodeMap16), TableSize(OpcodeMap17),
   TableSize(OpcodeMap18), TableSize(OpcodeMap19), TableSize(OpcodeMap1A), TableSize(OpcodeMap1B),
   TableSize(OpcodeMap1C), TableSize(OpcodeMap1D), TableSize(OpcodeMap1E), TableSize(OpcodeMap1F),
   TableSize(OpcodeMap20), TableSize(OpcodeMap21), TableSize(OpcodeMap22), TableSize(OpcodeMap23),
   TableSize(OpcodeMap24), TableSize(OpcodeMap25), TableSize(OpcodeMap26), TableSize(OpcodeMap27),
   TableSize(OpcodeMap28), TableSize(OpcodeMap29), TableSize(OpcodeMap2A), TableSize(OpcodeMap2B),
   TableSize(OpcodeMap2C), TableSize(OpcodeMap2D), TableSize(OpcodeMap2E), TableSize(OpcodeMap2F),
   TableSize(OpcodeMap30), TableSize(OpcodeMap31), TableSize(OpcodeMap32), TableSize(OpcodeMap33),
   TableSize(OpcodeMap34), TableSize(OpcodeMap35), TableSize(OpcodeMap36), TableSize(OpcodeMap37),
   TableSize(OpcodeMap38), TableSize(OpcodeMap39), TableSize(OpcodeMap3A), TableSize(OpcodeMap3B),
   TableSize(OpcodeMap3C), TableSize(OpcodeMap3D), TableSize(OpcodeMap3E), TableSize(OpcodeMap3F), 
   TableSize(OpcodeMap40), TableSize(OpcodeMap41), TableSize(OpcodeMap42), TableSize(OpcodeMap43),
   TableSize(OpcodeMap44), TableSize(OpcodeMap45), TableSize(OpcodeMap46), TableSize(OpcodeMap47),
   TableSize(OpcodeMap48), TableSize(OpcodeMap49), TableSize(OpcodeMap4A), TableSize(OpcodeMap4B), 
   TableSize(OpcodeMap4C), TableSize(OpcodeMap4D), TableSize(OpcodeMap4E), TableSize(OpcodeMap4F),
   TableSize(OpcodeMap50), TableSize(OpcodeMap51), TableSize(OpcodeMap52), TableSize(OpcodeMap53),
   TableSize(OpcodeMap54), TableSize(OpcodeMap55), TableSize(OpcodeMap56), TableSize(OpcodeMap57),
   TableSize(OpcodeMap58), TableSize(OpcodeMap59), TableSize(OpcodeMap5A), TableSize(OpcodeMap5B), 
   TableSize(OpcodeMap5C), TableSize(OpcodeMap5D), TableSize(OpcodeMap5E), TableSize(OpcodeMap5F), 
   TableSize(OpcodeMap60), TableSize(OpcodeMap61), TableSize(OpcodeMap62), TableSize(OpcodeMap63), 
   TableSize(OpcodeMap64), TableSize(OpcodeMap65), TableSize(OpcodeMap66), TableSize(OpcodeMap67),
   TableSize(OpcodeMap68), TableSize(OpcodeMap69), TableSize(OpcodeMap6A), TableSize(OpcodeMap6B),
   TableSize(OpcodeMap6C), TableSize(OpcodeMap6D), TableSize(OpcodeMap6E), TableSize(OpcodeMap6F),
   TableSize(OpcodeMap70), TableSize(OpcodeMap71), TableSize(OpcodeMap72), TableSize(OpcodeMap73),
   TableSize(OpcodeMap74), TableSize(OpcodeMap75), TableSize(OpcodeMap76), TableSize(OpcodeMap77),
   TableSize(OpcodeMap78), TableSize(OpcodeMap79), TableSize(OpcodeMap7A), TableSize(OpcodeMap7B), 
   TableSize(OpcodeMap7C), TableSize(OpcodeMap7D), TableSize(OpcodeMap7E), TableSize(OpcodeMap7F),
   TableSize(OpcodeMap80), TableSize(OpcodeMap81), TableSize(OpcodeMap82), TableSize(OpcodeMap83),
   TableSize(OpcodeMap84), TableSize(OpcodeMap85), TableSize(OpcodeMap86), TableSize(OpcodeMap87),
   TableSize(OpcodeMap88), TableSize(OpcodeMap89), TableSize(OpcodeMap8A), TableSize(OpcodeMap8B),
   TableSize(OpcodeMap8C), TableSize(OpcodeMap8D), TableSize(OpcodeMap8E), TableSize(OpcodeMap8F),
   TableSize(OpcodeMap90), TableSize(OpcodeMap91), TableSize(OpcodeMap92), TableSize(OpcodeMap93),
   TableSize(OpcodeMap94), TableSize(OpcodeMap95), TableSize(OpcodeMap96), TableSize(OpcodeMap97),
   TableSize(OpcodeMap98), TableSize(OpcodeMap99), TableSize(OpcodeMap9A), TableSize(OpcodeMap9B), 
   TableSize(OpcodeMap9C), TableSize(OpcodeMap9D), TableSize(OpcodeMap9E), TableSize(OpcodeMap9F),
   TableSize(OpcodeMapA0), TableSize(OpcodeMapA1), TableSize(OpcodeMapA2), TableSize(OpcodeMapA3),
   TableSize(OpcodeMapA4), TableSize(OpcodeMapA5), TableSize(OpcodeMapA6), TableSize(OpcodeMapA7),
   TableSize(OpcodeMapA8), TableSize(OpcodeMapA9), TableSize(OpcodeMapAA), TableSize(OpcodeMapAB),
   TableSize(OpcodeMapAC), TableSize(OpcodeMapAD), TableSize(OpcodeMapAE), TableSize(OpcodeMapAF),
   TableSize(OpcodeMapB0), TableSize(OpcodeMapB1), TableSize(OpcodeMapB2), TableSize(OpcodeMapB3),
   TableSize(OpcodeMapB4), TableSize(OpcodeMapB5), TableSize(OpcodeMapB6), TableSize(OpcodeMapB7),
   TableSize(OpcodeMapB8), TableSize(OpcodeMapB9), TableSize(OpcodeMapBA), TableSize(OpcodeMapBB),
   TableSize(OpcodeMapBC), TableSize(OpcodeMapBD), TableSize(OpcodeMapBE), TableSize(OpcodeMapBF),
   TableSize(OpcodeMapC0), TableSize(OpcodeMapC1), TableSize(OpcodeMapC2), TableSize(OpcodeMapC3),
   TableSize(OpcodeMapC4), TableSize(OpcodeMapC5), TableSize(OpcodeMapC6), TableSize(OpcodeMapC7),
   TableSize(OpcodeMapC8), TableSize(OpcodeMapC9), TableSize(OpcodeMapCA), TableSize(OpcodeMapCB),
   TableSize(OpcodeMapCC), TableSize(OpcodeMapCD), TableSize(OpcodeMapCE), TableSize(OpcodeMapCF),
   TableSize(OpcodeMapD0), TableSize(OpcodeMapD1), TableSize(OpcodeMapD2), TableSize(OpcodeMapD3),
   TableSize(OpcodeMapD4), TableSize(OpcodeMapD5), TableSize(OpcodeMapD6), TableSize(OpcodeMapD7),
   TableSize(OpcodeMapD8), TableSize(OpcodeMapD9), TableSize(OpcodeMapDA), TableSize(OpcodeMapDB),
   TableSize(OpcodeMapDC), TableSize(OpcodeMapDD), TableSize(OpcodeMapDE), TableSize(OpcodeMapDF),
   TableSize(OpcodeMapE0), TableSize(OpcodeMapE1), TableSize(OpcodeMapE2), TableSize(OpcodeMapE3),
   TableSize(OpcodeMapE4), TableSize(OpcodeMapE5), TableSize(OpcodeMapE6), TableSize(OpcodeMapE7),
   TableSize(OpcodeMapE8), TableSize(OpcodeMapE9), TableSize(OpcodeMapEA), TableSize(OpcodeMapEB),
   TableSize(OpcodeMapEC), TableSize(OpcodeMapED), TableSize(OpcodeMapEE), TableSize(OpcodeMapEF),
   TableSize(OpcodeMapF0), TableSize(OpcodeMapF1), TableSize(OpcodeMapF2), TableSize(OpcodeMapF3),
   TableSize(OpcodeMapF4), TableSize(OpcodeMapF5), TableSize(OpcodeMapF6), TableSize(OpcodeMapF7),
   TableSize(OpcodeMapF8), TableSize(OpcodeMapF9), TableSize(OpcodeMapFA), TableSize(OpcodeMapFB),
   TableSize(OpcodeMapFC), TableSize(OpcodeMapFD), TableSize(OpcodeMapFE), TableSize(OpcodeMapFF),
   TableSize(OpcodeMap100), TableSize(OpcodeMap101), TableSize(OpcodeMap102), TableSize(OpcodeMap103),
   TableSize(OpcodeMap104), TableSize(OpcodeMap105), TableSize(OpcodeMap106), TableSize(OpcodeMap107),
   TableSize(OpcodeMap108), TableSize(OpcodeMap109), TableSize(OpcodeMap10A), TableSize(OpcodeMap10B),
   TableSize(OpcodeMap10C), TableSize(OpcodeMap10D), TableSize(OpcodeMap10E), TableSize(OpcodeMap10F),
   TableSize(OpcodeMap110), TableSize(OpcodeMap111), TableSize(OpcodeMap112), TableSize(OpcodeMap113),
   TableSize(OpcodeMap114), TableSize(OpcodeMap115), TableSize(OpcodeMap116), TableSize(OpcodeMap117),
   TableSize(OpcodeMap118), TableSize(OpcodeMap119), TableSize(OpcodeMap11A), TableSize(OpcodeMap11B),
   TableSize(OpcodeMap11C), TableSize(OpcodeMap11D), TableSize(OpcodeMap11E), TableSize(OpcodeMap11F),
   TableSize(OpcodeMap120), TableSize(OpcodeMap121), TableSize(OpcodeMap122), TableSize(OpcodeMap123),
   TableSize(OpcodeMap124), TableSize(OpcodeMap125), TableSize(OpcodeMap126), TableSize(OpcodeMap127),
   TableSize(OpcodeMap128), TableSize(OpcodeMap129), TableSize(OpcodeMap12A), TableSize(OpcodeMap12B),
   TableSize(OpcodeMap12C), TableSize(OpcodeMap12D), TableSize(OpcodeMap12E), TableSize(OpcodeMap12F),
   TableSize(OpcodeMap130), TableSize(OpcodeMap131), TableSize(OpcodeMap132), TableSize(OpcodeMap133),
   TableSize(OpcodeMap134), TableSize(OpcodeMap135), TableSize(OpcodeMap136), TableSize(OpcodeMap137),
   TableSize(OpcodeMap138), TableSize(OpcodeMap139), TableSize(OpcodeMap13A), TableSize(OpcodeMap13B),
   TableSize(OpcodeMap13C), TableSize(OpcodeMap13D), TableSize(OpcodeMap13E), TableSize(OpcodeMap13F),
};

// number of entries in OpcodeTables
const uint32 NumOpcodeTables1 = TableSize(OpcodeTables);         
const uint32 NumOpcodeTables2 = TableSize(OpcodeTableLength);         

// Index to start pages, depending on VEX.mmmm bits
uint32 OpcodeStartPageVEX[] = {
   0xB0,                        // no escape,      VEX.mmmm = 0
   0xB1,                        // 0F    escape or VEX.mmmm = 1
   0x2,                         // 0F 38 escape or VEX.mmmm = 2
   0x4,                         // 0f 3A escape or VEX.mmmm = 3
   0, 0, 0, 0,                  // reserved for higher mmmm
   0xB2,                        // 0F 38 escape or EVEX.mmmm = 2 with F2 prefix (pp = 11)
   0xB3                         // 0F 38 escape or EVEX.mmmm = 2 with F3 prefix (pp = 10)
};

// Index to start pages, depending on XOP.mmmm bits
SOpcodeDef const * OpcodeStartPageXOP[] = {
   OpcodeMap64,                        // XOP.mmmm = 8
   OpcodeMap65,                        // XOP.mmmm = 9
   OpcodeMap66                         // XOP.mmmm = 0xA
};   

// Number of entries in OpcodeStartPages
const uint32 NumOpcodeStartPageVEX = TableSize(OpcodeStartPageVEX);
const uint32 NumOpcodeStartPageXOP = TableSize(OpcodeStartPageXOP);


// Define register names

// Names of 8 bit registers
const char * RegisterNames8[8] = {
   "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };

// Names of 8 bit registers with REX prefix
const char * RegisterNames8x[16] = {
   "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil", 
   "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"  };

// Names of 16 bit registers
const char * RegisterNames16[16] = {
   "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", 
   "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w" };
      
// Names of 32 bit registers
const char * RegisterNames32[16] = {
   "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", 
   "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d" };
      
// Names of 64 bit registers
const char * RegisterNames64[16] = {
   "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", 
   "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15" };
      
// Names of segment registers
const char * RegisterNamesSeg[8] = {
   "es", "cs", "ss", "ds", "fs", "gs", "hs?", "is?" }; // Last two are illegal or undocumented

// Names of control registers
const char * RegisterNamesCR[16] = {
   "cr0", "cr1 ?", "cr2", "cr3", "cr4", "cr5 ?", "cr6 ?", "cr7 ?", 
   "cr8", "cr9 ?", "cr10 ?", "cr11 ?", "cr12 ?", "cr13 ?", "cr14 ?", "cr15 ?" }; // Those with ? are illegal


// MVEX tables: Tables of the meaning of the sss bits in a MVEX prefix

SwizSpec Sf32r[8] = {  // 32-bit float or integer register permutation
    {0x64B,64,4,""},
    {0x64B,64,4,"cdab"},
    {0x64B,64,4,"badc"},
    {0x64B,64,4,"dacb"},
    {0x64B,64,4,"aaaa"},
    {0x64B,64,4,"bbbb"},
    {0x64B,64,4,"cccc"},
    {0x64B,64,4,"dddd"}};

SwizSpec Sf64r[8] = {  // 64-bit float or integer register permutation
    {0x64C,64,8,""},
    {0x64C,64,8,"cdab"},
    {0x64C,64,8,"badc"},
    {0x64C,64,8,"dacb"},
    {0x64C,64,8,"aaaa"},
    {0x64C,64,8,"bbbb"},
    {0x64C,64,8,"cccc"},
    {0x64C,64,8,"dddd"}};

SwizSpec Sf32m[8] = {  // 32-bit float memory broadcast or conversion
    {0x64B,64,4,""},
    {0x04B, 4,4,"1to16"},
    {0x44B,16,4,"4to16"},
    {0x54A,32,2,"float16"},
    {0x401,16,1,"uint8"},
    {0x401,16,1,"sint8 N/A!"},
    {0x502,32,2,"uint16"},
    {0x502,32,2,"sint16"}};

SwizSpec Sf64m[8] = {  // 64-bit float memory broadcast (no conversion)
    {0x64C,64,8,""},
    {0x04C, 8,8,"1to8"},
    {0x54C,32,8,"4to8"},
    {0x64C,64,8,"N/A!"},
    {0x64C,64,8,"N/A!"},
    {0x64C,64,8,"N/A!"},
    {0x64C,64,8,"N/A!"},
    {0x64C,64,8,"N/A!"}};

SwizSpec Si32m[8] = {  // 32-bit integer memory broadcast or conversion
    {0x603,64,4,""},
    {0x003, 4,4,"1to16"},
    {0x403,16,4,"4to16"},
    {0x54A,32,2,"N/A!"},
    {0x401,16,1,"uint8"},
    {0x401,16,1,"sint8"},
    {0x502,32,2,"uint16"},
    {0x502,32,2,"sint16"}};

SwizSpec Si64m[8] = {  // 64-bit integer memory broadcast (no conversion)
    {0x604,64,8,""},
    {0x004, 8,8,"1to8"},
    {0x504,32,8,"4to8"},
    {0x604,64,8,"N/A!"},
    {0x604,64,8,"N/A!"},
    {0x604,64,8,"N/A!"},
    {0x604,64,8,"N/A!"},
    {0x604,64,8,"N/A!"}};

SwizSpec Uf32m[8] = {  // 32-bit float memory up- or down-conversion
    {0x64B,64,4,""},
    {0x04B, 4,4,"N/A!"},
    {0x54B,16,4,"N/A!"},
    {0x54A,32,2,"float16"},
    {0x401,16,1,"uint8"},
    {0x401,16,1,"sint8"},
    {0x502,32,2,"uint16"},
    {0x502,32,2,"sint16"}};

SwizSpec Uf64m[8] = {  // 64-bit float memory, no up- or down-conversion
    {0x64C,64,8,""},
    {0x64C,64,8,"N/A!"},
    {0x64C,64,8,"N/A!"},
    {0x64C,64,8,"N/A!"},
    {0x64C,64,8,"N/A!"},
    {0x64C,64,8,"N/A!"},
    {0x64C,64,8,"N/A!"},
    {0x64C,64,8,"N/A!"}};

SwizSpec Ui32m[8] = {  // 32-bit integer memory up- or down-conversion
    {0x603,64,4,""},
    {0x003, 4,4,"N/A!"},
    {0x503,16,4,"N/A!"},
    {0x54A,32,2,"N/A!"},
    {0x401,16,1,"uint8"},
    {0x401,16,1,"sint8"},
    {0x502,32,2,"uint16"},
    {0x502,32,2,"sint16"}};

SwizSpec Ui64m[8] = {  // 64-bit integer memory, no up- or down-conversion
    {0x604,64,8,""},
    {0x604,64,8,"N/A!"},
    {0x604,64,8,"N/A!"},
    {0x604,64,8,"N/A!"},
    {0x604,64,8,"N/A!"},
    {0x604,64,8,"N/A!"},
    {0x604,64,8,"N/A!"},
    {0x604,64,8,"N/A!"}};

// special cases:

SwizSpec Uf32mx4[8] = {  // 32-bit float memory up-conversion, broadcast * 4, vbroadcastf32x4
    {0x44B,16,4*4,""},
    {0x04B,16,4*4,"N/A!"},
    {0x54B,16,4*4,"N/A!"},
    {0x004, 8,2*4,"float16"},
    {0x003, 4,1*4,"uint8"},
    {0x003, 4,1*4,"sint8"},
    {0x004, 8,2*4,"uint16"},
    {0x004, 8,2*4,"sint16"}};

SwizSpec Uf64mx4[8] = {  // 64-bit float memory, no up-conversion, broadcast * 4, vbroadcastf64x4
    {0x54C,32,8*4,""},
    {0x54C,32,8*4,"N/A!"},
    {0x54C,32,8*4,"N/A!"},
    {0x54C,32,8*4,"N/A!"},
    {0x54C,32,8*4,"N/A!"},
    {0x54C,32,8*4,"N/A!"},
    {0x54C,32,8*4,"N/A!"},
    {0x54C,32,8*4,"N/A!"}};

SwizSpec Ui32mx4[8] = {  // 32-bit integer memory up-conversion, broadcast * 4, vbroadcasti32x4
    {0x403,16,4*4,""},
    {0x003,16,4*4,"N/A!"},
    {0x503,16,4*4,"N/A!"},
    {0x54A, 8,2*4,"N/A!"},
    {0x003, 4,1*4,"uint8"},
    {0x003, 4,1*4,"sint8"},
    {0x004, 8,2*4,"uint16"},
    {0x004, 8,2*4,"sint16"}};

SwizSpec Ui64mx4[8] = {  // 64-bit integer memory, no up-conversion, broadcast * 4, vbroadcasti64x4
    {0x504,32,8*4,""},
    {0x504,32,8*4,"N/A!"},
    {0x504,32,8*4,"N/A!"},
    {0x504,32,8*4,"N/A!"},
    {0x504,32,8*4,"N/A!"},
    {0x504,32,8*4,"N/A!"},
    {0x504,32,8*4,"N/A!"},
    {0x504,32,8*4,"N/A!"}};

SwizSpec Si32mHalf[8] = {  // 32-bit integer memory broadcast with conversion to double (VCVTDQ2PD)
    {0x503,32,4,""},
    {0x003, 4,4,"1to8"},
    {0x403,16,4,"4to8"},
    {0x503,32,4,"N/A!"},
    {0x503,32,4,"N/A!"},
    {0x503,32,4,"N/A!"},
    {0x503,32,4,"N/A!"},
    {0x503,32,4,"N/A!"}};

SwizSpec Sf32mHalf[8] = {  // 32-bit float memory broadcast or conversion (vcvtps2pd)
    {0x54B,32,4,""},
    {0x04B, 4,4,"1to8"},
    {0x44B,16,4,"4to8"},
    {0x54A,32,2,"N/A!"},
    {0x401,16,1,"N/A!"},
    {0x401,16,1,"N/A!"},
    {0x502,32,2,"N/A!"},
    {0x502,32,2,"N/A!"}};

SwizSpec Snone[8] = {  // No swizzle
    {0x600,64,8*4,""},
    {0x600,64,8*4,"N/A!"},
    {0x600,64,8*4,"N/A!"},
    {0x600,64,8*4,"N/A!"},
    {0x600,64,8*4,"N/A!"},
    {0x600,64,8*4,"N/A!"},
    {0x600,64,8*4,"N/A!"},
    {0x600,64,8*4,"N/A!"}};

SwizSpec Sf32mfmadd233[8] = {  // 32-bit float memory, without register swizzle and limited broadcast, vfmadd233ps
    {0x64B,64,4,""},
    {0x04B, 4,4,"N/A!"},
    {0x44B,16,4,"4to16"},
    {0x54A,32,2,"N/A!"},
    {0x401,16,1,"N/A!"},
    {0x401,16,1,"N/A!"},
    {0x502,32,2,"N/A!"},
    {0x502,32,2,"N/A!"}};

SwizSpec Sdummy[8] = {  // For unused entries
    {0,0,0,""},
    {0,0,0,"??"},
    {0,0,0,"??"},
    {0,0,0,"??"},
    {0,0,0,"??"},
    {0,0,0,"??"},
    {0,0,0,"??"},
    {0,0,0,"??"}};

SwizSpec Signore[8] = {  // sss bits ignored or used only for sae. Offset multiplier defined
    {0x603,64,4,""},
    {0x603,64,4,""},
    {0x603,64,4,""},
    {0x603,64,4,""},
    {0x603,64,4,""},
    {0x603,64,4,""},
    {0x603,64,4,""},
    {0x603,64,4,""}};

SwizSpec Signore1[8] = {  // sss bits ignored or used only for sae. Offset multiplier defined, vector size not defined
    {0x000,64,4,""},
    {0x000,64,4,""},
    {0x000,64,4,""},
    {0x000,64,4,""},
    {0x000,64,4,""},
    {0x000,64,4,""},
    {0x000,64,4,""},
    {0x000,64,4,""}};

// Table of swizzle tables
SwizSpec const * SwizTables[][2] = {
    {Sdummy,Sdummy},     //  0 no swizzle, sss must be zero
    {Signore,Signore},   //  1 sss ignored or used only for sae, offset multiplier defined
    {Signore1,Signore1}, //  2 sss ignored or used only for sae, offset multiplier defined, no vector size
    {Sdummy,Sdummy},     //  3 unused
    {Sf32r,Sf32m},       //  4 Sf32
    {Sf64r,Sf64m},       //  5 Sf64
    {Sf32r,Si32m},       //  6 Si32
    {Sf64r,Si64m},       //  7 Si64
    {Sdummy,Uf32m},      //  8 Uf32
    {Sdummy,Uf64m},      //  9 Uf64
    {Sf32r,Ui32m},       //  A Ui32
    {Sf64r,Ui64m},       //  B Ui64
    {Sdummy,Uf32m},      //  C Df32
    {Sdummy,Uf64m},      //  D Df64
    {Sf32r,Ui32m},       //  E Di32
    {Sf64r,Ui64m},       //  F Di64
    // special cases
    {Sdummy,Uf32mx4},    // 10 Uf32 vbroadcastf32x4
    {Sdummy,Uf64mx4},    // 11 Uf64 vbroadcastf64x4
    {Sdummy,Ui32mx4},    // 12 Ui32 vbroadcasti32x4
    {Sdummy,Ui64mx4},    // 13 Ui64 vbroadcasti64x4
    {Sf32r,Si32mHalf},   // 14 Si32 vcvtdq2pd, vcvtudq2pd
    {Sf32r,Sf32mHalf},   // 15 Sf32 vcvtps2pd
    {Snone,Sf32mfmadd233}// 16 Sf32 without register swizzle and limited broadcast, vfmadd233ps
};

SwizSpec Sround_1[8] = {  // Register operand rounding mode and suppress all exceptions
    {0x64B,32,4,"rn"},    // syntax 1
    {0x64B,32,4,"rd"},
    {0x64B,32,4,"ru"},
    {0x64B,32,4,"rz"},
    {0x64B,32,4,"rn-sae"},
    {0x64B,32,4,"rd-sae"},
    {0x64B,32,4,"ru-sae"},
    {0x64B,32,4,"rz-sae"}};

SwizSpec Sround_2[8] = {  // Register operand rounding mode and suppress all exceptions
    {0x64B,32,4,"rn"},    // alternative syntax
    {0x64B,32,4,"rd"},
    {0x64B,32,4,"ru"},
    {0x64B,32,4,"rz"},
    {0x64B,32,4,"rn} {sae"},
    {0x64B,32,4,"rd} {sae"},
    {0x64B,32,4,"ru} {sae"},
    {0x64B,32,4,"rz} {sae"}};

// Table of swizzle tables for rounding mode
SwizSpec const * SwizRoundTables[1][2] = {
    {Sround_1,Sround_2}
};

// EVEX tables: Tables of rounding mode names for EVEX
const char * EVEXRoundingNames[5] = {
    "rn-sae", "rd-sae", "ru-sae", "rz-sae", "sae"
};

