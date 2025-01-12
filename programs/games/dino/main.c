#include <stdbool.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/ksys.h>

#include "misc.h"
#include "graphics.h"
#include "distance_meter.h"
#include "cloud.h"
#include "obstacle.h"
#include "horizon_line.h"
#include "trex.h"
#include "runner.h"

static uint8_t keyboard_layout[128];

int main(int argc, char* args[]) {
	srand((unsigned int)time(NULL)); // Seed the random number generator

	ksys_pos_t win_pos = _ksys_screen_size();
	win_pos.x /= 2;
	win_pos.x -= DEFAULT_WIDTH/2;
	win_pos.y /= 2;
	win_pos.y -= DEFAULT_HEIGHT/2;
	dbg_printf("wx = %d, wy = %d\n", win_pos.x, win_pos.y);
	ksys_colors_table_t sys_color_table;
    _ksys_get_system_colors(&sys_color_table);
	_ksys_set_event_mask(0xC0000027); // !
	_ksys_set_key_input_mode(KSYS_KEY_INPUT_MODE_SCANC);
	_ksys_keyboard_layout(KSYS_KEYBOARD_LAYOUT_NORMAL, keyboard_layout);

	graphicsInit();

	runnerInit();

	dbg_printf("dino started\n");

	int ext_code = 0;
    uint8_t old_mode = 0;

	bool quit = false;
	while (quit == false) {
		int frameStartTime = getTimeStamp();
		//printf("frameStartTime = %d\n", frameStartTime);
        uint32_t kos_event = _ksys_check_event();
        switch (kos_event) {
		case KSYS_EVENT_BUTTON:
			switch (_ksys_get_button()){
			case 1:
				quit = true;
				break;
			default:
				break;
			}
			break;
		case KSYS_EVENT_KEY:
			{
				ksys_oskey_t key = _ksys_get_key();
				uint8_t scancode = key.code;
				if (scancode == 0xE0 || scancode == 0xE1) {
                    ext_code = scancode;
                    break;
                }
                if (ext_code == 0xE1 && (scancode & 0x7F) == 0x1D) {
                    break;
                }
                if (ext_code == 0xE1 && scancode == 0xC5) {
                    ext_code = 0;
                    break;
                }
                uint8_t code = keyboard_layout[scancode & 0x7F];
 
                if (ext_code == 0xE0) {
					code -= 96;
				}
                ext_code = 0;
				
				if (scancode < 128) { // KEYDOWN
					//dbg_printf("Keydown: key = 0x%x, scancode = 0x%x, code = 0x%x (%u) state = 0x%x\n", key.val, scancode, code, code, key.state);
					//dbg_printf("keydown c  = %u\n", key.code);
					runnerOnKeyDown(code);
				} else { // KEYUP
					//dbg_printf("Keyup: key = 0x%x, scancode = 0x%x, code = 0x%x (%u) state = 0x%x\n", key.val, scancode, code, code, key.state);
					//dbg_printf("keyup c  = %u\n", key.code);
					runnerOnKeyUp(code);
				}
			}
			break;
		case KSYS_EVENT_REDRAW:
			//dbg_printf("KSYS_EVENT_REDRAW\n");
			_ksys_start_draw();
    		_ksys_create_window(win_pos.x, win_pos.y, screenImage->Width + 10, screenImage->Height + 29, WINDOW_TITLE, sys_color_table.work_area, 0x54); // 0x54. note: C = 1 !!
			graphicsRender();
			_ksys_end_draw();
			break; 
		default:
			break;
		}

		if (runner.nextUpdateScheduled) {
			//printf("runner update! %u\n", getTimeStamp());
			//dbg_printf("runnerUpdate\n");
			runnerUpdate();
		}
		else {
			if (runner.skipUpdateNow) {
				//printf("Skipped one update\n");
				runner.nextUpdateScheduled = true;
				runner.skipUpdateNow = false;
			}
		}

		int frameTime = getTimeStamp() - frameStartTime;
		if (frameTime < 0) {
			frameTime = DELTA_MS_DEFAULT;
		}
#define FRAME_TIME 20 //16
		// dbg_printf("frameTime = %d\n", frameTime);
		if (frameTime < FRAME_TIME) { // 1000ms/60frames = 16.(6)
			// printf("frameTime = %d\n", frameTime);
			if (runner.crashed) {
				// dbg_printf("runner.timeAfterCrashedMs +=\n");
				runner.timeAfterCrashedMs += FRAME_TIME - frameTime;
			}
			graphicsDelay(FRAME_TIME - frameTime);
		}
	}

	graphicsDestroy();

	return 0;
}
