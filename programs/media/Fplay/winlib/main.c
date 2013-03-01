
#include "system.h"
#include <string.h>
#include <stdlib.h>

#include "winlib.h"


//#define ID_APPLY  1
//#define ID_CANCEL 2


int MainWindowProc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    window_t  *win;

    win = (window_t*)ctrl;

    switch(msg)
    {
//        case MSG_SIZE:
//            break;

        case MSG_COMMAND:
            switch((short)arg1)
            {
//                case ID_APPLY:
    //                if(lbx->itemSelect!=-1)
    //                {
    //                    mode = (mode_t*)lbx->items[lbx->itemSelect].attr;
    //                    printf("%d x %d %d Hz\n\r",mode->width,mode->height,mode->freq);
    //                    set_mode(mode);
    //                };
 //                   break;
                case ID_CLOSE:
                    exit(0);
            };
            break;

        default:
            def_window_proc(ctrl,msg,arg1,arg2);
    };
    return 0;
}

int main(int argc, char* argv[], char *envp[])
{
    window_t    *MainWindow;
    button_t    *btn;
//    int          result;

    init_winlib();

    rect_t *rc;

    MainWindow = create_window(0,0,200,200,480,340,MainWindowProc);

//    rc = &MainWindow->client;

//    btn = create_button(NULL, ID_CLOSE,20,57,17,17,(ctrl_t*)MainWindow);
//    fr->close_btn = btn;

//    vscroll = create_scroller(0, 1, rc->r -24, rc->t, 24, rc->b - rc->t,
//                             (ctrl_t*)MainFrame);

//    btn = create_button("Apply",ID_APPLY,300,50,80,28,(ctrl_t*)MainFrame);
//    if( !btn )
//        return 0;

//    btn = create_button("Cancel",ID_CANCEL,300,90,80,28,(ctrl_t*)MainFrame);
//    if( !btn )
//        return 0;

    show_window(MainWindow, NORMAL);
    run_window(MainWindow);

    return 0;
};
