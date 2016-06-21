#ifndef KOLIBRI_PROGRESSBAR_H
#define KOLIBRI_PROGRESSBAR_H

struct progress_bar {
	unsigned int value; 
    unsigned int left;
    unsigned int top; 
    unsigned int width; 
    unsigned int height; 
    unsigned int style; 
    unsigned int min;
    unsigned int max;
    unsigned int back_color;
    unsigned int progress_color;
    unsigned int frame_color;
};

struct progress_bar* kolibri_new_progress_bar(unsigned int min_value, unsigned int max_value, unsigned int cur_value, unsigned int tlx, unsigned int tly, unsigned int sizex, unsigned int sizey)
{
    struct progress_bar *new_progressbar = (struct progress_bar *)malloc(sizeof(struct progress_bar));
    
    new_progressbar -> value = cur_value; 
    new_progressbar -> left = tlx;
    new_progressbar -> top = tly; 
    new_progressbar -> width = sizex; 
    new_progressbar -> height = sizey; 
    new_progressbar -> style = 1; 
    new_progressbar -> min = min_value;
    new_progressbar -> max = max_value;
    new_progressbar -> back_color = 0xffffff;
    new_progressbar -> progress_color = 0x00ff00;
    new_progressbar -> frame_color = 0x000000;
    return new_progressbar;
}

extern void (*progressbar_draw)(struct progress_bar *) __attribute__((__stdcall__));
extern void (*progressbar_progress)(struct progress_bar *) __attribute__((__stdcall__));

#endif /* KOLIBRI_PROGRESSBAR_H */
