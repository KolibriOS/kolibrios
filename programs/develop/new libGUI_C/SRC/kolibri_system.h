/*
	some system function of KolibriOS
*/

#define	KOLIBRIOS_SYS_EVENT_REDRAW							1
#define	KOLIBRIOS_SYS_EVENT_KEYS							2
#define	KOLIBRIOS_SYS_EVENT_BUTTON_PRESSED						3
#define	KOLIBRIOS_SYS_EVENT_REDRAW_BACKGROUND					5
#define	KOLIBRIOS_SYS_EVENT_MOUSE							6
#define	KOLIBRIOS_SYS_EVENT_IPC							7
#define	KOLIBRIOS_SYS_EVENT_NET							8
#define	KOLIBRIOS_SYS_EVENT_DEBUG							9

#define	KOLIBRIOS_SYS_MOUSE_BUTTON_LEFT_DOWN					0x1
#define	KOLIBRIOS_SYS_MOUSE_BUTTON_RIGHT_DOWN					(0x1<<1)
#define	KOLIBRIOS_SYS_MOUSE_BUTTON_MIDDLE_DOWN					(0x1 <<2)
#define	KOLIBRIOS_SYS_MOUSE_BUTTON_4_DOWN						(0x1 <<3)
#define	KOLIBRIOS_SYS_MOUSE_BUTTON_5_DOWN						(0x1 <<4)

#define	KOLIBRIOS_SYS_FILE_ACCESS_SUCCESSFULLY					0
#define	KOLIBRIOS_SYS_FILE_UNDEFINED_PARTITION_OR_HARDDRIVE_BASE		1
#define	KOLIBRIOS_SYS_FILE_FUNCTION_DONT_SUPPOROTE_FOR_CURRENT_FILE_SYSTEM	2
#define	KOLIBRIOS_SYS_FILE_UNKNOWN_FILE_SYSTEM					3
#define	KOLIBRIOS_SYS_FILE_NOT_FOUND						5
#define	KOLIBRIOS_SYS_FILE_FINISHED							6
#define	KOLIBRIOS_SYS_FILE_POINTER_OUTOFMEMORY_APPLICATION			7
#define	KOLIBRIOS_SYS_FILE_MEMORY_OF_DEVICE_FILLED				8
#define	KOLIBRIOS_SYS_FILE_TABLE_DESTROYED						9
#define	KOLIBRIOS_SYS_FILE_ACCESS_DENITED						10
#define	KOLIBRIOS_SYS_FILE_DEVICE_ERROR						11

#pragma pack(push,1)
struct KOLIBRIOS_FILEIO
{
	DWORD	number_subfunction;
	DWORD	offset_in_file_low;
	DWORD	offset_in_file_hight;
	DWORD	size;
	DWORD	*data;
	BYTE	null;
	char	*full_file_path;
};
#pragma pack(pop)

typedef struct KOLIBRIOS_FILEIO fileio_t;

#pragma pack(push,1)
struct BLOCK_DATA_ENTRY_DIRECTORY
{
	DWORD	attributes;
	DWORD	types_data_of_name;
	DWORD	time_created_file;
	DWORD	date_created_file;
	DWORD	time_last_access;
	DWORD	date_last_access;
	DWORD	time_last_modification;
	DWORD	date_last_modification;
	DWORD	file_size_low;
	DWORD	file_size_hight;
	DWORD	*filename;
};
#pragma pack(pop)

typedef struct BLOCK_DATA_ENTRY_DIRECTORY bded_t;

#pragma pack(push,1)
struct PROCESS_TABLE
{
	int cpu_usage;			//+0
	int window_pos_info;			//+4
	short int reserved1;			//+8
	char name[12];			//+10
	int memstart;				//+22
	int memused;				//+26
	int pid;				//+30
	int winx_start;			//+34
	int winy_start;			//+38
	int winx_size;			//+42
	int winy_size;			//+46
	short int slot_info;			//+50
	short int reserved2;			//+52
	int clientx;				//+54
	int clienty;				//+58
	int clientwidth;			//+62
	int clientheight;			//+66
	unsigned char window_state;		//+70
	char reserved3[1024-71];		//+71
};
#pragma pack(pop)

typedef struct PROCESS_TABLE	process_table_t;

#pragma pack(push,1)
struct IMPORT
{
	char *name;
	void *data;
};
#pragma pack(pop)

typedef struct IMPORT	import_t;

