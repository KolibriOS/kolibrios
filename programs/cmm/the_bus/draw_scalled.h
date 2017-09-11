void DrawScaledBar(dword x,y,w,h,col) {
	DrawBar(
		x*SCALE, 
		y*SCALE, 
		w*SCALE, 
		h*SCALE, 
		col		
	);
}
void DrawScaledImage(dword image_pointer,x,y,w,h,offx,offy) {
	img_draw stdcall (
		image_pointer, 
		x*SCALE, 
		y*SCALE, 
		w*SCALE, 
		h*SCALE, 
		offx*SCALE, 
		offy*SCALE
	);	
}
void WriteScaledText(dword x,y,font,color,text) {
	WriteText(
		x*SCALE,
		y*SCALE,
		font + SCALE,
		color,
		text
	);
}