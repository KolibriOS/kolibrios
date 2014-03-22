#ifndef __MGTK_BUTTON_H
#define __MGTK_BUTTON_H

#include<mgtk/widget.h>
#include<mgtk/pen.h>

#define bs_Normal		0
#define bs_DontDraw		1

class GTextButton;

class GButton: public GWidget
{
    friend class GTextButton;
public:
    GButton(GRect *,int flags,unsigned long color,int cmd);
    virtual ~GButton(void);
    virtual void HandleEvent(GEvent *);
    virtual void DrawWidget();
    virtual void ClickProc();
    virtual void SetCommand(int);
private:
    int bId;
    int bFlags;
    int bCmd;
    unsigned long bColor;
};

class GRadioButton;

class GCheckBox: public GWidget
{
    friend class GRadioButton;
public:
    GCheckBox(GRect *,bool);
    virtual ~GCheckBox();
    virtual void DrawWidget();
    virtual void HandleEvent(GEvent *);
    bool Checked();
    void Check(bool);
private:
    bool state;
};

class GRadioButton: public GCheckBox
{
public:
    GRadioButton(GRect *,bool);
    virtual ~GRadioButton();
    virtual void DrawWidget();
};

class GRadioGroup: public GGroup
{
public:
 GRadioGroup(GRect *,int);
 virtual ~GRadioGroup();
 virtual void HandleEvent(GEvent *); 
 virtual int ActualButton();
private:
 int actual;
 GRadioButton * _current_click;
};

class GTextButton: public GButton
{
public:
 GTextButton(GRect *,char *,int);
 virtual ~GTextButton();
 virtual void DrawWidget();
private:
 char * txt;
};

class GButtonAllocator
{
public:
 GButtonAllocator(int max);
 virtual ~GButtonAllocator();
 int New();
 void Del(int);
private:
 unsigned char * gbutton_bitmap;
 int nr_buttons;
};

extern GButtonAllocator * OSButtonAlloc;

class GImageButton: public GButton
{
public:
 GImageButton(GRect *,struct GImageData *,int);
 virtual ~GImageButton();
 virtual void DrawWidget();
private:
 struct GImageData * data;
};

#endif
