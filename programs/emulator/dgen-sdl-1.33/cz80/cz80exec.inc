/********************************************************************************/
/*                                                                              */
/* CZ80 exec include source file                                                */
/* C Z80 emulator version 0.91                                                  */
/* Copyright 2004-2005 Stephane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

int FASTCALL Cz80_Exec(cz80_struc *cpu, int cycles)
{
#if CZ80_USE_JUMPTABLE
    #include "cz80jmp.inc"
#endif

    cz80_struc *CPU;
    uint16_t PC;
    int CCnt;
    uint8_t Opcode;

    CPU = cpu;
    PC = CPU->PC;

    if (CPU->Status & (CZ80_RUNNING | CZ80_DISABLE | CZ80_FAULTED))
    {
        return -(CPU->Status);
    }

#if CZ80_SIZE_OPT
    CPU->CycleToDo = CPU->CycleSup = cycles;
    CCnt = 0;
#else
    CPU->CycleToDo = CCnt = cycles;
    CPU->CycleSup = 0;
#endif
    CPU->Status |= CZ80_RUNNING;
    
#if CZ80_SIZE_OPT
Cz80_Exec_Check:
    if (CCnt <= 0) goto Cz80_Exec_End;
#else
    goto Cz80_Exec_End;
#endif

Cz80_Exec:
    {
        Opcode = FETCH_BYTE;
    Cz80_Exec_IM0:
        {
            union16 *data = pzHL;

            #include "cz80_op.inc"
        }
    }

Cz80_Exec_End:
    CHECK_INT
    if ((CCnt += CPU->CycleSup) > 0)
    {
        CPU->CycleSup = 0;
        if (!(CPU->Status & CZ80_HALTED)) goto Cz80_Exec;

        // CPU halted
        CCnt = 0;
    }

Cz80_Exec_Really_End:
    CPU->Status &= ~CZ80_RUNNING;
    CPU->PC = PC;
    
    // number of executed cycles
    CCnt = CPU->CycleToDo - CCnt;
    
    // update R register
    zR = (zR + (CCnt >> 2)) & 0x7F;

    return CCnt;
}

