
inline fastcall void wintodos( ESI) 
{
   while (BL=ESBYTE[ESI])
   {
        IF (BL>=192)
        {
             IF (BL>=240) ESBYTE[ESI] = BL - 16;
             ELSE ESBYTE[ESI] = BL - 64;
        }
        ELSE
        {
			IF (BL==178) ESBYTE[ESI] = 73;  //I
			IF (BL==179) ESBYTE[ESI] = 105; //i
			IF (BL==175) ESBYTE[ESI] = 244; //J
			IF (BL==191) ESBYTE[ESI] = 245; //j
			IF (BL==170) ESBYTE[ESI] = 242; //E
			IF (BL==186) ESBYTE[ESI] = 243; //e
			IF (BL==168) ESBYTE[ESI] = 240; //ð
			IF (BL==184) ESBYTE[ESI] = 'e'; //e
			IF (BL==180) ESBYTE[ESI] = 254; //ã
			IF ((BL==147) || (BL==148) || (BL==171) || (BL==187)) ESBYTE[ESI] = 34;
			IF ((BL==150) || (BL==151)) ESBYTE[ESI] = 45;
        }
        ESI++;
   }
}


byte mas[66] = "î ¡æ¤¥ä£å¨©ª«¬­®¯ïàáâã¦¢ìë§èíéçêž€–„…”ƒ•ˆ‰Š‹ŒŽŸ‘’“†‚œ›‡˜™—š";
inline fastcall void koitodos( EDI)
{
	WHILE (BL=ESBYTE[EDI])
	{	
		IF (BL >= 0xC0)
		{
			BL -= 0xC0;
			ESBYTE[EDI] = mas[BL];
		}
		//IF (ESBYTE[EDI]=='\244') ESBYTE[EDI]='i';
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
    IF (BL == 0xD0) || (BL == 0xD1) EDI--;
        else IF (BL == 0x81) && (ESBYTE[ESI-1]==0xD0) ESBYTE[EDI] = 0xF0; //ø
        else IF (BL == 0x91) && (ESBYTE[ESI-1]==0xD1) ESBYTE[EDI] = 0xF1; //ì
        //0xE2 0x80 - ñãðóïïèðîâàòü
        else IF (BL == 0xE2) && (ESBYTE[ESI+1]==0x80)
        SWITCH (ESBYTE[ESI+2])
        {
        case 0x93: //long defis
        CASE 0x94:
        {
          ESBYTE[EDI] = '-';
          ESI+=2;
            BREAK;
        }
        CASE 0xA2: //central point
        {
          ESBYTE[EDI] = '*';
          ESI+=2;
            BREAK;
        }
        CASE 0xA6: //ìíîãîòî÷èå
        {
          ESBYTE[EDI] = ESBYTE[EDI+1] = ESBYTE[EDI+2] = '.';
          EDI+=2;
          ESI+=2;
          break;
          }
        }

        else IF (BL == 0xC2) //òàáëèöó ïåðåêîäèðîâîê?
          SWITCH(ESBYTE[ESI+1]) {
            case 0xAB: //"
            CASE 0xBB: //"
              {
                ESBYTE[EDI] = '\"';
                ESI++;
                BREAK;
              }
            CASE 0xB7: // _
              {
                ESBYTE[EDI] = '_';
                ESI++;
                BREAK;
              }
            CASE 0xA0: // Alt+160 - íåðàçáèâàþùèé ïðîáåë
              {
                ESBYTE[EDI] = ' ';
                ESI++;
                BREAK;
              }
            CASE 0xB0: // ãðàäóñ
              {
                ESBYTE[EDI] = '\29';
                ESI++;
                BREAK;
              }
			CASE 0xA9: // (c) --- âûëåò Î_î
			{
			  ESBYTE[EDI] = 'c';
			  //ESBYTE[EDI] = '(';
			  //ESBYTE[EDI+1] = 'c';
			  //ESBYTE[EDI+2] = ')';
			  //EDI+=2;
			  ESI++;
			  BREAK;
			}
			CASE 0xAE: // (r)
			{
			  ESBYTE[EDI] = 'r';
			  //ESBYTE[EDI] = '(';
			  //ESBYTE[EDI+1] = 'r';
			  //ESBYTE[EDI+2] = ')';
			  //EDI+=2;
			  ESI++;
			  BREAK;
			} 
     }

        ELSE IF (BL >= 0x90) && (BL <= 0xAF)
    {
      BL -= 0x10;
      ESBYTE[EDI] = BL;
    }
        ELSE IF (BL >= 0x80) && (BL <= 0x8F)
    {
      BL += 0x60;
      ESBYTE[EDI] = BL;
    }
      ELSE IF (BL >= 0xB0) && (BL <= 0xBF)
    {
     BL -= 0x10;
     ESBYTE[EDI] = BL;
    }
    ELSE ESBYTE[EDI] = BL;
    ESI++;
    EDI++;
  }
  WHILE (EDI<ESI)
  {
    ESBYTE[EDI] = ' ';
    EDI++;
   }
}

//------------------------------------------------------------------------------

dword Hex2Symb(char* htmlcolor)
{
  dword j=0, symbol=0;
  char ch=0x00;
  FOR (;j<2;j++)
  {
    ch=ESBYTE[htmlcolor+j];
    IF (ch==0x0d) || (ch=='\9') RETURN '';
    IF ((ch>='0') && (ch<='9')) ch -= '0';
    IF ((ch>='A') && (ch<='F')) ch -= 'A'-10;
    IF ((ch>='a') && (ch<='f')) ch -= 'a'-10;
    symbol = symbol*0x10 + ch;
  }
  wintodos(#symbol);
  AL=symbol;
}

/*
int hex2char(dword c)
{
  if (c <=9)
    return (c+48);

  return (c - 10 + 'a');
}*/
