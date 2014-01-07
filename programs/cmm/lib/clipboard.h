struct buffer_data
{
	dword	size;
	dword	type;
	dword	encoding;
	byte	buffer_data[4096];
};


struct Clipboard {
	buffer_data data;
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

dword Clipboard::GetSlotData( ECX) //ECX = slot number
{
	$mov eax, 54 
	$mov ebx, 1
	$int 0x40
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
	$mov ebx, 3
	$int 0x40
}