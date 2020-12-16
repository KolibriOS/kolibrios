#ifndef __MGTK_EDITOR_H
#define __MGTK_EDITOR_H

#include<mgtk/widget.h>
#include<mgtk/slider.h>

class GClipboard
{
public:
 typedef enum {
     CL_EMPTY=0,
     CL_BINARY=1,
     CL_IMAGE=2,
     CL_TEXT=3,
 } clip_data_type_t;
 GClipboard();
 virtual ~GClipboard();
 virtual void * GetData();
 virtual clip_data_type_t GetDataType();
 virtual unsigned long GetDataSize();
 virtual void SetClipData(clip_data_type_t type,void * ptr,unsigned long size);
 virtual void Empty();
private:
 void * buf_location;
 unsigned long size;
 clip_data_type_t data_type;
};

#define INVALID_EDITOR_OFFSET	0xffffffff

class GEditorCore
{
public:
 GEditorCore(unsigned long SizeLim);
 virtual ~GEditorCore();
 virtual int ExpandBuffer(int delta);
 virtual void Delete(unsigned long offset,unsigned long size);
 virtual void Cut(unsigned long offset,unsigned long size,
     GClipboard * Clip);
 virtual void Copy(unsigned long offset,unsigned long size,
     GClipboard * Clip);
 virtual void Paste(unsigned long offset,GClipboard * Clip);
 virtual unsigned long Search(unsigned long from_off,char * keyword,int name_len);
 virtual int Replace(unsigned long from_off,char * keyword,char * replace_to,
     int kword_len,int rt_len,bool ReplaceAll);
 virtual unsigned long GetTextSize();
 virtual char * GetTextBuffer();
 virtual void Empty();
 virtual bool IsEmpty();
 virtual void SetCaseSensitive(bool);
 virtual void InsertCharAt(unsigned long off,char c);
private:
 char * text_buffer;
 unsigned long text_size,buffer_size,size_limit;
 bool case_sensitive;
};

#endif
