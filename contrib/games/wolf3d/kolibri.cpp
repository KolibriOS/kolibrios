#include <stdlib.h>
#include <sys/ksys.h>
#include <string.h>

extern  unsigned screenWidth;
extern  unsigned screenHeight; 

void kolibri_set_win_max(void){
    unsigned multip1, multip2;
    ksys_pos_t screen_size = _ksys_screen_size();

    screen_size.y++;
    screen_size.x++;
    
    multip1 = (screen_size.y)/240;
    multip2 = (screen_size.y)/200;
    
    do{
        screenWidth =  320 * multip1;
        screenHeight = 240 * multip1;

        if(screenWidth<=screen_size.x){
            break;
        }
        
        screenWidth = 320 * multip2;
        screenHeight = 200 * multip2;
        
        if(screenWidth<=screen_size.y){
            break;
        }
        
        multip1--;
        multip2--;
        
    }while(multip1>0 && multip2>0);
}
