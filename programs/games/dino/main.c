#include <stdbool.h>
#include <stdlib.h>

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
	srand((unsigned int)_ksys_get_ns_count()); // Seed the random number generator

	ksys_pos_t win_pos = _ksys_screen_size();
	win_pos.x /= 2;
	win_pos.x -= DEFAULT_WIDTH/2;
	win_pos.y /= 2;
	win_pos.y -= DEFAULT_HEIGHT/2;
	_ksys_set_event_mask(0xC0000027); // !
	_ksys_set_key_input_mode(KSYS_KEY_INPUT_MODE_SCANC);
	_ksys_keyboard_layout(KSYS_KEYBOARD_LAYOUT_NORMAL, keyboard_layout);

	runnerInit();

	dbg_printf("dino started\n");

	int ext_code = 0;

	bool quit = false;
	while (quit == false) {
		int frameStartTime = getTimeStamp();
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
					runnerOnKeyDown(code);
				} else { // KEYUP
					runnerOnKeyUp(code);
				}
			}
			break;
		case KSYS_EVENT_REDRAW:
			_ksys_start_draw();
    		_ksys_create_window(win_pos.x, win_pos.y, DEFAULT_WIDTH + 10, DEFAULT_HEIGHT + 29, WINDOW_TITLE, BACKGROUND_COLOR, 0x54); // 0x54. note: C = 1 !!
			graphicsRender();
			_ksys_end_draw();
			break;
		default:
			break;
		}

		if (runner.nextUpdateScheduled) {
			runnerUpdate();
		}
		else {
			if (runner.skipUpdateNow) {
				runner.nextUpdateScheduled = true;
				runner.skipUpdateNow = false;
			}
		}

		int frameTime = getTimeStamp() - frameStartTime;
		if (frameTime < 0) {
			frameTime = DELTA_MS_DEFAULT;
		}
#define FRAME_TIME 20
		if (frameTime < FRAME_TIME) {
			graphicsDelay(FRAME_TIME - frameTime);
		}
	}

	return 0;
}
