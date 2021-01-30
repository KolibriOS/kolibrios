//////////////////////////////////////////
//
// Graphics .H file for Japanese version
// Recreated from images and used defines
//
//////////////////////////////////////////

typedef enum {
    // Lump Start
    H_HELP1PIC = 3,              // 3
    H_HELP2PIC,                  // 4
    H_HELP3PIC,                  // 5
    H_HELP4PIC,                  // 6
    H_HELP5PIC,                  // 7
    H_HELP6PIC,                  // 8
    H_HELP7PIC,                  // 9
    H_HELP8PIC,                  // 10
    H_HELP9PIC,                  // 11
    H_HELP10PIC,                 // 12

    // Lump Start
    C_OPTIONSPIC,                // 13
    C_CURSOR1PIC,                // 14
    C_CURSOR2PIC,                // 15
    C_NOTSELECTEDPIC,            // 16
    C_SELECTEDPIC,               // 17
    C_MOUSELBACKPIC,             // 18
    C_BABYMODEPIC,               // 19
    C_EASYPIC,                   // 20
    C_NORMALPIC,                 // 21
    C_HARDPIC,                   // 22
    C_LOADSAVEDISKPIC,           // 23
    C_DISKLOADING1PIC,           // 24
    C_DISKLOADING2PIC,           // 25
    C_CONTROLPIC,                // 26
    C_LOADGAMEPIC,               // 27
    C_SAVEGAMEPIC,               // 28
    C_EPISODE1PIC,               // 29
    C_EPISODE2PIC,               // 30
    C_EPISODE3PIC,               // 31
    C_EPISODE4PIC,               // 32
    C_EPISODE5PIC,               // 33
    C_EPISODE6PIC,               // 34
    C_CODEPIC,                   // 35
    C_TIMECODEPIC,               // 36
    C_LEVELPIC,                  // 37
    C_NAMEPIC,                   // 38
    C_SCOREPIC,                  // 39
    C_JOY1PIC,                   // 40
    C_JOY2PIC,                   // 41

    C_QUITMSGPIC,                // 42
    C_JAPQUITPIC,                // 43
    C_UNUSED_LOADING,            // 44
    C_JAPNEWGAMEPIC,             // 45
    C_JAPSAVEOVERPIC,            // 46

    C_MSCORESPIC,                // 47
    C_MENDGAMEPIC,               // 48
    C_MRETDEMOPIC,               // 49
    C_MRETGAMEPIC,               // 50
    C_INTERMISSIONPIC,           // 51
    C_LETSSEEPIC,                // 52
    C_ENDRATIOSPIC,              // 53

    C_ENDGAME1APIC,              // 54
    C_ENDGAME1BPIC,              // 55
    C_ENDGAME2APIC,              // 56
    C_ENDGAME2BPIC,              // 57
    C_ENDGAME3APIC,              // 58
    C_ENDGAME3BPIC,              // 59
    C_ENDGAME4APIC,              // 60
    C_ENDGAME4BPIC,              // 61
    C_ENDGAME5APIC,              // 62
    C_ENDGAME5BPIC,              // 63
    C_ENDGAME6APIC,              // 64
    C_ENDGAME6BPIC,              // 65

    // Lump Start
    L_GUYPIC,                    // 66
    L_COLONPIC,                  // 67
    L_NUM0PIC,                   // 68
    L_NUM1PIC,                   // 69
    L_NUM2PIC,                   // 70
    L_NUM3PIC,                   // 71
    L_NUM4PIC,                   // 72
    L_NUM5PIC,                   // 73
    L_NUM6PIC,                   // 74
    L_NUM7PIC,                   // 75
    L_NUM8PIC,                   // 76
    L_NUM9PIC,                   // 77
    L_PERCENTPIC,                // 78
    L_APIC,                      // 79
    L_BPIC,                      // 80
    L_CPIC,                      // 81
    L_DPIC,                      // 82
    L_EPIC,                      // 83
    L_FPIC,                      // 84
    L_GPIC,                      // 85
    L_HPIC,                      // 86
    L_IPIC,                      // 87
    L_JPIC,                      // 88
    L_KPIC,                      // 89
    L_LPIC,                      // 90
    L_MPIC,                      // 91
    L_NPIC,                      // 92
    L_OPIC,                      // 93
    L_PPIC,                      // 94
    L_QPIC,                      // 95
    L_RPIC,                      // 96
    L_SPIC,                      // 97
    L_TPIC,                      // 98
    L_UPIC,                      // 99
    L_VPIC,                      // 100
    L_WPIC,                      // 101
    L_XPIC,                      // 102
    L_YPIC,                      // 103
    L_ZPIC,                      // 104
    L_EXPOINTPIC,                // 105
    L_APOSTROPHEPIC,             // 106
    L_GUY2PIC,                   // 107
    L_BJWINSPIC,                 // 108
    STATUSBARPIC,                // 109
    TITLEPIC,                    // 110

    S_MOUSESENSPIC,              // 111
    S_OPTIONSPIC,                // 112
    S_SOUNDPIC,                  // 113
    S_SKILLPIC,                  // 114
    S_EPISODEPIC,                // 115
    S_CHANGEPIC,                 // 116
    S_CUSTOMPIC,                 // 117
    S_CONTROLPIC,                // 118

    CREDITSPIC,                  // 119
    HIGHSCORESPIC,               // 120
    // Lump Start
    KNIFEPIC,                    // 121
    GUNPIC,                      // 122
    MACHINEGUNPIC,               // 123
    GATLINGGUNPIC,               // 124
    NOKEYPIC,                    // 125
    GOLDKEYPIC,                  // 126
    SILVERKEYPIC,                // 127
    N_BLANKPIC,                  // 128
    N_0PIC,                      // 129
    N_1PIC,                      // 130
    N_2PIC,                      // 131
    N_3PIC,                      // 132
    N_4PIC,                      // 133
    N_5PIC,                      // 134
    N_6PIC,                      // 135
    N_7PIC,                      // 136
    N_8PIC,                      // 137
    N_9PIC,                      // 138
    FACE1APIC,                   // 139
    FACE1BPIC,                   // 140
    FACE1CPIC,                   // 141
    FACE2APIC,                   // 142
    FACE2BPIC,                   // 143
    FACE2CPIC,                   // 144
    FACE3APIC,                   // 145
    FACE3BPIC,                   // 146
    FACE3CPIC,                   // 147
    FACE4APIC,                   // 148
    FACE4BPIC,                   // 149
    FACE4CPIC,                   // 150
    FACE5APIC,                   // 151
    FACE5BPIC,                   // 152
    FACE5CPIC,                   // 153
    FACE6APIC,                   // 154
    FACE6BPIC,                   // 155
    FACE6CPIC,                   // 156
    FACE7APIC,                   // 157
    FACE7BPIC,                   // 158
    FACE7CPIC,                   // 159
    FACE8APIC,                   // 160
    GOTGATLINGPIC,               // 161
    MUTANTBJPIC,                 // 162
    PAUSEDPIC,                   // 163
    GETPSYCHEDPIC,               // 164

    TILE8,                       // 165

    ERRORSCREEN,                 // 166

    T_DEMO0,                     // 167
    T_DEMO1,                     // 168
    T_DEMO2,                     // 169
    T_DEMO3,                     // 170

    ENUMEND
} graphicnums;

