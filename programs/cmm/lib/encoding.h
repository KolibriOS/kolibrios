#ifndef INCLUDE_ENCODING_H
#define INCLUDE_ENCODING_H
#print "[include <encoding.h>]\n"

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

inline fastcall void wintodos( ESI) 
{
   while (BL=ESBYTE[ESI])
   {
        if (BL>=192)
        {
             if (BL>=240) ESBYTE[ESI] = BL - 16;
             else ESBYTE[ESI] = BL - 64;
        }
        else
        {
			if (BL==178) ESBYTE[ESI] = 73;  //I
			if (BL==179) ESBYTE[ESI] = 105; //i
			if (BL==175) ESBYTE[ESI] = 244; //J
			if (BL==191) ESBYTE[ESI] = 245; //j
			if (BL==170) ESBYTE[ESI] = 242; //E
			if (BL==186) ESBYTE[ESI] = 243; //e
			if (BL==168) ESBYTE[ESI] = 240; //ð
			if (BL==184) ESBYTE[ESI] = 'e'; //e
			if (BL==180) ESBYTE[ESI] = 254; //ã
			if ((BL==147) || (BL==148) || (BL==171) || (BL==187)) ESBYTE[ESI] = 34;
			if ((BL==150) || (BL==151)) ESBYTE[ESI] = 45;
        }
        ESI++;
   }
}


byte mas[66] = "î ¡æ¤¥ä£å¨©ª«¬­®¯ïàáâã¦¢ìë§èíéçêž€–„…”ƒ•ˆ‰Š‹ŒŽŸ‘’“†‚œ›‡˜™—š";
inline fastcall void koitodos( EDI)
{
	while (BL=ESBYTE[EDI])
	{	
		if (BL >= 0xC0)
		{
			BL -= 0xC0;
			ESBYTE[EDI] = mas[BL];
		}
		//if (ESBYTE[EDI]=='\244') ESBYTE[EDI]='i';
		EDI++;
	}
}

//Asper, lev
//uncomplete
inline fastcall void utf8rutodos( ESI)
{
    EDI=ESI;
  while (BL=ESBYTE[ESI])
  {
    if (BL == 0xD0) || (BL == 0xD1) EDI--;
        else if (BL == 0x81) && (ESBYTE[ESI-1]==0xD0) ESBYTE[EDI] = 0xF0; //ø
        else if (BL == 0x91) && (ESBYTE[ESI-1]==0xD1) ESBYTE[EDI] = 0xF1; //ì
        //0xE2 0x80 - ñãðóïïèðîâàòü
        else if (BL == 0xE2) && (ESBYTE[ESI+1]==0x80)
        switch (ESBYTE[ESI+2])
        {
        case 0x93: //long defis
        case 0x94:
        {
          ESBYTE[EDI] = '-';
          ESI+=2;
            break;
        }
        case 0xA2: //central point
        {
          ESBYTE[EDI] = '*';
          ESI+=2;
            break;
        }
        case 0xA6: //ìíîãîòî÷èå
        {
          ESBYTE[EDI] = ESBYTE[EDI+1] = ESBYTE[EDI+2] = '.';
          EDI+=2;
          ESI+=2;
          break;
          }
        }

        else if (BL == 0xC2) //òàáëèöó ïåðåêîäèðîâîê?
          switch(ESBYTE[ESI+1]) {
            case 0xAB: //"
            case 0xBB: //"
              {
                ESBYTE[EDI] = '\"';
                ESI++;
                break;
              }
            case 0xB7: // _
              {
                ESBYTE[EDI] = '_';
                ESI++;
                break;
              }
            case 0xA0: // Alt+160 - íåðàçáèâàþùèé ïðîáåë
              {
                ESBYTE[EDI] = ' ';
                ESI++;
                break;
              }
            case 0xB0: // ãðàäóñ
              {
                ESBYTE[EDI] = '\29';
                ESI++;
                break;
              }
			case 0xA9: // (c) --- âûëåò Î_î
			{
			  ESBYTE[EDI] = 'c';
			  ESI++;
			  break;
			}
			case 0xAE: // (r)
			{
			  ESBYTE[EDI] = 'r';
			  ESI++;
			  break;
			} 
     }

        else if (BL >= 0x90) && (BL <= 0xAF)
    {
      BL -= 0x10;
      ESBYTE[EDI] = BL;
    }
        else if (BL >= 0x80) && (BL <= 0x8F)
    {
      BL += 0x60;
      ESBYTE[EDI] = BL;
    }
      else if (BL >= 0xB0) && (BL <= 0xBF)
    {
     BL -= 0x10;
     ESBYTE[EDI] = BL;
    }
    else ESBYTE[EDI] = BL;
    ESI++;
    EDI++;
  }
  while (EDI<ESI)
  {
    ESBYTE[EDI] = ' ';
    EDI++;
   }
}

//------------------------------------------------------------------------------

:dword Hex2Symb(char* htmlcolor)
{
  dword j=0, symbol=0;
  char ch=0x00;
  for (;j<2;j++)
  {
    ch=ESBYTE[htmlcolor+j];
    if (ch==0x0d) || (ch=='\9') RETURN 0;
    if ((ch>='0') && (ch<='9')) ch -= '0';
    if ((ch>='A') && (ch<='F')) ch -= 'A'-10;
    if ((ch>='a') && (ch<='f')) ch -= 'a'-10;
    symbol = symbol*0x10 + ch;
  }
  wintodos(#symbol);
  AL=symbol;
}


#endif