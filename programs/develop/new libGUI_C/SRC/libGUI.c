/*
	libGUI dinamic library
	(c) andrew_programmer 2009
 */

//service libGUI types of data,functions and constants
#include "types.h"
#include "libGUI.h"
#include "kolibri_system.h"
#include "draw_controls.h"
#include "fonts_meneger.h"
#include "keys.h"

//controls
#include "control_button.h"
#include "control_image.h"
#include "control_progress_bar.h"
#include "control_scroll_bar.h"
#include "control_scrolled_window.h"
#include "control_text.h"

//some libC functions
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "stdio.inc"
#include "string.inc"
#include "stdlib.inc"
#include "kolibri_system.inc"
#include "draw_controls.inc"
#include "fonts_meneger.inc"
#include "libGUI_menegement.inc"
#include "parent_window.inc"
#include "main_libGUI.inc"
#include "control_text.inc"
#include "control_image.inc"
#include "control_button.inc"
#include "control_progress_bar.inc"
#include "control_scroll_bar.inc"
#include "control_scrolled_window.inc"


typedef struct
{
	char *name;
	void *function;
}export_t;

//char szSTART[]                                           = "START";
char szLibGUIversion[]                                   = "LibGUIversion";
char szInitLibGUI[]                                      = "InitLibGUI";
char szLibGUImain[]                                      = "LibGUImain";
char szQuitLibGUI[]                                      = "QuitLibGUI";

char szCreateWindow[]                                    = "CreateWindow";
char szSetWindowSizeRequest[]                            = "SetWindowSizeRequest";

char szPackControls[]                                    = "PackControls";
char szDestroyControl[]                                  = "DestroyControl";
char szSetControlSizeRequest[]                           = "SetControlSizeRequest";
char szGetControlSizeX[]                                 = "GetControlSizeX";
char szGetControlSizeY[]                                 = "GetControlSizeY";
char szSetControlNewPosition[]                           = "SetControlNewPosition";
char szGetControlPositionX[]                             = "GetControlPositionX";
char szGetControlPositionY[]                             = "GetControlPositionY";
char szSetFocuse[]                                       = "SetFocuse";
char szRedrawControl[]                                   = "RedrawControl";
char szSpecialRedrawControl[]                            = "SpecialRedrawControl";

char szSetCallbackFunction[]                             = "SetCallbackFunction";
char szBlockCallbackFunction[]                           = "BlockCallbackFunction";
char szUnblockCallbackFunction[]                         = "UnblockCallbackFunction";

char szSetIDL_Function[]                                 = "SetIDL_Function";
char szDestroyIDL_Function[]                             = "DestroyIDL_Function";

char szSetTimerCallbackForFunction[]                     = "SetTimerCallbackForFunction";
char szDestroyTimerCallbackForFunction[]                 = "DestroyTimerCallbackForFunction";

char szSetCallbackFunctionForEvent[]                     = "SetCallbackFunctionForEvent";
char szDestroyCallbackFunctionForEvent[]                 = "DestroyCallbackFunctionForEvent";

char szCreateButton[]                                    = "CreateButton";
char szCreateButtonWithText[]                            = "CreateButtonWithText";

char szCreateProgressBar[]                               = "CreateProgressBar";
char szSetProgressBarPulse[]                             = "SetProgressBarPulse";
char szProgressBarSetText[]                              = "ProgressBarSetText";
char szProgressBarGetText[]                              = "ProgressBarGetText";

char szCreateHorizontalScrollBar[]                       = "CreateHorizontalScrollBar";
char szCreateVerticalScrollBar[]                         = "CreateVerticalScrollBar";

char szCreateScrolledWindow[]                            = "CreateScrolledWindow";
char szScrolledWindowPackControls[]                      = "ScrolledWindowPackControls";

char szCreateImage[]                                     = "CreateImage";

char szCreateText[]                                      = "CreateText";
char szTextBackgroundOn[]                                = "TextBackgroundOn";
char szTextBackgroundOff[]                               = "TextBackgroundOff";

char szLoadFont[]                                        = "LoadFont";
char szFreeFont[]                                        = "FreeFont";

export_t	EXPORTS[]__asm__("EXPORTS") =
	{
		{szLibGUIversion,                         LibGUIversion                        },
		{szInitLibGUI,                            InitLibGUI                           },
		{szLibGUImain,                            LibGUImain                           },
		{szQuitLibGUI,                            QuitLibGUI                           },
		
		{szCreateWindow,                          CreateWindow                         },
		{szSetWindowSizeRequest,                  SetWindowSizeRequest                 },
		
		{szPackControls,                          PackControls                         },
		{szDestroyControl,                        DestroyControl                       },
		{szSetControlSizeRequest,                 SetControlSizeRequest                },
		{szGetControlSizeX,                       GetControlSizeX                      },
		{szGetControlSizeY,                       GetControlSizeY                      },
		{szSetControlNewPosition,                 SetControlNewPosition                },
		{szGetControlPositionX,                   GetControlPositionX                  },
		{szGetControlPositionY,                   GetControlPositionY                  },
		{szSetFocuse,                             SetFocuse                            },       
		{szRedrawControl,                         RedrawControl                        },
		{szSpecialRedrawControl,                  SpecialRedrawControl                 },
		
		{szSetCallbackFunction,                   SetCallbackFunction                  },
		{szBlockCallbackFunction,                 BlockCallbackFunction                },
		{szUnblockCallbackFunction,               UnblockCallbackFunction              },
		
		{szSetIDL_Function,                       SetIDL_Function                      },
		{szDestroyIDL_Function,                   DestroyIDL_Function                  },
		
		{szSetTimerCallbackForFunction,           SetTimerCallbackForFunction          },
		{szDestroyTimerCallbackForFunction,       DestroyTimerCallbackForFunction      },
		
		{szSetCallbackFunctionForEvent,           SetCallbackFunctionForEvent          },
		{szDestroyCallbackFunctionForEvent,       DestroyCallbackFunctionForEvent      },
		
		{szCreateButton,                          CreateButton                         },
		{szCreateButtonWithText,                  CreateButtonWithText                 },
		
		{szCreateProgressBar,                     CreateProgressBar                    },
		{szSetProgressBarPulse,                   SetProgressBarPulse                  },
		{szProgressBarSetText,                    ProgressBarSetText                   },
		{szProgressBarGetText,                    ProgressBarGetText                   },
		
		{szCreateHorizontalScrollBar,             CreateHorizontalScrollBar            },
		{szCreateVerticalScrollBar,               CreateVerticalScrollBar              },
		
		{szCreateScrolledWindow,                  CreateScrolledWindow                 },
		{szScrolledWindowPackControls,            ScrolledWindowPackControls           },
		
		{szCreateImage,                           CreateImage                          },
		
		{szCreateText,                            CreateText                           },
		{szTextBackgroundOn,                      TextBackgroundOn                     },
		{szTextBackgroundOff,                     TextBackgroundOff                    },
		
		{szLoadFont,                              LoadFont                             },
		{szFreeFont,                              FreeFont                             },
		
		{NULL,NULL},
	};
