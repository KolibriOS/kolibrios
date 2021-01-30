//
//      ID Engine
//      ID_SD.c - Sound Manager for Wolfenstein 3D
//      v1.2
//      By Jason Blochowiak
//

//
//      This module handles dealing with generating sound on the appropriate
//              hardware
//
//      Depends on: User Mgr (for parm checking)
//
//      Globals:
//              For User Mgr:
//                      SoundBlasterPresent - SoundBlaster card present?
//                      AdLibPresent - AdLib card present?
//                      SoundMode - What device is used for sound effects
//                              (Use SM_SetSoundMode() to set)
//                      MusicMode - What device is used for music
//                              (Use SM_SetMusicMode() to set)
//                      DigiMode - What device is used for digitized sound effects
//                              (Use SM_SetDigiDevice() to set)
//
//              For Cache Mgr:
//                      NeedsDigitized - load digitized sounds?
//                      NeedsMusic - load music?
//

/// ///////////// ///
/// In Kolibrios - stub

#include "wl_def.h"
//#include <SDL_mixer.h>
#if defined(GP2X_940)
#include "gp2x/fmopl.h"
#else
#ifdef USE_GPL
#include "dosbox/dbopl.h"
#else
#include "mame/fmopl.h"
#endif
#endif

#define ORIGSAMPLERATE 7042

typedef struct
{
	char RIFF[4];
	longword filelenminus8;
	char WAVE[4];
	char fmt_[4];
	longword formatlen;
	word val0x0001;
	word channels;
	longword samplerate;
	longword bytespersec;
	word bytespersample;
	word bitspersample;
} headchunk;

typedef struct
{
	char chunkid[4];
	longword chunklength;
} wavechunk;

typedef struct
{
    uint32_t startpage;
    uint32_t length;
} digiinfo;

//      Global variables
        boolean         AdLibPresent,
                        SoundBlasterPresent,SBProPresent,
                        SoundPositioned;
        SDMode          SoundMode;
        SMMode          MusicMode;
        SDSMode         DigiMode;
static  byte          **SoundTable;
        int             DigiMap[LASTSOUND];
        int             DigiChannel[STARTMUSIC - STARTDIGISOUNDS];

//      Internal variables
static  boolean                 SD_Started;
static  boolean                 nextsoundpos;
static  soundnames              SoundNumber;
static  soundnames              DigiNumber;
static  word                    SoundPriority;
static  word                    DigiPriority;
static  int                     LeftPosition;
static  int                     RightPosition;

        word                    NumDigi;
static  digiinfo               *DigiList;
static  boolean                 DigiPlaying;

//      PC Sound variables
static  volatile byte           pcLastSample;
static  byte * volatile         pcSound;
static  longword                pcLengthLeft;

//      AdLib variables
static  byte * volatile         alSound;
static  byte                    alBlock;
static  longword                alLengthLeft;
static  longword                alTimeCount;
static  Instrument              alZeroInst;

//      Sequencer variables
static  volatile boolean        sqActive;
static  word                   *sqHack;
static  word                   *sqHackPtr;
static  int                     sqHackLen;
static  int                     sqHackSeqLen;
static  longword                sqHackTime;

// STUB
void    SD_Startup(void){};
void  SD_Shutdown(void){};

int     SD_GetChannelForDigi(int which){};
void    SD_PositionSound(int leftvol,int rightvol){};
boolean SD_PlaySound(soundnames sound){};
void    SD_SetPosition(int channel, int leftvol,int rightvol){};
void    SD_StopSound(void){};
        void        SD_WaitSoundDone(void){};

void    SD_StartMusic(int chunk){};
void    SD_ContinueMusic(int chunk, int startoffs){};
void    SD_MusicOn(void){};
 void               SD_FadeOutMusic(void){};
int     SD_MusicOff(void){};

boolean SD_MusicPlaying(void){};
boolean SD_SetSoundMode(SDMode mode){};
boolean SD_SetMusicMode(SMMode mode){};
word    SD_SoundPlaying(void){};

void    SD_SetDigiDevice(SDSMode){};
void	SD_PrepareSound(int which){};
int     SD_PlayDigitized(word which,int leftpos,int rightpos){};
void    SD_StopDigitized(void){};
