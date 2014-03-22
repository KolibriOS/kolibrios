#ifndef __MGTK_EVENT_H
#define __MGTK_EVENT_H

typedef struct {
    int	evType;
    union {
	struct {
	    int evCommand;
	    int evMessage;
	    void * info;
	    unsigned long info_long;
	    unsigned char info_char;
	} command;
	struct {
	    int xMouse;
	    int yMouse;
	    int bMouse;
	} mouse;
	struct {
	    int keyId;
	} key;
	struct {
	    int butId;
	} button;
    } type;
} GEvent;

#define GEVENT_NOTHING		0
#define GEVENT_COMMAND		1
#define GEVENT_MOUSE		2
#define GEVENT_CLICK		3
#define GEVENT_KEYBOARD		4
#define GEVENT_IDLE		5
#define GEVENT_BUTTON		6

#define cm_Ignore	0
#define cm_Disable	1
#define cm_Enable	2
#define cm_Click	3
#define cm_Paint	4
#define cm_Quit		5
#define cm_Clicked	6
#define cm_Scroll	7
#define cm_Radio	8
#define cm_Scrollup	9
#define cm_Scrolldown	10
#define cm_KillMenu	11

#endif
