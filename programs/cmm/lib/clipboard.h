#ifndef INCLUDE_CLIPBOARD_H
#define INCLUDE_CLIPBOARD_H
#print "[include <clipboard.h>]\n"

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif


#ifndef INCLUDE_FILESYSTEM_H
#include "../lib/fs.h"
#endif

//===================================================//
//                                                   //
//       Kolibri Clipboard System Functions          //
//                                                   //
//===================================================//

#define SLOT_DATA_TYPE_TEXT 0
#define SLOT_DATA_TYPE_TEXT_BLOCK 1
#define SLOT_DATA_TYPE_IMAGE 2
#define SLOT_DATA_TYPE_RAW 3

inline fastcall dword Clipboard__GetSlotCount()
{
	$mov eax, 54 
	$mov ebx, 0
	$int 0x40
}

inline fastcall dword Clipboard__GetSlotData( ECX) //ECX = slot number
{
	$mov eax, 54 
	$mov ebx, 1
	$int 0x40
}

inline fastcall dword Clipboard__SetSlotData( ECX, EDX) //ECX = data size, EDX - pointer to data
{
	$mov eax, 54 
	$mov ebx, 2
	$int 0x40
}

inline fastcall dword Clipboard__DeleteLastSlot()
{
	$mov eax, 54 
	$mov ebx, 3
	$int 0x40
}

inline fastcall dword Clipboard__ResetBlockingBuffer()
{
	$mov eax, 54 
	$mov ebx, 4
	$int 0x40
}

//===================================================//
//                                                   //
//               Some useful patterns                //
//                                                   //
//===================================================//

:void Clipboard__CopyText(dword _text)
{
int size_buf;
dword buff_data;

	size_buf = strlen(_text) + 12;
	buff_data = malloc(size_buf);
	ESDWORD[buff_data] = size_buf;
	ESDWORD[buff_data+4] = SLOT_DATA_TYPE_TEXT;
	ESDWORD[buff_data+8] = 1; //encoding 0=UTF, 1=866, 2=1251
	strcpy(buff_data+12, _text);

	Clipboard__SetSlotData(size_buf, buff_data);
	if (EAX!=0) notify("'Error while copying to clipboard!'E");

	buff_data = free(buff_data);
}


#endif