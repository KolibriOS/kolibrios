dword drawbuf;
void DrawBufInit()
{
	free(drawbuf);
	drawbuf = malloc(WB1.width * WB1.line_h +4 * 4 + 8); //+1 for good luck
	ESDWORD[drawbuf] = WB1.width;
	ESDWORD[drawbuf+4] = WB1.line_h;
}
void DrawBufFill()
{
	int i;
	for (i=0; i<WB1.width* WB1.line_h +4 * 4; i+=4) ESDWORD[drawbuf+i+8] = bg_color;
}
void DrawBufBar(dword x, y, w, h, color)
{
	int i, j;
	for (j=0; j<h; j++)
	{
		for (i = y+j*WB1.width+x*4; i<y+j*WB1.width+x+w*4 ; i+=4) ESDWORD[drawbuf+i+8] = color;
	}
}

char shift[]={8,8,4,4};
void DrawBufSkew(dword x, y, w, h)
{
	int i, j;
	stolbec++;
	for (j=0; j<=3; j++)
	{
		for (i = y+j*WB1.width+x+w+h*4; i>y+j*WB1.width+x+h-12*4 ; i-=4)
								ESDWORD[drawbuf+i+8] = ESDWORD[-shift[j]+drawbuf+i+8];
	}
}

void DrawBufAlignRight(dword x,y,w,h)
{
	int i, j, l;
	int content_width = stolbec * 6;
	int content_left = w - content_width / 2;
	for (j=0; j<h; j++)
	{
		for (i=j*w+w-x*4, l=j*w+content_width+x*4; (i>=j*w+content_left*4) && (l>=j*w*4); i-=4, l-=4)
		{
			ESDWORD[drawbuf+8+i] >< ESDWORD[drawbuf+8+l];
		}
	}
}


void DrawBufAlignCenter(dword x,y,w,h)
{
	int i, j, l;
	int content_width = stolbec * 6;
	int content_left = w - content_width / 2;
	for (j=0; j<h; j++)
	{
		for (i=j*w+content_width+content_left*4, l=j*w+content_width+x*4; (i>=j*w+content_left*4) && (l>=j*w*4); i-=4, l-=4)
		{
			ESDWORD[drawbuf+8+i] >< ESDWORD[drawbuf+8+l];
		}
	}
}


void TextGoDown(int left1, top1, width1)
{
	if (!stroka) DrawBar(WB1.left, WB1.top, WB1.width, 5, bg_color); //çàêðàøèâàåì ôîí íàä ïåðâîé ñòðîêîé
	if (top1>=WB1.top) && ( top1 < WB1.height+WB1.top-10)  && (!anchor)
	{
		if (text_align == ALIGN_CENTER) DrawBufAlignCenter(left1,top1,WB1.width,WB1.line_h);
		if (text_align == ALIGN_RIGHT) DrawBufAlignRight(left1,top1,WB1.width,WB1.line_h);
		PutPaletteImage(drawbuf+8, WB1.width, WB1.line_h, left1-5, top1, 32,0);
		DrawBufFill();
	}
	stroka++;
	if (blq_text) stolbec = 8; else stolbec = 0;
	if (li_text) stolbec = li_tab * 5;
}