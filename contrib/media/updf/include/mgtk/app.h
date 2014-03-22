#ifndef __MGTK_APP_H
#define __MGTK_APP_H

#include<mgtk/widget.h>

class GApplication: public GGroup
{
public:
    GApplication(GRect *);
    virtual ~GApplication();
    virtual void SetCaption(char *);
    virtual void Init();
    virtual void Run();
    virtual void Done();
    virtual void DrawWidget();
private:
    char * Caption;
    GPoint priv_origin,priv_size;
    unsigned long body,grab,frame;
};

#endif
