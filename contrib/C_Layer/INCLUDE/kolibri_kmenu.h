#ifndef KOLIBRI_KMENU_H
#define KOLIBRI_KMENU_H

#define KMENUITEM_NORMAL    0
#define KMENUITEM_SUBMENU   1
#define KMENUITEM_SEPARATOR 2

#define KMENUITEM_MAINMENU   0x80000000

#define KMENUITEM_SEPARATOR_WIDTH 10//170
#define KMENUITEM_SEPARATOR_HEIGHT 2

#define KMENU_LBORDER_SIZE 2
#define KMENU_DBORDER_SIZE 1

typedef struct {
	int type;
	uint32_t color;
	union {
		uint32_t bgcolor;
		void *buffer;
	};
} t_font;

typedef struct __attribute__ ((__packed__)) {
	uint16_t left, top, right, bottom;
} t_rect;

typedef struct kmenuitem_t{
	int type;
	char *text;
	union {
		void *submenu;
		//kmenuitem_callback_t callback;
		size_t btnid;
	};

	void (*paint)(struct kmenuitem_t *item, t_rect *rc) __attribute__((__stdcall__));

	int is_focused;
	int is_enabled;
	int is_visible;
	size_t style;

	int font_width, font_height;
	t_font font;

	size_t left, top;
	size_t width, height, pref_width, pref_height;
	size_t margin_left, margin_right, margin_top, margin_bottom;
	size_t padding_left, padding_right, padding_top, padding_bottom;
} kmenuitem_t;

typedef struct ksubmenu_t{
	kmenuitem_t **items;
	int count;

	void (*paint)(struct ksubmenu_t *menu) __attribute__((__stdcall__));

	int is_opened;
	int focus_idx;

	size_t width, height;
	size_t left, top;
	int tid;
	int parent_wnd, parent_tid, submenu_tid, self_wnd;
	int level;
	size_t return_btnid;

	int set_new_padding, set_new_margin;
	t_rect items_padding;
	t_rect items_margin;

	struct ksubmenu_t *parent;
} ksubmenu_t;

typedef struct {
	kmenuitem_t **submenu;
	int count;

	size_t width, height;
	size_t left, top;

	int line_height;

	int focus_idx;
	int submenu_tid;
} kmenu_t;


extern int kolibri_kmenu_init(void);

extern void (*kmainmenu_draw)(ksubmenu_t *) __attribute__((__stdcall__));
extern void (*kmainmenu_dispatch_cursorevent)(ksubmenu_t *) __attribute__((__stdcall__));
extern void (*kmenu_init)(void *) __attribute__((__stdcall__));
extern ksubmenu_t* (*ksubmenu_new)() __attribute__((__stdcall__));
extern void (*ksubmenu_add)(ksubmenu_t *, kmenuitem_t *) __attribute__((__stdcall__));
extern kmenuitem_t* (*kmenuitem_new)(uint32_t, const char *, uint32_t) __attribute__((__stdcall__));
extern kmenuitem_t* (*kmenuitem__submenu_new)(uint32_t, const char *, ksubmenu_t *) __attribute__((__stdcall__));

#endif /* KOLIBRI_KMENU_H */