//
// Data LUMPs
//
#define README_LUMP_START       H_BJPIC
#define README_LUMP_END         H_BOTTOMINFOPIC

#define CONTROLS_LUMP_START     C_OPTIONSPIC
#define CONTROLS_LUMP_END       (L_GUYPIC - 1)

#define LEVELEND_LUMP_START     L_GUYPIC
#define LEVELEND_LUMP_END       L_BJWINSPIC

#define LATCHPICS_LUMP_START    KNIFEPIC
#define LATCHPICS_LUMP_END      GETPSYCHEDPIC


//
// Amount of each data item
//
#define NUMCHUNKS    ENUMEND
#define NUMFONT      2
#define NUMFONTM     0
#define NUMPICS      (GETPSYCHEDPIC - NUMFONT)
#define NUMPICM      0
#define NUMSPRITES   0
#define NUMTILE8     72
#define NUMTILE8M    0
#define NUMTILE16    0
#define NUMTILE16M   0
#define NUMTILE32    0
#define NUMTILE32M   0
#define NUMEXTERNS   13
//
// File offsets for data items
//
#define STRUCTPIC    0

#define STARTFONT    1
#define STARTFONTM   3
#define STARTPICS    3
#define STARTPICM    TILE8
#define STARTSPRITES TILE8
#define STARTTILE8   TILE8
#define STARTTILE8M  ERRORSCREEN
#define STARTTILE16  ERRORSCREEN
#define STARTTILE16M ERRORSCREEN
#define STARTTILE32  ERRORSCREEN
#define STARTTILE32M ERRORSCREEN
#define STARTEXTERNS ERRORSCREEN
