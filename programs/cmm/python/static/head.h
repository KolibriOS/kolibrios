#pragma option OST
#pragma option ON
#pragma option cri-
#pragma option -CPA
#initallvar 0
#jumptomain FALSE
 
#startaddress 65536
 
#code32 TRUE

#define PY_NONE 0
#define PY_BOOL 1
#define PY_INT 2
#define PY_FLT 3
#define PY_STR 4
#define PY_FNC 6
#define PY_CPL 7
#define PY_LST 8
#define PY_TPL 9
#define PY_RNG 10
#define PY_BTS 11
#define PY_BTA 12
#define PY_MVW 13
#define PY_SET 14
#define PY_FST 15
#define PY_DCT 16
#define PY_CLS 17
#define PY_MDL 18

#define PY_STD_FNC 30

#define PY_NA_STR 32
#define PY_SYM_STR 33


#define PY_ADD 0
#define PY_POW 1
#define PY_MUL 2
#define PY_SUB 3
#define PY_DIV 4
#define PY_MOD 5
#define PY_XOR 6
#define PY_AND 7
#define PY__OR 8
#define PY_LSH 9
#define PY_RSH 10
#define PY_FDV 11
#define PY_TDV 12

#define MEMBUF 0xF
#define MEMARR 0xF
 
char   os_name[8]   = {'M','E','N','U','E','T','0','1'};
dword  os_version   = 0x00000001;
dword  start_addr   = #______INIT______;
dword  final_addr   = #______STOP______+32;
dword  alloc_mem    = 160000;
dword  x86esp_reg   = 160000;
dword  I_Param      = #param;
dword  I_Path       = #program_path;
char param[4096]={0};
char program_path[4096]={0};

:dword arraySymbolHEX = "0123456789ABCDEF";
:dword libPath = "/sys/lib/";
:dword HASH = 0;
:dword TEMP = 0;
:dword RDATA[30] = {0};

// global variable
:dword X = 0;
:dword Y = 0;
:dword Z = 0;
:dword A = 0;
:dword B = 0;
:dword C = 0;
:dword D = 0;
:dword E = 0;
:dword F = 0;

:dword beginStack = 0;

:dword COUNT_CONST = 0;

:byte std__STRING = 0;
:byte std__INTEGER = 1;

:byte S = 0;
//--------------

:struct f70{
        dword   func;
        dword   param1;
        dword   param2;
        dword   param3;
        dword   param4;
        char    rezerv;
        dword   name;
};