static DWORD gui_get_file_size(char *filename,DWORD *buf_for_size);
static DWORD gui_read_file(char *filename,DWORD *buf_pos_size,DWORD size_read,char *buf);
static DWORD gui_create_rewrite_file(char *filename,DWORD *buf_pos_size,DWORD size_write,char *buf);
static DWORD gui_append_to_file(char *filename,DWORD *buf_pos_size,DWORD size_write,char *buf);
static void gui_debug_out_str(char *s);
static void* gui_cofflib_getproc(import_t *lib, char *name);

#define	alwinline	__attribute__((always_inline))
//------------------------------------------------------------------------------------------
//					draw window
//------------------------------------------------------------------------------------------
extern inline void __attribute__((always_inline)) gui_ksys_draw_window(DWORD x,DWORD y,DWORD sizex,DWORD sizey,DWORD flags)
{
	__asm__ __volatile__(
	"xorl %%eax,%%eax\n\t"
	"movl %0,%%ebx\n\t"
	"movl %1,%%ecx\n\t"
	"movl %4,%%edx\n\t"
	"shll $16,%%ebx\n\t"
	"shll $16,%%ecx\n\t"
	"addl %2,%%ebx\n\t"
	"addl %3,%%ecx\n\t"
	"int $0x40"
	:/*no output*/
	:"g"(x),"g"(y),"g"(sizex),"g"(sizey),"g"(flags)
	:"eax","ebx","ecx","edx");
}

//------------------------------------------------------------------------------------------
//		begin redraw window
//------------------------------------------------------------------------------------------
extern inline void  __attribute__((always_inline)) gui_ksys_begin_draw_window(void)
{
	__asm__ __volatile__(
	"int $0x40"
	:/*no output*/
	:"a"(12),"b"(1));
}

//------------------------------------------------------------------------------------------
//		finish redraw window
//------------------------------------------------------------------------------------------
extern inline void __attribute__((always_inline)) gui_ksys_finish_draw_window(void)
{
	__asm__ __volatile__(
	"int $0x40"
	:/*no output*/
	:"a"(12),"b"(2));
}

//------------------------------------------------------------------------------------------
//		set new position and new size of window
//------------------------------------------------------------------------------------------
extern inline void  alwinline gui_ksys_set_position_and_size_window(DWORD new_x,
			DWORD new_y,DWORD new_sizex,DWORD new_sizey)
{
	__asm__ __volatile__(
	"int $0x40"
	:/*no output*/
	:"a"(67),"b"(new_x),"c"(new_y),"d"(new_sizex),"S"(new_sizey));
}

//------------------------------------------------------------------------------------------
//		set title of  window
//------------------------------------------------------------------------------------------
extern inline void  gui_ksys_set_title_window(char *title)
{
	__asm__ __volatile__(
	"int $0x40"
	:/*no output*/
	:"a"(71),"b"(1),"c"(title));
}

//------------------------------------------------------------------------------------------
//		delete title of  window
//------------------------------------------------------------------------------------------
extern inline void  gui_ksys_delete_title_window(void)
{
	__asm__ __volatile__(
	"int $0x40"
	:/*no output*/
	:"a"(71),"b"(1),"c"(0));
}

//------------------------------------------------------------------------------------------
//		get information about current process
//------------------------------------------------------------------------------------------
extern inline int  gui_ksys_get_current_process_information(void *mem)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(9),"b"(mem),"c"(-1));

	return(value);
}

//------------------------------------------------------------------------------------------
//		delete title of  window
//------------------------------------------------------------------------------------------
extern inline int __attribute__((always_inline)) gui_ksys_get_skin_height(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(48),"b"(4));

	return(value);
}

//------------------------------------------------------------------------------------------
//		get pressed key
//------------------------------------------------------------------------------------------
extern inline int  __attribute__((always_inline)) gui_ksys_get_key(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(2));

	return(value);
}

//------------------------------------------------------------------------------------------
//		set keyboard input mode
//------------------------------------------------------------------------------------------
extern inline void  gui_ksys_set_keyboard_input_mode(int mode)
{
	__asm__ __volatile__(
	"int $0x40"
	:/*no output*/
	:"a"(66),"b"(1),"c"(mode));

}

//------------------------------------------------------------------------------------------
//		get keyboard input mode
//------------------------------------------------------------------------------------------
extern inline int  gui_ksys_get_keyboard_input_mode(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(66),"b"(2));

	return(value);
}

