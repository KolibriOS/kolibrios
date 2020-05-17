
/*
 * Author: JohnXenox aka Aleksandr Igorevich.
 *
 * Programme name: Beat
 * Description: A simple metronome.
 */

#include <stdio.h>
#include <string.h>
#include <kolibrisys.h>

char header[] = "Beat 2020.05.17";

unsigned int skin_height = 0;

unsigned int key = 0;
unsigned int btn = 0;

short ctrl_keys_state = 0;

int thread_stack = 0x1000;
int stack_size = 0x100;


char startButtonBit = 0;

char tempoSelector = 0;

short tempo = 100; // Beats Per Minute.

char meter = 4;  // 4/4

char accentBeatFlags[12] = {0};


char counter = 1;

counterIndicatorFlag = 0;


//Event mask bits for function 40.
enum EVENT_MASKS
{
     EVM_REDRAW =        0x00000001,
     EVM_KEY =           0x00000002,
     EVM_BUTTON =        0x00000004,
     EVM_EXIT =          0x00000008,
     EVM_BACKGROUND =    0x00000016,
     EVM_MOUSE =         0x00000032,
     EVM_IPC =           0x00000064,
     EVM_STACK =         0x00000128,
     EVM_DEBUG =         0x00000256,
     EVM_STACK2 =        0x00000512,
     EVM_MOUSE_FILTER =  0x80000000,
     EVM_CURSOR_FILTER = 0x40000000
};



enum EVENTS
{
     EVENT_REDRAW = 1,   /* Window and window elements should be redrawn */
     EVENT_KEY = 2,      /* A key on the keyboard was pressed */
     EVENT_BUTTON = 3,   /* A button was clicked with the mouse */
     EVENT_MOUSE = 6     /* Mouse activity (movement, button press) was detected */
};


struct system_colors
{
    int frame;             // color of frame.
    int grab;              // color of header.
    int grab_button;       // color of button on header bar.
    int grab_button_text;  // color of text on button on header bar.
    int grab_text;         // color of text on header.
    int work;              // color of working area.
    int work_button;       // color of button in working area.
    int work_button_text;  // color of text on button in working area.
    int work_text;         // color of text in working area.
    int work_graph;        // color of graphics in working area.
};


struct system_colors sc;

#include "Beat_lang.h"
#include "Beat_lib.h"


#define  KEY_ARROW_UP     0xB2
#define  KEY_ARROW_DOWN   0xB1
#define  KEY_ARROW_LEFT   0xB0
#define  KEY_ARROW_RIGHT  0xB3

#define  KEYS_CTRL_ARROW_UP     0x52
#define  KEYS_CTRL_ARROW_DOWN   0x51
#define  KEYS_CTRL_ARROW_LEFT   0x50
#define  KEYS_CTRL_ARROW_RIGHT  0x53

#define  KEY_SPACE  0x20

#define  KEY_ESCAPE  0x1B

#define  KEY_SLASH  0x2F




char redraw_flag = 0;


#define SMPL_NAME1 "Beep1.raw"
#define SMPL_NAME2 "Beep2.raw"
#define PRG_NAME "PlayNote"

char _path_to_a_sample1[4096] = {0};
char _path_to_a_sample2[4096] = {0};
char _path_to_a_playnote[4096] = {0};

#define SEARCH_PATHES_NUMBER 3

unsigned char* search_pathes_to_a_playnote[] = {
_path_to_a_playnote,
"/sys/"PRG_NAME,
"/sys/Media/"PRG_NAME,
};

char *path_to_a_playnote = 0;



