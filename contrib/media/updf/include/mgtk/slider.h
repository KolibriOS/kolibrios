#ifndef __MGTK_SLIDER_H
#define __MGTK_SLIDER_H

#include<mgtk/widget.h>
#include<mgtk/button.h>

#define slider_Horizontal		0
#define slider_Vertical			1
#define slider_Solid			2

class GSlider: public GWidget
{
public:
 GSlider(GRect * r,int mode,unsigned long min,unsigned long max,unsigned long cur);
 virtual ~GSlider();
 virtual void DrawWidget();
 virtual void HandleEvent(GEvent *);
 virtual void VSetMin(unsigned long);
 virtual void VSetMax(unsigned long);
 virtual void VSetCur(unsigned long);
 virtual void SetMin(unsigned long);
 virtual void SetMax(unsigned long);
 virtual void SetCur(unsigned long);
 virtual unsigned long GetCur();
 virtual unsigned long GetMin();
 virtual unsigned long GetMax();
private:
 unsigned long SMode,SMin,SMax,SCur;
};

class GPercentSlider: public GSlider
{
public:
 GPercentSlider(GRect * r,int percent);
 virtual ~GPercentSlider();
 virtual void DrawWidget();
 virtual void HandleEvent(GEvent *);
private:
 char _txt[10];   /* xxx % */
};

class GScroll: public GGroup
{
public:
 GScroll(GRect * r,int mode,unsigned long min,unsigned long max,unsigned long cur,
     unsigned long _delta);
 virtual ~GScroll();
 virtual void HandleEvent(GEvent *);
private:
 GButton * b1,* b2;
 GSlider * slider;
 unsigned long delta;
};

#endif
