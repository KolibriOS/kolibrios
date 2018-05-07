/*
We have a long text and need to show it in block.
Normal line break '\n' must be applied.
Long lines should be breaked by word. 
TODO: scroll
*/
:int DrawTextViewArea(int x,y,w,h, dword buf_start, bg_col, text_col)
{
	dword write_start;
	dword buf_end;
	int line_h = 15;
	int label_length_max;
	int write_length;
	bool end_found;

	write_start = buf_start;
	buf_end = strlen(buf_start) + buf_start;
	label_length_max  = w / 8; // 8 big font char width

	loop() 
	{
		if (bg_col!=-1) DrawBar(x, y, w+1, line_h, bg_col);
		end_found = false;
		write_length = strchr(write_start, '\n') - write_start; //search normal line break
		if (write_length > label_length_max) || (write_length<=0) //check its position: exceeds maximum line length or not found
		{ 
			if (buf_end - write_start < label_length_max) //check does current line the last
			{ 
				write_length = buf_end - write_start;
				end_found = true;
			}
			else
			{
				for (write_length=label_length_max; write_length>0; write_length--) { //search for white space to make the line break
					if (ESBYTE[write_start+write_length] == ' ') {
						end_found = true;
						break;
					}
				}
			} 
			if (end_found != true) write_length = label_length_max; //no white space, so we write label_length_max
		}
		ESI = write_length; //set text length attribute for WriteText()
		WriteText(x, y, 0x10, text_col, write_start);
		// if (editpos >= write_start-buf_start) && (editpos <= write_start-buf_start + write_length) {
		// 	WriteTextB(-write_start+buf_start+editpos * 8 + x - 5 +1, y, 0x90, 0xFF0000, "|");
		// }
		write_start += write_length + 1;
		y += line_h;
		if (write_start >= buf_end) break;
	}
	if (bg_col!=-1) DrawBar(x,y,w+1,h-y+line_h-4,bg_col);
	return y+line_h;
}
