#ifndef KOLIBRI_FRAME_H
#define KOLIBRI_FRAME_H

enum {
	TOP,
	BOTTON
};

struct frame {
	unsigned int type;
	uint16_t size_x;                  
	uint16_t start_x;                 
	uint16_t size_y;                  
	uint16_t start_y;                
	unsigned int ext_col;            
	unsigned int int_col;            
	unsigned int draw_text_flag;  
	char *text_pointer;          
	unsigned int text_position;   
	unsigned int font_number;     
	unsigned int font_size_y;           
	unsigned int font_color;            
	unsigned int font_backgr_color;
}; 

struct frame* kolibri_new_frame(uint16_t tlx, uint16_t tly, uint16_t sizex, uint16_t sizey, unsigned int ext_col, unsigned int int_col, unsigned int draw_text_flag, char *text_pointer, unsigned int text_position, unsigned int font_color, unsigned int font_bgcolor)
{
    struct frame *new_frame = (struct frame *)malloc(sizeof(struct frame));
    new_frame -> type = 0;
    new_frame -> size_x = sizex;               
    new_frame -> start_x = tlx;              
    new_frame -> size_y = sizey;               
    new_frame -> start_y = tly;              
    new_frame -> ext_col = ext_col;          
    new_frame -> int_col = int_col;          
    new_frame -> draw_text_flag = draw_text_flag;  
    new_frame -> text_pointer = text_pointer;    
    new_frame -> text_position = text_position;   
    new_frame -> font_number = 1;     
    new_frame -> font_size_y = 12;      
    new_frame -> font_color = font_color;       
    new_frame -> font_backgr_color = font_bgcolor;
    return new_frame;
}

extern void (*frame_draw)(struct frame *) __attribute__((__stdcall__));

#endif /* KOLIBRI_FRAME_H */
