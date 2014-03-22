#ifndef __MGTK_WIDGET_H
#define __MGTK_WIDGET_H

#include<mgtk/event.h>
#include<mgtk/types.h>

#define wf_Visible		0x00000001
#define wf_Disabled		0x00000002
#define wf_Focused		0x00000004
#define wf_Selected		0x00000008

class GGroup;

class GWidget
{
public:
    GGroup *	Parent;
    GPoint	Origin;
    GPoint	Size;
    GWidget *	WNext;
    GWidget *	WPrev;
    unsigned long Flags;
    GWidget();
    virtual ~GWidget();
    virtual void HandleEvent(GEvent *);
    void Draw(void);
    virtual void DrawWidget();
    virtual void ToGlobal(GPoint *);
    virtual void ToLocal(GPoint *);
    virtual void Idle();
    virtual void SetBounds(GRect *);
    int MouseInWidget(int,int);
    void ClearEvent(GEvent *);
    virtual void Show();
    virtual void Hide();
    virtual void Select(bool);
    virtual bool IsGroup();
    inline bool IsDisabled() { return Flags&wf_Disabled; }
};

class GGroup: public GWidget
{
public:
    GRect	Clip;
    GWidget *	WFirst;
    GWidget * 	WSelect;
    GGroup();
    virtual ~GGroup();
    virtual void HandleEvent(GEvent *);
    virtual void DrawWidget();
    virtual void Insert(GWidget *);
    virtual void Remove(GWidget *);
    virtual void ForEach(void (*fn)(GGroup *,GWidget *,void *),void *);
    virtual void Show();
    virtual void Hide();
    virtual void Select(GWidget *,bool);
    virtual GWidget * Select();
    virtual bool IsGroup();
    virtual void BroadcastEvent(GEvent *);
    virtual void RepaintWhole();
};

#endif
