/********************************************************************************/
/*                                                                              */
/* CZ80 XY opcode include source file                                           */
/* C Z80 emulator version 0.91                                                  */
/* Copyright 2004-2005 Stephane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

#if CZ80_USE_JUMPTABLE
    goto *JumpTableXY[Opcode];
#else
switch (Opcode)
{
#endif

    OPXY(0x00): // NOP

    // 8 BITS LOAD

    OPXY(0x40): // LD   B,B
    OPXY(0x49): // LD   C,C
    OPXY(0x52): // LD   D,D
    OPXY(0x5b): // LD   E,E
    OPXY(0x64): // LD   H,H
    OPXY(0x6d): // LD   L,L
    OPXY(0x7f): // LD   A,A
        goto OP_NOP;

    OPXY(0x41): // LD   B,C
    OPXY(0x42): // LD   B,D
    OPXY(0x43): // LD   B,E
    OPXY(0x47): // LD   B,A

    OPXY(0x48): // LD   C,B
    OPXY(0x4a): // LD   C,D
    OPXY(0x4b): // LD   C,E
    OPXY(0x4f): // LD   C,A

    OPXY(0x50): // LD   D,B
    OPXY(0x51): // LD   D,C
    OPXY(0x53): // LD   D,E
    OPXY(0x57): // LD   D,A

    OPXY(0x58): // LD   E,B
    OPXY(0x59): // LD   E,C
    OPXY(0x5a): // LD   E,D
    OPXY(0x5f): // LD   E,A

    OPXY(0x78): // LD   A,B
    OPXY(0x79): // LD   A,C
    OPXY(0x7a): // LD   A,D
    OPXY(0x7b): // LD   A,E
        goto OP_LD_R_R;

    OPXY(0x44): // LD   B,HX
    OPXY(0x4c): // LD   C,HX
    OPXY(0x54): // LD   D,HX
    OPXY(0x5c): // LD   E,HX
    OPXY(0x7c): // LD   A,HX
        zR8((Opcode >> 3) & 7) = data->B.H;
        RET(4)

    OPXY(0x45): // LD   B,LX
    OPXY(0x4d): // LD   C,LX
    OPXY(0x55): // LD   D,LX
    OPXY(0x5d): // LD   E,LX
    OPXY(0x7d): // LD   A,LX
        zR8((Opcode >> 3) & 7) = data->B.L;
        RET(4)

    OPXY(0x60): // LD   HX,B
    OPXY(0x61): // LD   HX,C
    OPXY(0x62): // LD   HX,D
    OPXY(0x63): // LD   HX,E
    OPXY(0x67): // LD   HX,A
        data->B.H = zR8(Opcode & 7);
        RET(4)

    OPXY(0x68): // LD   LX,B
    OPXY(0x69): // LD   LX,C
    OPXY(0x6a): // LD   LX,D
    OPXY(0x6b): // LD   LX,E
    OPXY(0x6f): // LD   LX,A
        data->B.L = zR8(Opcode & 7);
        RET(4)

    OPXY(0x65): // LD   HX,LX
        data->B.H = data->B.L;
        RET(4)

    OPXY(0x6c): // LD   LX,HX
        data->B.L = data->B.H;
        RET(4)

    OPXY(0x06): // LD   B,#imm
    OPXY(0x0e): // LD   C,#imm
    OPXY(0x16): // LD   D,#imm
    OPXY(0x1e): // LD   E,#imm
    OPXY(0x3e): // LD   A,#imm
        goto OP_LD_R_imm;

    OPXY(0x26): // LD   HX,#imm
        data->B.H = FETCH_BYTE;
        RET(7)

    OPXY(0x2e): // LD   LX,#imm
        data->B.L = FETCH_BYTE;
        RET(7)

    OPXY(0x0a): // LD   A,(BC)
        goto OP_LOAD_A_mBC;

    OPXY(0x1a): // LD   A,(DE)
        goto OP_LOAD_A_mDE;

    OPXY(0x3a): // LD   A,(nn)
        goto OP_LOAD_A_mNN;

    OPXY(0x02): // LD   (BC),A
        goto OP_LOAD_mBC_A;

    OPXY(0x12): // LD   (DE),A
        goto OP_LOAD_mDE_A;

    OPXY(0x32): // LD   (nn),A
        goto OP_LOAD_mNN_A;

    {
        uint16_t adr;

    OPXY(0x46): // LD   B,(IX+o)
    OPXY(0x56): // LD   D,(IX+o)
    OPXY(0x5e): // LD   E,(IX+o)
    OPXY(0x66): // LD   H,(IX+o)
    OPXY(0x6e): // LD   L,(IX+o)
    OPXY(0x4e): // LD   C,(IX+o)
    OPXY(0x7e): // LD   A,(IX+o)
        PRE_IO
        adr = data->W + FETCH_BYTE_S;
        READ_BYTE(adr, zR8((Opcode >> 3) & 7))
        POST_IO
        RET(15)

    OPXY(0x70): // LD   (IX+o),B
    OPXY(0x71): // LD   (IX+o),C
    OPXY(0x72): // LD   (IX+o),D
    OPXY(0x73): // LD   (IX+o),E
    OPXY(0x74): // LD   (IX+o),H
    OPXY(0x75): // LD   (IX+o),L
    OPXY(0x77): // LD   (IX+o),A
        PRE_IO
        adr = data->W + FETCH_BYTE_S;
        WRITE_BYTE(adr, zR8(Opcode & 7))
        POST_IO
        RET(15)

    OPXY(0x36): // LD   (IX+o),#imm
        PRE_IO
        adr = data->W + FETCH_BYTE_S;
        WRITE_BYTE(adr, FETCH_BYTE)
        POST_IO
        RET(15)
    }

    // 16 BITS LOAD

    OPXY(0x01): // LD   BC,nn
    OPXY(0x11): // LD   DE,nn
        goto OP_LOAD_RR_imm16;

    OPXY(0x21): // LD   IX,nn
        FETCH_WORD(data->W)
        RET(10)

    OPXY(0x31): // LD   SP,nn
        goto OP_LOAD_SP_imm16;

    OPXY(0x2a): // LD   IX,(w)
        goto OP_LD_xx_mNN;

    OPXY(0x22): // LD   (w),IX
        goto OP_LD_mNN_xx;

    OPXY(0xf9): // LD   SP,IX
        goto OP_LD_SP_xx;


    // PUSH / POP

    OPXY(0xf1): // POP  AF
        goto OP_POP_AF;

    OPXY(0xc1): // POP  BC
    OPXY(0xd1): // POP  DE
        goto OP_POP_RR;

    OPXY(0xe1): // POP  IX
        goto OP_POP;

    OPXY(0xf5): // PUSH AF
        goto OP_PUSH_AF;

    OPXY(0xc5): // PUSH BC
    OPXY(0xd5): // PUSH DE
        goto OP_PUSH_RR;

    OPXY(0xe5): // PUSH IX
        goto OP_PUSH;


    // EXCHANGE

    OPXY(0x08): // EX   AF,AF'
        goto OP_EX_AF_AF2;

    OPXY(0xeb): // EX   DE,HL
        goto OP_EX_DE_HL;

    OPXY(0xd9): // EXX
        goto OP_EXX;

    OPXY(0xe3): // EX   (SP),IX
        goto OP_EX_xx_mSP;


    // 8 BITS ARITHMETIC
    // INC

    OPXY(0x04): // INC  B
    OPXY(0x0c): // INC  C
    OPXY(0x14): // INC  D
    OPXY(0x1c): // INC  E
    OPXY(0x3c): // INC  A
        goto OP_INC_R;

    OPXY(0x24): // INC  HX
        data->B.H++;
        zF = (zF & CZ80_CF) | SZXYHV_inc[data->B.H];
        RET(4)

    OPXY(0x2c): // INC  LX
        data->B.L++;
        zF = (zF & CZ80_CF) | SZXYHV_inc[data->B.L];
        RET(4)

    OPXY(0x34): // INC  (IX+o)
        goto OP_INC_mIx;

    // DEC

    OPXY(0x05): // DEC  B
    OPXY(0x0d): // DEC  C
    OPXY(0x15): // DEC  D
    OPXY(0x1d): // DEC  E
    OPXY(0x3d): // DEC  A
        goto OP_DEC_R;

    OPXY(0x25): // DEC  HX
        data->B.H--;
        zF = (zF & CZ80_CF) | SZXYHV_dec[data->B.H];
        RET(4)

    OPXY(0x2d): // DEC  LX
        data->B.L--;
        zF = (zF & CZ80_CF) | SZXYHV_dec[data->B.L];
        RET(4)

    OPXY(0x35): // DEC  (IX+o)
        goto OP_DEC_mIx;


    // ADD

    OPXY(0x80): // ADD  A,B
    OPXY(0x81): // ADD  A,C
    OPXY(0x82): // ADD  A,D
    OPXY(0x83): // ADD  A,E
    OPXY(0x87): // ADD  A,A
        goto OP_ADD_R;

    OPXY(0x84): // ADD  A,HX
        goto OP_ADD_IxH;

    OPXY(0x85): // ADD  A,LX
        goto OP_ADD_IxL;

    OPXY(0xc6): // ADD  A,n
        goto OP_ADD_imm;

    OPXY(0x86): // ADD  A,(IX+o)
        goto OP_ADD_mIx;

    // ADC

    OPXY(0x88): // ADC  A,B
    OPXY(0x89): // ADC  A,C
    OPXY(0x8a): // ADC  A,D
    OPXY(0x8b): // ADC  A,E
    OPXY(0x8f): // ADC  A,A
        goto OP_ADC_R;

    OPXY(0xce): // ADC  A,n
        goto OP_ADC_imm;

    OPXY(0x8c): // ADC  A,HX
        goto OP_ADC_IxH;

    OPXY(0x8d): // ADC  A,LX
        goto OP_ADC_IxL;

    OPXY(0x8e): // ADC  A,(IX+o)
        goto OP_ADC_mIx;

    // SUB

    OPXY(0x90): // SUB  B
    OPXY(0x91): // SUB  C
    OPXY(0x92): // SUB  D
    OPXY(0x93): // SUB  E
    OPXY(0x97): // SUB  A
        goto OP_SUB_R;

    OPXY(0x94): // SUB  HX
        goto OP_SUB_IxH;

    OPXY(0x95): // SUB  LX
        goto OP_SUB_IxL;

    OPXY(0xd6): // SUB  A,n
        goto OP_SUB_imm;

    OPXY(0x96): // SUB  (IX+o)
        goto OP_SUB_mIx;

    // SBC

    OPXY(0x98): // SBC  A,B
    OPXY(0x99): // SBC  A,C
    OPXY(0x9a): // SBC  A,D
    OPXY(0x9b): // SBC  A,E
    OPXY(0x9f): // SBC  A,A
        goto OP_SBC_R;

    OPXY(0x9c): // SBC  A,HX
        goto OP_SBC_IxH;

    OPXY(0x9d): // SBC  A,LX
        goto OP_SBC_IxL;

    OPXY(0xde): // SBC  A,n
        goto OP_SBC_imm;

    OPXY(0x9e): // SBC  A,(IX+o)
        goto OP_SBC_mIx;

    // CP

    OPXY(0xb8): // CP   B
    OPXY(0xb9): // CP   C
    OPXY(0xba): // CP   D
    OPXY(0xbb): // CP   E
    OPXY(0xbf): // CP   A
        goto OP_CP_R;

    OPXY(0xbc): // CP   HX
        goto OP_CP_IxH;

    OPXY(0xbd): // CP   LX
        goto OP_CP_IxL;

    OPXY(0xfe): // CP   n
        goto OP_CP_imm;

    OPXY(0xbe): // CP   (IX+o)
        goto OP_CP_mIx;

    // AND

    OPXY(0xa0): // AND  B
    OPXY(0xa1): // AND  C
    OPXY(0xa2): // AND  D
    OPXY(0xa3): // AND  E
        goto OP_AND_R;

    OPXY(0xa7): // AND  A
        goto OP_AND_A;

    OPXY(0xa4): // AND  HX
        goto OP_AND_IxH;

    OPXY(0xa5): // AND  LX
        goto OP_AND_IxL;

    OPXY(0xe6): // AND  A,n
        goto OP_AND_imm;

    OPXY(0xa6): // AND  (IX+o)
    {
        uint8_t val;

        PRE_IO
        READ_BYTE(data->W + FETCH_BYTE_S, val)
        POST_IO
        zA = zA & val;
        zF = SZXYP[zA] | CZ80_HF;
        RET(15)
    }

    // XOR

    OPXY(0xa8): // XOR  B
    OPXY(0xa9): // XOR  C
    OPXY(0xaa): // XOR  D
    OPXY(0xab): // XOR  E
        goto OP_XOR_R;

    OPXY(0xaf): // XOR  A
        goto OP_XOR_A;

    OPXY(0xac): // XOR  HX
        goto OP_XOR_IxH;

    OPXY(0xad): // XOR  LX
        goto OP_XOR_IxL;

    OPXY(0xee): // XOR  A,n
        goto OP_XOR_imm;

    OPXY(0xae): // XOR  (IX+o)
    {
        uint8_t val;

        PRE_IO
        READ_BYTE(data->W + FETCH_BYTE_S, val)
        POST_IO
        zA = zA ^ val;
        zF = SZXYP[zA];
        RET(15)
    }

    // OR

    OPXY(0xb0): // OR   B
    OPXY(0xb1): // OR   C
    OPXY(0xb2): // OR   D
    OPXY(0xb3): // OR   E
        goto OP_OR_R;

    OPXY(0xb7): // OR   A
        goto OP_OR_A;

    OPXY(0xb4): // OR   HX
        goto OP_OR_IxH;

    OPXY(0xb5): // OR   LX
        goto OP_OR_IxL;

    OPXY(0xf6): // OR   A,n
        goto OP_OR_imm;

    OPXY(0xb6): // OR   (IX+o)
    {
        uint8_t val;

        PRE_IO
        READ_BYTE(data->W + FETCH_BYTE_S, val)
        POST_IO
        zA = zA | val;
        zF = SZXYP[zA];
        RET(15)
    }


    // MISC ARITHMETIC & CPU CONTROL

    OPXY(0x27): // DAA
        goto OP_DAA;

    OPXY(0x2f): // CPL
        goto OP_CPL;

    OPXY(0x37): // SCF
        goto OP_SCF;

    OPXY(0x3f): // CCF
        goto OP_CCF;

    OPXY(0x76): // HALT
        goto OP_HALT;

    OPXY(0xf3): // DI
        goto OP_DI;

    OPXY(0xfb): // EI
        goto OP_EI;


    // 16 BITS ARITHMETIC

    OPXY(0x03): // INC  BC
        goto OP_INC_BC;

    OPXY(0x13): // INC  DE
        goto OP_INC_DE;

    OPXY(0x23): // INC  IX
        goto OP_INC_xx;

    OPXY(0x33): // INC  SP
        goto OP_INC_SP;

    OPXY(0x0b): // DEC  BC
        goto OP_DEC_BC;

    OPXY(0x1b): // DEC  DE
        goto OP_DEC_DE;

    OPXY(0x2b): // DEC  IX
        goto OP_DEC_xx;

    OPXY(0x3b): // DEC  SP
        goto OP_DEC_SP;

    // ADD16

    OPXY(0x09): // ADD  IX,BC
        goto OP_ADD16_xx_BC;

    OPXY(0x19): // ADD  IX,DE
        goto OP_ADD16_xx_DE;

    OPXY(0x29): // ADD  IX,IX
        goto OP_ADD16_xx_xx;

    OPXY(0x39): // ADD  IX,SP
        goto OP_ADD16_xx_SP;


    // ROTATE

    OPXY(0x07): // RLCA
        goto OP_RLCA;

    OPXY(0x0f): // RRCA
        goto OP_RRCA;

    OPXY(0x17): // RLA
        goto OP_RLA;

    OPXY(0x1f): // RRA
        goto OP_RRA;


    // JUMP

    OPXY(0xc3): // JP   nn
        goto OP_JP;

    OPXY(0xe9): // JP   (IX)
        goto OP_JP_xx;


    OPXY(0xd2): // JP   NC,nn
        goto OP_JP_NC;

    OPXY(0xda): // JP   C,nn
        goto OP_JP_C;

    OPXY(0xe2): // JP   PO,nn
        goto OP_JP_PO;

    OPXY(0xea): // JP   PE,nn
        goto OP_JP_PE;

    OPXY(0xf2): // JP   P,nn
        goto OP_JP_P;

    OPXY(0xfa): // JP   M,nn
        goto OP_JP_M;

    OPXY(0xca): // JP   Z,nn
        goto OP_JP_Z;

    OPXY(0xc2): // JP   NZ,nn
        goto OP_JP_NZ;


    OPXY(0x18): // JR   n
        goto OP_JR;


    OPXY(0x38): // JR   C,n
        goto OP_JR_C;

    OPXY(0x30): // JR   NC,n
        goto OP_JR_NC;

    OPXY(0x28): // JR   Z,n
        goto OP_JR_Z;

    OPXY(0x20): // JR   NZ,n
        goto OP_JR_NZ;


    OPXY(0x10): // DJNZ n
        goto OP_DJNZ;


    // CALL & RETURN

    OPXY(0xcd): // CALL nn
        goto OP_CALL;


    OPXY(0xd4): // CALL NC,nn
        goto OP_CALL_NC;

    OPXY(0xdc): // CALL C,nn
        goto OP_CALL_C;

    OPXY(0xe4): // CALL PO,nn
        goto OP_CALL_PO;

    OPXY(0xec): // CALL PE,nn
        goto OP_CALL_PE;

    OPXY(0xf4): // CALL P,nn
        goto OP_CALL_P;

    OPXY(0xfc): // CALL M,nn
        goto OP_CALL_M;

    OPXY(0xcc): // CALL Z,nn
        goto OP_CALL_Z;

    OPXY(0xc4): // CALL NZ,nn
        goto OP_CALL_NZ;


    OPXY(0xc9): // RET
        goto OP_RET;


    OPXY(0xd0): // RET  NC
        goto OP_RET_NC;

    OPXY(0xd8): // RET  C
        goto OP_RET_C;

    OPXY(0xe0): // RET  PO
        goto OP_RET_PO;

    OPXY(0xe8): // RET  PE
        goto OP_RET_PE;

    OPXY(0xf0): // RET  P
        goto OP_RET_P;

    OPXY(0xf8): // RET  M
        goto OP_RET_M;

    OPXY(0xc0): // RET  NZ
        goto OP_RET_NZ;

    OPXY(0xc8): // RET  Z
        goto OP_RET_Z;


    OPXY(0xc7): // RST  0
    OPXY(0xcf): // RST  1
    OPXY(0xd7): // RST  2
    OPXY(0xdf): // RST  3
    OPXY(0xe7): // RST  4
    OPXY(0xef): // RST  5
    OPXY(0xf7): // RST  6
    OPXY(0xff): // RST  7
        goto OP_RST;


    // INPUT & OUTPUT

    OPXY(0xd3): // OUT  (n),A
        goto OP_OUT_mN_A;

    OPXY(0xdb): // IN   A,(n)
        goto OP_IN_A_mN;


    // PREFIXE

    OPXY(0xcb): // XYCB PREFIXE
    {
        uint16_t adr;
        uint8_t src;
        uint8_t res;

        adr = data->W + FETCH_BYTE_S;
        Opcode = FETCH_BYTE;
        #include "cz80_opxycb.inc"
    }

    OPXY(0xed): // ED PREFIXE
        goto ED_PREFIXE;

    OPXY(0xdd): // DD PREFIXE (IX)
        goto DD_PREFIXE;

    OPXY(0xfd): // FD PREFIXE (IY)
        goto FD_PREFIXE;

#if CZ80_USE_JUMPTABLE
#else
}
#endif

