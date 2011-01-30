/*=====================================================================
  z80.c -> Main File related to the Z80 emulation code.

  Please read documentation files to know how this works :)

  Thanks go to Marat Fayzullin (read z80.h for more info), Raúl Gomez
  (check his great R80 Spectrum emulator!), Philip Kendall (some code
  of this emulator, such as the flags lookup tabled are from his fuse
  Spectrum emulator) and more people I forget to name here ...

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Copyright (c) 2000 Santiago Romero Iglesias.
 Email: sromero@escomposlinux.org
 ======================================================================*/

#include "z80.h"
#include "tables.h"


/* RAM variable, debug toggle variable, pressed key and
   row variables for keyboard emulation                   */
extern byte *RAM;
extern int debug, main_tecla, scanl;

extern int fila[5][5];

//extern char *tapfile;
//extern FILE *tapfile;
extern char *tfont;

#include "macros.c"


/*====================================================================
  void Z80Reset( Z80Regs *regs, int cycles )

  This function simulates a z80 reset by setting the registers
  to the values they are supposed to take on a real z80 reset.
  You must pass it the Z80 register structure and the number
  of cycles required to check for interrupts and do special
  hardware checking/updating.
 ===================================================================*/
void Z80Reset( Z80Regs *regs, int int_cycles )
{
   /* reset PC and the rest of main registers: */
   regs->PC.W = regs->R.W = 0x0000;

   regs->AF.W = regs->BC.W = regs->DE.W = regs->HL.W =
   regs->AFs.W = regs->BCs.W = regs->DEs.W = regs->HLs.W =
   regs->IX.W = regs->IY.W = 0x0000;

   /* Make the stack point to $F000 */
   regs->SP.W = 0xF000;

   /* reset variables to their default values */
   regs->I = 0x00;
   regs->IFF1 = regs->IFF2 = regs->IM = regs->halted = 0x00;
   regs->ICount = regs->IPeriod = int_cycles;

   regs->IRequest = INT_NOINT;
   regs->we_are_on_ddfd = regs->dobreak = regs->BorderColor = 0;

//#ifdef _DEBUG_
   regs->DecodingErrors = 1;
//#endif

}


/*====================================================================
  word Z80Run( Z80Regs *regs, int numopcodes )

  This function does the whole Z80 simulation. It consists on a
  for(;;) loop (as stated on Marat's Fayzullin HOWTO -How to
  Write a Computer Emulator-) which fetchs the next opcode,
  interprets it (using a switch statement) and then it's
  executed in the right CASE: of that switch. I've put the different
  case statements into C files included here with #include to
  make this more readable (and programming easier! :).

  This function will change regs->ICount register and will execute
  an interrupt when it reaches 0 (or <0). You can then do anything
  related to your machine emulation here, using the Z80Hardware()
  function. This function must be filled by yourself: put there
  the code related to the emulated machine hardware, such as
  screen redrawing, sound playing and so on. This functions can
  return an special value to make Z80Run stop the emulation (and
  return to the caller): that's INT_QUIT. If there is time to
  execute an interrupt, please return INT_IRQ or INT_NMI. Return
  INT_NOINT if there is no time for an interrupt :) .

  Z80Execute() will change PC and all the z80 registers acording
  to the executed opcode, and those values will be returned when
  a INT_QUIT is received.

  Pass as numcycles the number of clock cycle you want to execute
  z80 opcodes for or < 0 (negative) to execute "infinite" opcodes.
 ===================================================================*/
