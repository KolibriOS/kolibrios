

:static int __wsprintfproc;
:static int cdecl __wsprintf(int buff,mask,...){
  char  buffer[MAX_PATH];
  char  bufnum[32];
  int   bufferlen;
  int   param;
  int   length;
  int   pad;

  struct _wsprintf_mf{
    int   precision;
    int   width;
    int   minus ;
    int   plus  ;
    int   diez  ;
    int   zero  ;
    byte upcase ;
    byte _signed;//
  }mf;

  $push EBX,ESI,EDI;
  param=#mask[sizeof(mask)];
  length=0;
  EDX=mask;
  EBX=buff;
  for(;;){
    mf=0;
    mf.precision=0x7FFF;
    REPEATSMALL:
    if(!DSCHAR[EDX]){
      AL=0;
      $call __wsprintfproc;
      $pop EDI,ESI,EBX;
      return length;
    }
    if(DSCHAR[EDX]!='%'){
      WRITECHAR: AL=DSCHAR[EDX];$call __wsprintfproc; EDX++;GOTO REPEATSMALL;
    }

    GETMASK:
    EDX++;
    SWITCH(DSCHAR[EDX]){
      CASE '-': mf.minus++;goto GETMASK;
      CASE '+': mf.plus ++;goto GETMASK;
      CASE '#': mf.diez ++;goto GETMASK;
      CASE '0': mf.zero ++;EDX++;
    }

    WHILE(DSCHAR[EDX]>='0')&&(DSCHAR[EDX]<='9'){
      $push EDX;mf.width=mf.width*10+DSCHAR[EDX]-'0';$pop EDX;EDX++;
    }
    if(DSCHAR[EDX]=='.'){
      mf.precision=0;
  GETPRECISION:
      EDX++;
      if(DSCHAR[EDX]>='0')
      &&(DSCHAR[EDX]<='9'){
        $push EDX;mf.precision=mf.precision*10+DSCHAR[EDX]-'0';$pop EDX;
        GOTO GETPRECISION;
      }
    }
    if(DSCHAR[EDX]=='l'){
      EDX++;
    }
    if(DSCHAR[EDX]=='h'){
      EDX++;
    }
    if(DSCHAR[EDX]<'a'){mf.upcase=1;}
    SWITCH(DSCHAR[EDX]|0x20){
      CASE 'c': buffer=DSCHAR[param];bufferlen=1;goto FORMAT;
      CASE 's': if(!DSDWORD[param])DSDWORD[param]="(null)";//??????
                FOR(bufferlen=0;bufferlen<mf.precision;bufferlen++){
                  AL=DSCHAR[DSDWORD[param]+bufferlen];
                  if(!AL)BREAK;
                  buffer[bufferlen]=AL;
                }
                goto FORMAT;
    }

    //с символьными покончили. теперь числовые
    if(mf.precision==0x7FFF){mf.precision=mf.width;}
    if(!mf.precision)mf.precision=1;
    if(DSCHAR[EDX]|0x20=='x'){
      $push EDX;
      EDI=0;//cnt
      EDX=DSDWORD[param];
      WHILE(EDX){
        AL=DL&0xF+0x30;if(AL>'9'){AL+='A'-'9'-1;if(!mf.upcase)AL|=0x20;}
        bufnum[EDI]=AL;EDI++;EDX>>=4;
      }
      $pop EDX;
      WHILE(EDI<mf.precision){bufnum[EDI]='0';EDI++;}
      if(mf.diez){bufnum[EDI]=DSCHAR[EDX];EDI++;bufnum[EDI]='0';EDI++;}
      FOR(bufferlen=0;EDI;bufferlen++){
        EDI--;buffer[bufferlen]=bufnum[EDI];
      }
      goto FORMAT;
    }
    switch(DSCHAR[EDX]){
      CASE 'i':
      CASE 'd': $push EDX;EDX=DSDWORD[param];
                if(int EDX<0){-EDX;mf._signed++;}
                GOTO WRITEINT;
      CASE 'u': $push EDX;mf.plus=0;EDX=DSDWORD[param];
  WRITEINT:
                EDI=0;//cnt
                WHILE(EDX){
                  EAX=0;EDX><EAX;EAX/=10;EDX><EAX;bufnum[EDI]=AL+'0';EDI++;
                }
                WHILE(EDI<mf.precision){bufnum[EDI]='0';EDI++;}
                if(mf._signed){bufnum[EDI]='-';EDI++;}
                ELSE
                if(mf.plus){bufnum[EDI]='+';EDI++;}
                FOR(bufferlen=0;EDI;bufferlen++){
                  EDI--;buffer[bufferlen]=bufnum[EDI];
                }
                $pop EDX;
            FORMAT:
                EDX++;
                param+=sizeof(int);
                pad=mf.width-bufferlen;if(pad<0)pad=0;
                if(!mf.minus)$call FRM;
                FOR(EDI=0;EDI<bufferlen;EDI++){AL=buffer[EDI];$call __wsprintfproc;}
                if(mf.minus)$call FRM;
                BREAK;
      default:
                goto WRITECHAR;
    }
  }//for(;;)
  FRM: WHILE(pad){AL=' ';$call __wsprintfproc;pad--;}$ret;
}

:int cdecl wsprintf(...)inline{
  __wsprintfproc=#WSP;
  goto __wsprintf;
  WSP: DSCHAR[EBX]=AL;EBX++;$ret;
}

:void FillMemory(int dest,len,byt){
  EDI = dest;
  ECX = len;
  AL = byt;
  $cld $rep $stosb
}

//#define CopyMemory memmov
:void CopyMemory(int dest,src,len){
  EDI = dest;
  ESI = src;
  ECX = len;
  $cld $rep $movsb
}