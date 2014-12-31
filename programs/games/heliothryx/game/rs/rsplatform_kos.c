#include "rsplatform.h"

#include "../rsgame.h"
 
rs_app_t rs_app;
 
 
 
 
// set this macro to zero (0) if bug is fixed
#define FIX_MENUETOS_LEGACY_ONE_PIXEL_BORDER_GAP_BUG (-1)
 
// Fixed frame rate, set to 25
#define GAME_REQUIRED_FPS 25
 
 
 
//extern char   PATH[256];
//extern char   PARAM[256];
 
int window_width, window_height;
 
int fps = 0;
int dt = 1;
int draw_dt = 1;
int area_width = 160;
int area_height = 160;

int low_performance_counter = 0;

int logic_halted = 0;
 
int w_plus = 0;
 
 
 
 
 
#define     BIT_SET(var,mask)   { var |= (mask); }
#define     BIT_CLEAR(var,mask) { var &= ~(mask); }
#define     BIT_TOGGLE(var,mask) { var ^= (mask); }
 
#define     IS_BIT_SET(var,mask)      ( (var) & (mask) )
#define     IS_BIT_CLEARED(var,mask)  (!( (var) & (mask) ))
 
 
void BoardPuts(const char *s)
{
        unsigned int i = 0;
        while(*(s + i))
        {
                asm volatile ("int $0x40"::"a"(63), "b"(1), "c"(*(s + i)));
                i++;
        }
}
 
void board_write_integer(const char *s, int i) {
 
        char tmp[16];
       
       
 
};
 
 
void kol_wnd_resize(unsigned w, unsigned h)
{
        asm volatile ("int $0x40"::"a"(67), "b"(-1), "c"(-1), "d"(w), "S"(h));
}
 
 
 
 
void wnd_draw()
{
 
        char tmp[] = "Fps:000 | Heliothryx ";
       
        kol_paint_start();
 
        tmp[4] = '0' + ( (fps/100) % 10 );
        tmp[5] = '0' + ( (fps/10) % 10 );
        tmp[6] = '0' + ( (fps) % 10 );
 
        
        if (!logic_halted) {
	        kol_wnd_define(100, 100, window_width, window_height, 0x74ddddff, 0x34ddddff, "Heliothryx");
	        kol_wnd_caption(tmp);
        	GameProcess();
        }
        else {
	        kol_wnd_define(100, 100, window_width, window_height, 0x34ddddff, 0x34ddddff, "Heliothryx");
	        //kol_paint_bar(0, 0, window_width, window_height, 0xffffffff);
        	kol_paint_string(20, 20, "Performance is too low. Halted. ", 0x902222ff);
        };
 
        kol_paint_end();
 
}
 
 
 
 
/// ===========================================================
 
void kol_main()
{
 
        BoardPuts("Hello, Heliothryx!\n");
       
        int    err;
    int    version =-1;
 
    if((err = InitSound(&version)) !=0 ){
        BoardPuts("Sound Error 1\n");
    };
 
    if( (SOUND_VERSION>(version&0xFFFF)) ||
        (SOUND_VERSION<(version >> 16))){
        BoardPuts("Sound Error 2\n");
    }
 
 
        unsigned event;
        unsigned key;
        unsigned key_up;
       
        unsigned btn;
        unsigned pos, x, y;
        int gx, gy;
 
        //srand(kol_system_time_get());
 
        // kol_event_mask( 0xC0000027 ); // enable using of mouse
        kol_event_mask(7); // keyboard only
       
        kol_key_mode_set(1);
       
        area_width = 640;
        area_height = 360;
 
        // Initializing variables
        window_width = FIX_MENUETOS_LEGACY_ONE_PIXEL_BORDER_GAP_BUG + area_width + 10; // 2 x 5px border
        window_height = FIX_MENUETOS_LEGACY_ONE_PIXEL_BORDER_GAP_BUG + kol_skin_height() + area_height + 5; // bottom 5px border
 
 
        GameInit();
 
        wnd_draw();
 
        fps = 0;
       
        unsigned int tick_start = kol_time_tick();
        unsigned int tick_current = tick_start;
        unsigned int tick_last = tick_start;
       
        unsigned int fps_counter = 0;
        int wait_time;
        int already_drawn = 0;
       
        float xf;
        float xfs;
        int xfs_i;
       
        while (1) {
                tick_last = tick_current;
                tick_current = kol_time_tick();
                dt = tick_current - tick_last;
                tick_last = tick_current;
               
                already_drawn = 0;
               
                while (( event = kol_event_wait_time(1) )) {
               
                        switch (event) {
               
                                case 1:
                                        wnd_draw(); // <--- need to clear event!
                                        already_drawn = 1;
                                        break;
 
                                case 2:
                                        key = kol_key_get();
                                        key = (key & 0xff00)>>8;       
                                        key_up = key & 0x80;                           
                                        key = key & 0x7F;
                                        
                                        if (!logic_halted) {
                                       
		                                    if (key_up) {
		                                            GameKeyUp(key);
		                                    }
		                                    else {
		                                            GameKeyDown(key);
		                                    };
                                        
                                        };
                               
                                        break;
                       
                                case 3:
                                        switch ((kol_btn_get() & 0xff00)>>8)
                                                {
                                                case 1: // close button
                                                        kol_exit();
                                                case 2: // 'new' button
                                                        //init_board();
                                                        //wnd_draw();
                                                        break;
                                                }
                                        break;
                       
                                case 6:
                                        btn = kol_mouse_btn(); // read mouse button
                                        pos = kol_mouse_posw(); // read mouse position
                                        x = pos / 65536;
                                        y = pos % 65536;
                                        if (x > window_width)
                                                x=0;
                                        if (y > window_height)
                                                y=0;
 
                               
                                        break;
                        }
                       
                };
               
               
                if (!already_drawn) {
                        wnd_draw();
                };
               
               
                fps_counter++;         
               
                tick_current = kol_time_tick();
               
                if (tick_current > tick_start+100) {
                        fps = fps_counter;
                        fps_counter = 0;
                        tick_start += 100;
                };
       
                draw_dt = tick_current - tick_last;
               
                wait_time = (100/GAME_REQUIRED_FPS) - draw_dt;
                if (wait_time <= 0) {
                        wait_time = 1;
                };
                kol_sleep(wait_time);
                
                if (draw_dt > 19) {
                	low_performance_counter++;
                }
                else {
                	low_performance_counter--;
                	if (low_performance_counter < 0) {
                		low_performance_counter = 0;
                	};
                };
                
                if (low_performance_counter > 6) {
                	logic_halted = 1;
                	window_width = 280;
                	window_height = 80;
                	kol_wnd_caption("Heliothryx");
                	kol_wnd_resize(window_width, window_height);
                };
 
 
        }
 
        GameTerm();
 
        kol_exit();
}