word Z80Run( Z80Regs *regs, int numcycles )
{
   /* opcode and temp variables */
   register byte opcode;
   eword tmpreg, ops, mread, tmpreg2;
   unsigned long tempdword;
   register int loop;
   unsigned short tempword;

   /* emulate <numcycles> cycles */
   loop = (regs->ICount - numcycles);

   /* this is the emulation main loop */
   while( regs->ICount > loop )
   {
   #ifdef DEBUG
      /* test if we have reached the trap address */
      if( regs->PC.W == regs->TrapAddress && regs->dobreak != 0 )
         return(regs->PC.W);
   #endif

     if( regs->halted == 1 )
     { r_PC--; AddCycles(4); }

       /* read the opcode from memory (pointed by PC) */
       opcode = Z80ReadMem(regs->PC.W);
       regs->PC.W++;

       /* increment the R register and decode the instruction */
       AddR(1);
       switch(opcode)
       {
             #include "opcodes.c"
          case PREFIX_CB:
             AddR(1);
             #include "op_cb.c"
             break;
          case PREFIX_ED:
             AddR(1);
             #include "op_ed.c"
             break;
          case PREFIX_DD:
          case PREFIX_FD:
             AddR(1);
             if( opcode == PREFIX_DD )
             {
               #define REGISTER regs->IX
               regs->we_are_on_ddfd = WE_ARE_ON_DD;
               #include "op_dd_fd.c"
               #undef  REGISTER
             }
             else
             {
               #define REGISTER regs->IY
               regs->we_are_on_ddfd = WE_ARE_ON_FD;
               #include "op_dd_fd.c"
               #undef  REGISTER
             }
             regs->we_are_on_ddfd = 0;
             break;
       }

     /* patch ROM loading routine */
     // address contributed by Ignacio Burgueño :)
//     if( r_PC == 0x0569 )       
     if( r_PC >= 0x0556 && r_PC <= 0x056c )
          Z80Patch( regs );

     /* check if it's time to do other hardware emulation */
     if( regs->ICount <= 0 )
     {
          tmpreg.W = Z80Hardware(regs);
          regs->ICount += regs->IPeriod;
          loop = regs->ICount + loop;

          /* check if we must exit the emulation or there is an INT */
          if( tmpreg.W == INT_QUIT )
          return( regs->PC.W );
          if( tmpreg.W != INT_NOINT )
             Z80Interrupt( regs, tmpreg.W );
     }
   }

   return(regs->PC.W);
}



/*====================================================================
  void Z80Interrupt( Z80Regs *regs, word ivec )
 ===================================================================*/
void Z80Interrupt( Z80Regs *regs, word ivec )
{
   word intaddress;

   /* unhalt the computer */
   if( regs->halted == 1 )
      regs->halted = 0;
   
   if( regs->IFF1 )
   {
      PUSH(PC);
      regs->IFF1 = 0;
      switch(regs->IM)
      {
        case 0: r_PC = 0x0038; AddCycles(12); break;
        case 1: r_PC = 0x0038; AddCycles(13); break;
        case 2: intaddress = (((regs->I & 0xFF)<<8) | 0xFF);
                regs->PC.B.l = Z80ReadMem(intaddress);
                regs->PC.B.h = Z80ReadMem(intaddress+1);
                AddCycles(19);
                break;
      }

   }

}


/*====================================================================
  word  Z80Hardware(register Z80Regs *regs)

  Do here your emulated machine hardware emulation. Read Z80Execute()
  to know about how to quit emulation and generate interrupts.
 ===================================================================*/
word  Z80Hardware( register Z80Regs *regs )
{
  if( 
	debug != 1  // && scanl >= 224 
	)
  {
      ;
  }
  return( INT_IRQ );
}


/*====================================================================
  void Z80Patch( register Z80Regs *regs )

  Write here your patches to some z80 opcodes that are quite related
  to the emulated machines (i.e. maybe accessing to the I/O ports
  and so on), such as ED_FE opcode:

     case ED_FE:     Z80Patch(regs);
                     break;

  This allows "BIOS" patching (cassette loading, keyboard ...).
 ===================================================================*/