int main(int argc, char** argv)
{
    setCurrentPathToARawFile(_path_to_a_sample1, argv[0], SMPL_NAME1);
    setCurrentPathToARawFile(_path_to_a_sample2, argv[0], SMPL_NAME2);
    setCurrentPathToARawFile(_path_to_a_playnote, argv[0], PRG_NAME);

    // searches for a PlayNote programme.
    for(char i = 0; (i < SEARCH_PATHES_NUMBER); i++)
    {
        if(startApp("/sys/loool.raw", 0, search_pathes_to_a_playnote[i]) > 0)
        {
            path_to_a_playnote = search_pathes_to_a_playnote[i];
        }
    }

    if(path_to_a_playnote == 0)
    {
        #if defined (lang_en)
            startApp("\"Can't find a PlayNote programme!\" -W", 0, "/sys/@notify");
        #elif defined (lang_ru)
            startApp("\"Не могу найти программу PlayNote!\" -W", 0, "/sys/@notify");
        #endif

        return 1;
    }

    _ksys_set_wanted_events(EVM_REDRAW | EVM_KEY | EVM_BUTTON | EVM_MOUSE_FILTER);





    drawWindow();

    for (;;)
    {

        if(startButtonBit == 1)
        {

            if(redraw_flag == 0)
            {

                //if(counter == 0) counter++;

                if(counter < meter)
                    counter++;
                else if(counter == meter)
                    counter = 1;



                if(accentBeatFlags[counter - 1] == 0)
                {
                    // play a beep sound.
                    startApp(_path_to_a_sample2, 0, path_to_a_playnote);
                }
                else
                {
                    // play a beep sound.
                    startApp(_path_to_a_sample1, 0, path_to_a_playnote);
                }




                showCounterIndicator();


//              makeDelay(7);

            }
            else
            {
                redraw_flag = 0;
            }

        }

        switch(_ksys_wait_for_event(6000 / (tempo)))
        {
            case EVENT_REDRAW:

                   redraw_flag = 1;

                   drawWindow();
                   break;

            case EVENT_KEY:
                   ctrl_keys_state = getControlKeysOnAKeyboard();

                   // key pressed, read it and ignore
                   key = _ksys_get_key();

                   key = ((key >> 8) & 0x000000FF);


                   //printfOnADebugBoard("ctrl_keys_state: %d\n", ctrl_keys_state);

                   // makes exit.
                   if(key == KEY_ESCAPE) return 0;

                   // starts beats.
                   if(key == KEY_SPACE)
                   {
                       if(startButtonBit == 0)
                           startButtonBit = 1;
                       else
                           startButtonBit = 0;

                       showStartButton();
                   }


                   // decreases tempo.
                   if(key == KEY_ARROW_LEFT)
                   {
                       if(tempo != 1)
                       {
                           showTempoBar2(--tempo);
                           setTempoSelectorByTempo(&tempo, &tempoSelector);

                       }
                   }

                   // increases tempo.
                   if(key == KEY_ARROW_RIGHT)
                   {
                       if(tempo != 320)
                       {
                           showTempoBar2(++tempo);
                           setTempoSelectorByTempo(&tempo, &tempoSelector);
                       }
                   }


                   // decreases a tempo selector.
                   if(key == KEYS_CTRL_ARROW_LEFT)
                   {
                       if ((ctrl_keys_state == 4) || (ctrl_keys_state == 8))
                       {
                           if(tempoSelector != 0)
                           {
                               showTempoBar1(--tempoSelector);
                               setTempoByTempoSelector(&tempo, tempoSelector);
                               showTempoBar2(tempo);
                           }
                       }
                   }


                   // increases a tempo selector.
                   if(key == KEYS_CTRL_ARROW_RIGHT)
                   {
                       if ((ctrl_keys_state == 4) || (ctrl_keys_state == 8))
                       {
                           if(tempoSelector != 9)
                           {
                               showTempoBar1(++tempoSelector);
                               setTempoByTempoSelector(&tempo, tempoSelector);
                               showTempoBar2(tempo);
                           }
                       }
                   }


                   if(key == 0x2C)
                   {
                       if(meter > 1)
                       {
                            showMeterBar(--meter);
                            showMeterIndicator();
                       }

                   }

                   if(key == 0x2E)
                   {
                       if(meter < 12)
                       {
                            showMeterBar(++meter);
                            showMeterIndicator();
                       }
                   }


                   for(unsigned char i = 0; i < 9; i++)
                   {
                       if(key == (0x31 + i))
                       {
                               if(accentBeatFlags[i] == 0)
                               {
                                   accentBeatFlags[i] = 1;
                               }
                               else if (accentBeatFlags[i] != 0)
                               {
                                   accentBeatFlags[i] = 0;
                               }


                           showMeterIndicator();
                       }
                   }

                   if(key == 0x30)
                   {
                       if(accentBeatFlags[9] == 0)
                           accentBeatFlags[9] = 1;
                       else if (accentBeatFlags[9] != 0)
                           accentBeatFlags[9] = 0;

                       showMeterIndicator();
                   }

                   if(key == 0x2D)
                   {
                       if(accentBeatFlags[10] == 0)
                           accentBeatFlags[10] = 1;
                       else if (accentBeatFlags[10] != 0)
                           accentBeatFlags[10] = 0;

                       showMeterIndicator();
                   }

                   if(key == 0x3D)
                   {
                       if(accentBeatFlags[11] == 0)
                           accentBeatFlags[11] = 1;
                       else if (accentBeatFlags[11] != 0)
                           accentBeatFlags[11] = 0;

                       showMeterIndicator();
                   }




                   // invertation of colours.
                   if(key == KEY_SLASH)
                   {
                       if(counterIndicatorFlag != 0)
                           counterIndicatorFlag = 0;
                       else if (counterIndicatorFlag == 0)
                           counterIndicatorFlag = 1;

                       showCounterIndicator();
                   }


                   break;

            case EVENT_BUTTON:
                   // button pressed; we have only one button, close
                   btn = _ksys_get_button_id();

                   if(btn == 1) return 0;

                   if(btn == 7)
                   {

                       if(startButtonBit == 0)
                           startButtonBit = 1;
                       else
                           startButtonBit = 0;

                       showStartButton();
                  }


                   // decreases a tempo selector.
                   if(btn == 10)
                   {
                       if(tempoSelector != 0)
                       {
                           showTempoBar1(--tempoSelector);
                           setTempoByTempoSelector(&tempo, tempoSelector);
                           showTempoBar2(tempo);
                       }
                   }


                   // increases a tempo selector.
                   if(btn == 11)
                   {
                       if(tempoSelector != 9)
                       {
                           showTempoBar1(++tempoSelector);
                           setTempoByTempoSelector(&tempo, tempoSelector);
                           showTempoBar2(tempo);
                       }
                   }


                   // decreases tempo.
                   if(btn == 12)
                   {
                       if(tempo != 1)
                       {
                           showTempoBar2(--tempo);
                           setTempoSelectorByTempo(&tempo, &tempoSelector);

                       }
                   }


                   // increases tempo.
                   if(btn == 13)
                   {
                       if(tempo < 320)
                       {
                           showTempoBar2(++tempo);
                           setTempoSelectorByTempo(&tempo, &tempoSelector);
                       }
                   }


                   if(btn == 14)
                   {
                       if(meter > 1)
                       {
                            showMeterBar(--meter);

                            // clreans unused flags.
                            for(unsigned char i = meter; i < 11; i++)
                            {
                                accentBeatFlags[i] = 0;
                            }

                            //if(meter == 1) accentBeatFlags[0] = 0;


                            showMeterIndicator();
                       }

                   }

                   if(btn == 15)
                   {
                       if(meter < 12)
                       {
                            showMeterBar(++meter);

                            // clreans unused flags.
                            for(unsigned char i = meter; i < 11; i++)
                            {
                                accentBeatFlags[i] = 0;
                            }

                            showMeterIndicator();
                       }

                   }


                   for(unsigned char i = 0; i < 12; i++)
                   {
                       if(btn == (100 + i))
                       {
                           //if(meter > 1)
                           //{
                               if(accentBeatFlags[i] == 0)
                               {
                                   accentBeatFlags[i] = 1;
                               }
                               else if (accentBeatFlags[i] != 0)
                               {
                                   accentBeatFlags[i] = 0;
                               }
                           //}
                           //else if(meter == 1) accentBeatFlags[i] = 0;

                           showMeterIndicator();
                       }
                   }


                   if(btn == 200)
                   {
                       if(counterIndicatorFlag != 0)
                           counterIndicatorFlag = 0;
                       else if (counterIndicatorFlag == 0)
                           counterIndicatorFlag = 1;

                       showCounterIndicator();
                   }



                   break;

        }
    }
}



















