/********************************************************************************/
/*                                                                              */
/* CZ80 macro file                                                              */
/* C Z80 emulator version 0.91                                                  */
/* Copyright 2004-2005 Stephane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

#if CZ80_USE_JUMPTABLE
#define _SSOP(A,B) A##B
#define OP(A) _SSOP(OP,A)
#define OPCB(A) _SSOP(OPCB,A)
#define OPED(A) _SSOP(OPED,A)
#define OPXY(A) _SSOP(OPXY,A)
#define OPXYCB(A) _SSOP(OPXYCB,A)
#else
#define OP(A) case A
#define OPCB(A) case A
#define OPED(A) case A
#define OPXY(A) case A
#define OPXYCB(A) case A
#endif

#define GET_BYTE                \
	(((uint8_t *)CPU->BasePC)[PC])

#define GET_BYTE_S              \
	(((int8_t *)CPU->BasePC)[PC])

#define GET_WORD                \
	(((uint8_t *)CPU->BasePC)[PC] |				\
	 (((uint8_t *)CPU->BasePC)[((PC + 1) & 0xffff)] << 8))

#define FETCH_BYTE              \
	(((uint8_t *)CPU->BasePC)[PC++])

#define FETCH_BYTE_S            \
	(((int8_t *)CPU->BasePC)[PC++])

#define FETCH_WORD(A)           \
    A = GET_WORD;               \
    PC += 2;

#if CZ80_SIZE_OPT
    #define RET(A)              \
        CCnt -= A;              \
        goto Cz80_Exec_Check;
#else
    #define RET(A)              \
        if ((CCnt -= A) <= 0) goto Cz80_Exec_End;  \
        goto Cz80_Exec;
#endif

#define SET_PC(A)               \
    CPU->BasePC = (uintptr_t) CPU->Fetch[(A) >> CZ80_FETCH_SFT];  \
    PC = ((A) & 0xffff);

#define PRE_IO                  \
    CPU->CycleIO = CCnt;

#define POST_IO                 \
    CCnt = CPU->CycleIO;

#define READ_BYTE(A, D)                 \
    D = CPU->Read_Byte(CPU->ctx, (A));

#if CZ80_USE_WORD_HANDLER
#define READ_WORD(A, D)                 \
    D = CPU->Read_Word(CPU->ctx, (A));
#define READ_WORD_LE(A, D) READ_WORD(A, D)
#elif CZ80_LITTLE_ENDIAN
#define READ_WORD(A, D)                 \
    D = CPU->Read_Byte(CPU->ctx, (A)) | (CPU->Read_Byte(CPU->ctx, ((A) + 1)) << 8);
#define READ_WORD_LE(A, D) READ_WORD(A, D)
#else
#define READ_WORD(A, D)                 \
    D = (CPU->Read_Byte(CPU->ctx, (A)) << 8) | CPU->Read_Byte(CPU->ctx, ((A) + 1));
#define READ_WORD_LE(A, D)              \
    D = CPU->Read_Byte(CPU->ctx, (A)) | (CPU->Read_Byte(CPU->ctx, ((A) + 1)) << 8);
#endif

#define READSX_BYTE(A, D)               \
    D = CPU->Read_Byte(CPU->ctx, (A));

#define WRITE_BYTE(A, D)                \
    CPU->Write_Byte(CPU->ctx, (A), (D));

#if CZ80_USE_WORD_HANDLER
#define WRITE_WORD(A, D)                \
    CPU->Write_Word(CPU->ctx, (A), (D));
#define WRITE_WORD_LE(A, D) WRITE_WORD(A, D);
#elif CZ80_LITTLE_ENDIAN
#define WRITE_WORD(A, D)				\
    CPU->Write_Byte(CPU->ctx, (A), (D));		\
    CPU->Write_Byte(CPU->ctx, ((A) + 1), ((D) >> 8));
#define WRITE_WORD_LE(A, D) WRITE_WORD(A, D);
#else
#define WRITE_WORD(A, D)			\
    CPU->Write_Byte(CPU->ctx, (A), ((D) >> 8));	\
    CPU->Write_Byte(CPU->ctx, ((A) + 1), (D));
#define WRITE_WORD_LE(A, D)				\
    CPU->Write_Byte(CPU->ctx, (A), (D));		\
    CPU->Write_Byte(CPU->ctx, ((A) + 1), ((D) >> 8));
#endif

#define PUSH_16(A)              \
    {                           \
        uint16_t sp;            \
                                \
        zSP -= 2;               \
        sp = zSP;               \
        WRITE_WORD_LE(sp, A);   \
    }

#define POP_16(A)           \
    {                       \
        uint16_t sp;        \
                            \
        sp = zSP;           \
        READ_WORD_LE(sp, A);\
        zSP = sp + 2;       \
    }

#define IN(A, D)            \
    D = CPU->IN_Port(CPU->ctx, (A));

#define OUT(A, D)           \
    CPU->OUT_Port(CPU->ctx, (A), (D));

#define CHECK_INT                                           \
    if (CPU->Status & (zIFF1 | CZ80_HAS_NMI))               \
    {                                                       \
        uint16_t newPC;                                     \
                                                            \
        if (CPU->Status & CZ80_HAS_NMI)                     \
        {                                                   \
            /* NMI */                                       \
            CPU->Status &= ~(CZ80_HALTED | CZ80_HAS_NMI);   \
            zIFF1 = 0;                                      \
            newPC = 0x66;                                   \
        }                                                   \
        else                                                \
        {                                                   \
            /* MI */                                        \
            CPU->Status &= ~(CZ80_HALTED | CZ80_HAS_INT);   \
            zIFF= 0;                                        \
                                                            \
            if (zIM == 1) newPC = 0x38;                     \
            else                                            \
            {                                               \
                uint16_t adr;                               \
                                                            \
                Opcode = CPU->Interrupt_Ack(CPU->ctx, CPU->IntVect) & 0xFF; \
                if (zIM == 0) goto Cz80_Exec_IM0;           \
                                                            \
                adr = Opcode | (zI << 8);                   \
                READ_WORD(adr, newPC)                       \
                CCnt -= 8;                                  \
            }                                               \
        }                                                   \
                                                            \
        {                                                   \
            uint16_t src = PC;                              \
                                                            \
            PUSH_16(src)                                    \
            SET_PC(newPC)                                   \
            CCnt -= 11;                                     \
        }                                                   \
    }
 