void Z80Patch( register Z80Regs *regs )
{

///!!!   if( tapfile != NULL )
///!!!   {
///!!!        LoadTAP( regs, tapfile );
///!!!        POP(PC);
///!!!   }

   /*
        if( strlen(tapfile) != 0 )
        {
           if( LoadTapFile( regs, tapfile ) )
           {  POP(PC);  }
        }
        else
        {
           FileMenu( tfont, 3, tapfile );
           if( strlen(tapfile) != 0 )
             if( LoadTapFile( regs, tapfile ) )
                {  POP(PC);  }
        }
   */
}


/*====================================================================
  byte Z80Debug( register Z80Regs *regs )

  This function is written for debugging purposes (it's supposed to
  be a debugger by itself!). It will debug a single opcode, given
  by the current PC address.

  Return DEBUG_OK to state success and DEBUG_QUIT to quit emulation.
 ===================================================================*/
byte Z80Debug( register Z80Regs *regs )
{
   return( DEBUG_QUIT );
}



/*====================================================================
  byte Z80MemRead( register word address )

  This function reads from the given memory address. It is not inlined,
  and it's written for debugging purposes.
 ===================================================================*/
byte Z80MemRead( register word address, Z80Regs *regs )
{
  return(Z80ReadMem(address));
}


/*====================================================================
  void Z80MemWrite( register word address, register byte value )

  This function writes on memory the given value. It is not inlined,
  ands it's written for debugging purposes.
 ===================================================================*/
void Z80MemWrite( register word address, register byte value, Z80Regs *regs )
{
  Z80WriteMem( address, value, regs );
}


/*====================================================================
  byte Z80InPort( register word port )

  This function reads from the given I/O port. It is not inlined,
  and it's written for debugging purposes.
 ===================================================================*/
byte Z80InPort( register word port )
{
  int porth;
  int code = 0xFF;

  porth = port >> 8;

  if (!(porth & 0x01)) code &= fila[4][1];
  if (!(porth & 0x02)) code &= fila[3][1];
  if (!(porth & 0x04)) code &= fila[2][1];
  if (!(porth & 0x08)) code &= fila[1][1];
  if (!(porth & 0x10)) code &= fila[1][2];
  if (!(porth & 0x20)) code &= fila[2][2];
  if (!(porth & 0x40)) code &= fila[3][2];
  if (!(porth & 0x80)) code &= fila[4][2];

  /*
     issue 2 emulation, thx to Raul Gomez!!!!!
     I should implement this also:
     if( !ear_on && mic_on )
        code &= 0xbf;
     where earon = bit 4 of the last OUT to the 0xFE port
     and   micon = bit 3 of the last OUT to the 0xFE port
  */
  code &= 0xbf;

  if( (port & 0xFF) == 0xFF )
  {
      if( (rand() % 10) > 7 ) return(0xff);
      else return( rand()%0xFF );
  }

  return( code );
}


/*====================================================================
  void Z80OutPort( register word port, register byte value )

  This function outs a value to a given I/O port. It is not inlined,
  and it's written for debugging purposes.
 ===================================================================*/
void Z80OutPort( register Z80Regs *regs,
                 register word port, register byte value )
{
    /* change border colour */
    if( ! (port & 0x01) )
        regs->BorderColor = (value & 0x07);
}



/*====================================================================
   static void Z80FlagTables ( void );

   Creates a look-up table for future flag setting...
   Taken from fuse's sources. Thanks to Philip Kendall.
 ===================================================================*/
void Z80FlagTables(void)
{
  int i,j,k;
  byte parity;

  for(i=0;i<0x100;i++) {
    sz53_table[i]= i & ( FLAG_3 | FLAG_5 | FLAG_S );
    j=i; parity=0;
    for(k=0;k<8;k++) { parity ^= j & 1; j >>=1; }
    parity_table[i]= ( parity ? 0 : FLAG_P );
    sz53p_table[i] = sz53_table[i] | parity_table[i];
  }

  sz53_table[0]  |= FLAG_Z;
  sz53p_table[0] |= FLAG_Z;
}

