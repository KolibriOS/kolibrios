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

int main(int argc, char* args[]) {
	srand((unsigned int)_ksys_get_ns_count()); // Seed the random number generator

	ksys_pos_t win_pos = _ksys_screen_size();
	win_pos.x /= 2;
	win_pos.x -= DEFAULT_WIDTH/2;
	win_pos.y /= 2;
	win_pos.y -= DEFAULT_HEIGHT/2;
	_ksys_set_event_mask(0xC0000027); // !
	_ksys_set_key_input_mode(KSYS_KEY_INPUT_MODE_SCANC);

	runnerInit();

	dbg_printf("dino started\n");

	int ext_code = 0;

	for (;;) {
		int frameStartTime = getTimeStamp();
        switch (_ksys_check_event()) {
		case KSYS_EVENT_BUTTON:
			_ksys_get_button();
			_ksys_exit(); // the close button is the only one
		case KSYS_EVENT_KEY:
			{
				ksys_oskey_t key = _ksys_get_key();
				uint8_t scancode = key.code;
				if (scancode == 0xE0) {
                    ext_code = 0x100;
                    break;
                }
                // Compare raw scancodes; bit 0x100 marks the E0 extended prefix
                int code = (scancode & 0x7F) | ext_code;
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
			_ksys_create_window(
				win_pos.x, win_pos.y,
				DEFAULT_WIDTH + 2*WINDOW_BORDER,
				DEFAULT_HEIGHT + _ksys_get_skin_height() + WINDOW_BORDER,
				WINDOW_TITLE, BACKGROUND_COLOR, 0x74);
			graphicsRender();
			_ksys_end_draw();
			break;
		default:
			break;
		}

		if (runner.nextUpdateScheduled) {
			runnerUpdate();
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
