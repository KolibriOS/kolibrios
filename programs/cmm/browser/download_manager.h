#ifdef LANG_RUS
    #define DL_WINDOW_HEADER "Менеджер загрузок"
    #define START_DOWNLOADING "Начать закачку"
    #define STOP_DOWNLOADING "Остановить"
    #define SHOW_IN_FOLDER "Показать в папке"
    #define OPEN_FILE_TEXT "Открыть файл"
    #define FILE_SAVED_AS "'Менеджер загрузок\nФайл сохранен как "
    #define KB_RECEIVED "Идет скачивание... %s получено"
#else
    #define DL_WINDOW_HEADER "Download Manager"
    #define START_DOWNLOADING "Start downloading"
    #define STOP_DOWNLOADING "Stop downloading"
    #define SHOW_IN_FOLDER "Show in folder"
    #define OPEN_FILE_TEXT "Open file"
    #define FILE_SAVED_AS "'Download manager\nFile saved as "
    #define KB_RECEIVED "Downloading... %s received"
#endif
char save_to[4096] = "/tmp0/1/Downloads";
 
proc_info DL_Form;
char downloader_edit[10000];
char filepath[4096];
int mouse_twbi;
edit_box ed = {250,20,20,0xffffff,0x94AECE,0xffffff,0xffffff,0x10000000,sizeof(downloader_edit),#downloader_edit,#mouse_twbi,2,19,19};
progress_bar pb = {0, 20, 58, 350, 17, 0, 0, 100, 0xFFFfff, 0x74DA00, 0x9F9F9F};
//progress_bar pb = {0, 180, 55, 225, 12, 0, 0, 100, 0xFFFfff, 0x74DA00, 0x9F9F9F};
//progress_bar: value, left, top, width, height, style, min, max, back_color, progress_color, frame_color;
    
 
byte downloader_opened;
char downloader_stak[4096];
char str[2048];
char aux[2048];
 
 
 
void Downloader()  
{
    int key, i;
    char notify_message[4296];
    downloader_opened = 1;
    SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER + EVM_STACK);

    system.color.get();
    pb.frame_color = system.color.work_dark;
 
    filepath[0] = NULL;
    
    downloader.Stop();
    if (downloader_edit[0]) StartDownloading(); else strcpy(#downloader_edit, "http://");
    ed.size = ed.pos = ed.shift = ed.shift_old = strlen(#downloader_edit);
 
    loop() switch(WaitEvent())
    {
        case evMouse:
            if (!CheckActiveProcess(DL_Form.ID)) break;
            edit_box_mouse stdcall (#ed);
            break;

        case evButton:
            Key_Scan(GetButtonID());
            break;

        case evKey:
            GetKeys();
            edit_box_key stdcall(#ed);
            if (key_scancode==SCAN_CODE_ENTER) Key_Scan(301);
            break;

        case evReDraw:
            DefineAndDrawWindow(215, 100, 580, 130, 0x74, system.color.work, DL_WINDOW_HEADER, 0);
            GetProcessInfo(#DL_Form, SelfInfo);
            if (DL_Form.status_window>2) break;
            if (DL_Form.height<120) MoveSize(OLD,OLD,OLD,120);
            if (DL_Form.width<280) MoveSize(OLD,OLD,280,OLD);
            DL_Draw_Window();
            break;
           
        default:
            if (!downloader.MonitorProgress()) break;
            pb.max = downloader.data_full_size;
            if (pb.value != downloader.data_downloaded_size)
            {
                pb.value = downloader.data_downloaded_size;
                progressbar_draw stdcall(#pb);
                DrawDownloading();
            }
            if (downloader.state == STATE_COMPLETED)
            {
                if (!dir_exists(#save_to)) CreateDir(#save_to);
                strcpy(#filepath, #save_to);
                chrcat(#filepath, '/');
   				// Clean all slashes at the end
       			strcpy(#aux,  #downloader_edit);
       			while (aux[strlen(#aux)-1] == '/') {
                    aux[strlen(#aux)-1] = 0;
                }
                strcat(#filepath,  #aux+strrchr(#aux, '/'));
				
				for (i=0; i<strlen(#filepath); i++) if(filepath[i]==':')||(filepath[i]=='?')filepath[i]='-';
 
                if (CreateFile(downloader.data_downloaded_size, downloader.bufpointer, #filepath)==0)
                    sprintf(#notify_message, "%s%s%s",FILE_SAVED_AS,#filepath,"' -Dt");
                else
                    sprintf(#notify_message, "%s%s%s","'Download manager\nError! Can\96t save file as ",#filepath,"' -Et");
                
                notify(#notify_message);
                StopDownloading();
                DL_Draw_Window();
                break;
            }          
    }
}
 
void Key_Scan(int id)
{
    if (id==001) { downloader_opened=0; StopDownloading(); ExitProcess(); }
    if (id==301) && (downloader.http_transfer <= 0) StartDownloading();
    if (id==302) StopDownloading();
    if (id==305) RunProgram("/sys/File managers/Eolite", #save_to);
    if (id==306) RunProgram("/sys/@open", #filepath);
}
 
void DL_Draw_Window()
{  
    int cleft = 15;
    int but_x = 0;
    int but_y = 58;
    DrawBar(0,0, DL_Form.cwidth, DL_Form.cheight, system.color.work);
    DeleteButton(305);
    DeleteButton(306);
    if (downloader.state == STATE_NOT_STARTED) || (downloader.state == STATE_COMPLETED)
    {
        but_x = cleft + DrawStandartCaptButton(cleft, but_y, 301, START_DOWNLOADING);   
        if (filepath[0])
        {
            but_x += DrawStandartCaptButton(but_x, but_y, 305, SHOW_IN_FOLDER);
            DrawStandartCaptButton(but_x, but_y, 306, OPEN_FILE_TEXT);  
        }
    }
    if (downloader.state == STATE_IN_PROGRESS)
    {
        DrawCaptButton(DL_Form.width - 190, but_y, 167, 26, 302, system.color.work_button, system.color.work_button_text, STOP_DOWNLOADING);
        DrawDownloading();
    }
    WriteText(cleft, ed.top + 4, 0x90, system.color.work_text, "URL:");
    ed.left = strlen("URL:")*8 + 10 + cleft;
    ed.width = DL_Form.cwidth - ed.left - cleft - 3;
    ed.offset=0;
    DrawEditBox(#ed);
}
 
void StartDownloading()
{
    StopDownloading();
    if (strncmp(#downloader_edit,"http://",7)!=0) {
        notify("'File address should start from http://' -E");
        return;
    }
    if (!downloader.Start(#downloader_edit)) {
        notify("'Error while starting download process.\nPlease, check entered path and internet connection.' -E");
        StopDownloading();
        return;
    }
    ed.blur_border_color = 0xCACACA;
    ed.flags = 100000000000b;
    pb.value = 0;
    DL_Draw_Window();
}
 
void DrawDownloading()
{
    char bytes_received[70];
    sprintf(#bytes_received, KB_RECEIVED, ConvertSizeToKb(downloader.data_downloaded_size) );
    DrawBar(15, pb.top + 22, strlen(#bytes_received+4)*12, 16, system.color.work);
    WriteText(15, pb.top + 22, 0x90, system.color.work_text, #bytes_received);
    progressbar_draw stdcall(#pb);
}
 
void StopDownloading()
{
    downloader.Stop();
    ed.blur_border_color = 0xFFFfff;
    ed.flags = 10b;
    DL_Draw_Window();
}