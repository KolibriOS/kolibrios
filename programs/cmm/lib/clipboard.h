#ifndef INCLUDE_CLIPBOARD_H
#define INCLUDE_CLIPBOARD_H
#print "[include <clipboard.h>]\n"

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

struct buffer_data
{
	dword	size;
	dword	type;
	dword	encoding;
	dword	content;
};


struct Clipboard {
	buffer_data slot_data;
	dword GetSlotCount();
	dword GetSlotData( ECX);
	dword SetSlotData( EDX, ESI);
	dword DelLastSlot();
	dword ResetBlockingBuffer();
};

dword Clipboard::GetSlotCount()
{
	$mov eax, 54 
	$mov ebx, 0
	$int 0x40
}

#define SLOT_DATA_TYPE_TEXT 0
#define SLOT_DATA_TYPE_IMAGE 1
#define SLOT_DATA_TYPE_RAW 2
#define SLOT_DATA_TYPE_RESERVED 3
dword Clipboard::GetSlotData( ECX) //ECX = slot number
{
	dword result;
	$mov eax, 54 
	$mov ebx, 1
	$int 0x40
	result = EAX;
	slot_data.size = DSDWORD[result];
	slot_data.type = DSDWORD[result+4];
	slot_data.encoding = DSDWORD[result+8];
	if (slot_data.type == SLOT_DATA_TYPE_TEXT) slot_data.content = result+12;
	else slot_data.content = result+8;
	return result;
}

dword Clipboard::SetSlotData( ECX, EDX) //ECX = data size, EDX - pointer to data
{
	$mov eax, 54 
	$mov ebx, 2
	$int 0x40
}

dword Clipboard::DelLastSlot()
{
	$mov eax, 54 
	$mov ebx, 3
	$int 0x40
}

dword Clipboard::ResetBlockingBuffer()
{
	$mov eax, 54 
	$mov ebx, 4
	$int 0x40
}

#endif