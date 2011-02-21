#ifndef __HTML_h
#define __HTML_h

#include"parser.h"

template<class T> class CHTMLStack
{
public:
 CHTMLStack()
 {
  FirstItem=NULL;
  NR=0;
 }
 void Push(T * ff)
 {
  T * ff1;
  if(!ff) return;
  ff1=(T *)malloc(sizeof(T));
  memcpy((void *)ff1,(const void *)ff,sizeof(T));
  NR++;
  ff1->next=FirstElem;
  FirstElem=ff1;
 }
 T * Pop()
 {
  T * R;
  if(!NR) return NULL;
  NR--;
  R=FirstElem;
  FirstElem=R->next;
  return R;
 }
 void Free(T * x)
 {
  free(x);
 }
 ~CHTMLStack() 
 {
  T * __tmp;
  while((__tmp=Pop())) this->Free(__tmp);
 }
protected:
 T * FirstElem;
 int NR;
};

struct text_style {
 unsigned long flags;
 char * FontName;
 int FontSize;
 unsigned long color;
 struct text_style * next;
};

struct page_style {
 unsigned long background;
 unsigned long text;
 unsigned long link,alink,vlink;
 unsigned long flags;
 struct page_style * next;
};

struct image_properties {
 int width,height;
 int border; 
};

struct text_properties {
 char * FontName;
 int font_size;
 int color;
 unsigned long flags;
};

struct cpb_Text {
 struct text_properties prop;
 char * Text;
 int len;
};

struct cpb_Image {
 struct image_properties img;
 void * ImageData;
};

class CPageBuffer
{
public:
 CPageBuffer();
 ~CPageBuffer();
 virtual void AddText(char *,int,struct text_properties *);
 virtual void AddImage(char *,struct image_properties *); 
 virtual void Paint();
 virtual void Reset();
private:
 int nr_text,nr_images;
 struct cpb_Text ** _text;
 struct cpb_Image ** _images;
};

class CHTMLParser
{
public:
 CHTMLParser(CParser * par,CPageBuffer * buf);
 ~CHTMLParser();
 virtual void DefaultStyles();
 virtual void Parse();
 virtual void ParseHTML();
protected:
 CParser * parser;
 CPageBuffer * pgbuf;
 struct text_style text_style;
 struct page_style page_style;
};

#define FFLAG_ALIGNLEFT		0x00000001
#define FFLAG_ALIGNRIGHT	0x00000002
#define FFLAG_ALIGNCENTER	0x00000003
#define FFLAG_BOLD		0x00000004
#define FFLAG_ITALIC		0x00000008
#define FFLAG_UNDERLINE		0x00000010

#define PFLAG_ALIGNLEFT		0x00000001
#define PFLAG_ALIGNRIGHT	0x00000002
#define PFLAG_ALIGNCENTER	0x00000003
#define PFLAG_RAWMODE		0x00000004

#endif