//------------------------------------------------------------------------------------------
//		get state of menegers keys
//------------------------------------------------------------------------------------------
extern inline int  gui_ksys_get_state_menegers_keys(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(66),"b"(3));

	return(value);
}

//------------------------------------------------------------------------------------------
//		set events mask
//------------------------------------------------------------------------------------------
extern inline void  gui_ksys_set_events_mask(DWORD mask)
{
	__asm__ __volatile__(
	"int $0x40"
	:/*no output*/
	:"a"(40),"b"(mask));
}

//------------------------------------------------------------------------------------------
//		wait event
//------------------------------------------------------------------------------------------
extern inline int  __attribute__((always_inline)) gui_ksys_wait_event(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(10));

	return(value);
}

//------------------------------------------------------------------------------------------
//		check for event
//------------------------------------------------------------------------------------------
extern inline int  gui_ksys_check_event(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(11));

	return(value);
}

//------------------------------------------------------------------------------------------
//		wait event
//------------------------------------------------------------------------------------------
extern inline int  gui_ksys_wait_event_with_timeout(DWORD timeout)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(23),"b"(timeout));

	return(value);
}

//------------------------------------------------------------------------------------------
//		get code of pressed button
//------------------------------------------------------------------------------------------
extern inline int  gui_ksys_get_code_pressed_button(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(17));

	return(value);
}

//------------------------------------------------------------------------------------------
//		get window mouse coordinates
//------------------------------------------------------------------------------------------
extern inline int  gui_ksys_get_window_mouse_coordinates(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(37),"b"(1));

	return(value);
}

//------------------------------------------------------------------------------------------
//		get screen mouse coordinates
//------------------------------------------------------------------------------------------
extern inline int  gui_ksys_get_screen_mouse_coordinates(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(37),"b"(0));

	return(value);
}

//------------------------------------------------------------------------------------------
//		get mouse buttons state
//------------------------------------------------------------------------------------------
extern inline int  gui_ksys_get_mouse_buttons_state(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(37),"b"(2));

	return(value);
}

//------------------------------------------------------------------------------------------
//		get mouse ruler state
//------------------------------------------------------------------------------------------
extern inline int  gui_ksys_get_mouse_ruler_state(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(37),"b"(7));

	return(value);
}

//------------------------------------------------------------------------------------------
//		put pixel in window
//------------------------------------------------------------------------------------------
extern inline void  gui_ksys_put_pixel_window(int x,int y,DWORD color)
{
	__asm__ __volatile__(
	"int $0x40"
	:/*no output*/
	:"a"(1),"b"(x),"c"(y),"d"(color));
}

//------------------------------------------------------------------------------------------
//		put image in window
//------------------------------------------------------------------------------------------
extern inline void  gui_ksys_put_image_window(char *p,int x,int y,int sizex,int sizey)
{
	__asm__ __volatile__(
	"shll $16,%%ecx\n\t"
	"shll $16,%%edx\n\t"
	"addl %%esi,%%ecx\n\t"
	"addl %%edi,%%edx\n\t"
	"int $0x40"
	:/*no output*/
	:"a"(7),"b"(p),"c"(sizex),"d"(x),"S"(sizey),"D"(y));
}

//------------------------------------------------------------------------------------------
//		draw filled rectangle in window
//------------------------------------------------------------------------------------------
extern inline void  gui_ksys_draw_filled_rectangle_window(int x,int y,int sizex,int sizey,DWORD color)
{
	__asm__ __volatile__(
	"shll $16,%%ebx\n\t"
	"shll $16,%%ecx\n\t"
	"addl %%esi,%%ebx\n\t"
	"addl %%edi,%%ecx\n\t"
	"int $0x40"
	:/*no output*/
	:"a"(13),"b"(x),"c"(y),"d"(color),"S"(sizex),"D"(sizey));
}

//------------------------------------------------------------------------------------------
//		get screen size
//------------------------------------------------------------------------------------------
extern inline DWORD  gui_ksys_get_screen_size(void)
{
	DWORD	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(14));

	return(value);
}

//------------------------------------------------------------------------------------------
//		get color of pixel in coordinates (x,y)
//------------------------------------------------------------------------------------------
extern inline DWORD  gui_ksys_get_color_pixel_window(DWORD coordinates)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(35),"b"(coordinates));

	return(value);
}

//------------------------------------------------------------------------------------------
//		get bits per pixel on the screen
//------------------------------------------------------------------------------------------
extern inline DWORD  gui_ksys_get_screen_bits_per_pixel(void)
{
	int	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(61),"b"(2));

	return(value);
}

//------------------------------------------------------------------------------------------
//		draw line in window
//------------------------------------------------------------------------------------------
extern inline void  gui_ksys_draw_line_window(int x1,int y1,int x2,int y2,DWORD color)
{
	__asm__ __volatile__(
	"shll $16,%%ebx\n\t"
	"shll $16,%%ecx\n\t"
	"addl %%esi,%%ebx\n\t"
	"addl %%edi,%%ecx\n\t"
	"int $0x40"
	:/*no output*/
	:"a"(38),"b"(x1),"c"(y1),"d"(color),"S"(x2),"D"(y2));
}

//------------------------------------------------------------------------------------------
//		get standart colors table
//------------------------------------------------------------------------------------------
extern inline void  gui_ksys_get_standart_colors_table(char *buf)
{
	__asm__ __volatile__(
	"int $0x40"
	:/*no output*/
	:"a"(48),"b"(3),"c"(buf),"d"(40));
}

//------------------------------------------------------------------------------------------
//		get time from start system to current in 1/100 sec.
//------------------------------------------------------------------------------------------
extern inline DWORD  gui_ksys_get_ticks(void)
{
	DWORD	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(26),"b"(9));

	return(value);
}

//------------------------------------------------------------------------------------------
//		initialize heap of memory
//------------------------------------------------------------------------------------------
extern inline DWORD  gui_ksys_init_user_heap(void)
{
	DWORD	value;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(value)
	:"a"(68),"b"(11));

	return(value);
}

//------------------------------------------------------------------------------------------
//		alloctae size bytes of user memory
//------------------------------------------------------------------------------------------
extern inline void* gui_ksys_malloc(DWORD size)
{
  void *value;

  __asm__ __volatile__(
      "int $0x40"	:"=a"(value)
	:"a"(68),"b"(12),"c"(size)
	:"memory");

  return(value);
}

//------------------------------------------------------------------------------------------
//		free pointer of memory
//------------------------------------------------------------------------------------------
extern inline void gui_ksys_free(void *mem)
{
     __asm__ __volatile__(
	"int $0x40"
	:
	:"a"(68),"b"(13),"c"(mem)
	:"memory");
}

//------------------------------------------------------------------------------------------
//		reallocate of memory
//------------------------------------------------------------------------------------------
extern inline void* gui_ksys_realloc(DWORD new_size,void *old_mem)
{
	void *new_mem;
     __asm__ __volatile__(
	"int $0x40"
	:"=a"(new_mem)
	:"a"(68),"b"(20),"c"(new_size),"d"(old_mem)
	:"memory");

	return(new_mem);
}


//------------------------------------------------------------------------------------------
//		load user mode DLL
//------------------------------------------------------------------------------------------
extern inline void* gui_ksys_load_dll(char *path)
{
	void	*dll_export;

     __asm__ __volatile__(
	"int $0x40"
	:"=a"(dll_export)
	:"a"(68),"b"(19),"c"(path));

	return(dll_export);
}

//------------------------------------------------------------------------------------------
//		create thred
//------------------------------------------------------------------------------------------
extern inline void* gui_ksys_create_thread(DWORD *thread_eip,DWORD *thread_esp)
{
	void	*thread_TID;

     __asm__ __volatile__(
	"int $0x40"
	:"=a"(thread_TID)
	:"a"(51),"b"(1),"c"(thread_eip),"d"(thread_esp)
	:"memory");

	return(thread_TID);
}

//------------------------------------------------------------------------------------------
//		acces to files input output
//------------------------------------------------------------------------------------------
extern inline DWORD gui_ksys_files_io(fileio_t *f,DWORD	value)
{
	DWORD	err_status;

     __asm__ __volatile__(
	"int $0x40"
	:"=a"(err_status),"=b"(value)
	:"a"(70),"b"(f));

	return(err_status);
}

//------------------------------------------------------------------------------------------
//		debug board output
//------------------------------------------------------------------------------------------
extern inline void gui_ksys_debug_out(int c)
{
     __asm__ __volatile__(
	"int $0x40"
	:
	:"a"(63),"b"(1),"c"(c));
}

//------------------------------------------------------------------------------------------
//		KolibriOS system exit program
//------------------------------------------------------------------------------------------
extern inline void gui_ksys_exit(int value)
{
     __asm__ __volatile__(
	"int $0x40"
	:
	:"a"(-1),"b"(value));
}

