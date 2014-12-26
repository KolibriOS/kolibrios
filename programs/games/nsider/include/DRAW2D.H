void draw_window() {
	_ksys_window_redraw(1);
	_ksys_draw_window(0, 0, ScreenX, ScreenY, 0, 0, 0, 0, 0);
	_ksys_window_redraw(2);
}

void DrawLine(int x0, int y0, int x1, int y1, int color) {
	int Temp = 0;
	int x=0;
	int y=0;

	//cases when line is out of screen
	if (y0 < 0 && y1 < 0) return;
	if (y0 >Height - 1 && y1>Height - 1) return;
	if (x0 < 0 && x1 < 0) return;
	if (x0>Width - 1 && x1>Width - 1) return;

	if ((x1 - x0) == 0) {
		if (y1 == min(y0, y1)) {
			Temp = y1;
			y1 = y0;
			y0 = Temp;
		}
		//correcting borders
		if (y0 < 0) y0 = 0;
		if (y1>Height - 1) y1 = Height - 1;
		for (y = y0; y <= y1; y++) {
			BufferDraw[x0][y] = color;
			if (x0+1<=Width-1) BufferDraw[x0+1][y] = color;
		}
	}
	else if ((y1 - y0) == 0) {
		if (x1 == min(x0, x1)) {
			Temp = x1;
			x1 = x0;
			x0 = Temp;
		}
		if (x0 < 0) x0 = 0;
		if (x1>Width - 1) x1 = Width - 1;
		for (x = x0; x < x1; x++) {
			BufferDraw[x][y0] = color;
			if (y0+1<=Height-1) BufferDraw[x][y0+1] = color;
		}
	}
	else {
		int PartX=0;
		int PartY=0;

		int LeftX=min(x0,x1);
		if (LeftX<0) LeftX=0;
		if (LeftX>Width-1) LeftX=Width-1;

		int RightX=max(x0,x1);
		if (RightX<0) RightX=0;
		if (RightX>Width-1) RightX=Width-1;

		int LeftY=min(y0,y1);
		if (LeftY<0) LeftY=0;
		if (LeftY>Height-1) LeftY=Height-1;

		int RightY=max(y0,y1);
		if (RightY<0) RightY=0;
		if (RightY>Height-1) RightY=Height-1;

		if (RightX-LeftX>=RightY-LeftY) {
			for (x=LeftX; x<=RightX; x++) {
				y=(y1-y0)*(x-x0)/(x1-x0)+y0;
				if (y<0) continue;
				BufferDraw[x][y]=color;
				if (y+1<=Height-1) BufferDraw[x][y+1]=color;
			}
		} else {
			for (y=LeftY; y<=RightY; y++) {
				x=(x1-x0)*(y-y0)/(y1-y0)+x0;
				if (x<0) continue;
				BufferDraw[x][y]=color;
				if (x+1<=Width-1) BufferDraw[x+1][y]=color;
			}
		}

	}
}

void DrawText(int x, int y, char Text[] , int color) {
	int i=0;
	for (i=0; Text[i]!='\0'; i++) {
		int j=0;
		unsigned char isMatch=0;
		for (j=0; Alphabet[j]!='\0'; j++) {
			if (Text[i]==Alphabet[j]) {
				isMatch=1;
				break;
			}
		}
		if (isMatch==0) {
			x+=17;
			continue;
		}
		int bitLen=15;
		int bitX=0;
		int bitY=0;
		for (bitX=0; bitX<bitLen; bitX++) {
			for (bitY=0; bitY<bitLen; bitY++) {
				if (*(AlphaGraphic[j]+bitY*bitLen+bitX)==1) {
					if (x+bitX>=0 && x+bitX<=(Width-1) && y+bitY>=0 && y+bitY<=(Height-1)) BufferDraw[x+bitX][y+bitY]=color;
				}
			}
		}
		x+=bitLen+2;
	}
}

void DrawPit (int x, int y, int color) {
	DrawLine (0+x,43+y,15+x,10+y,color);
	DrawLine (15+x,10+y,30+x,43+y,color);
	DrawLine (0+x,43+y,30+x,43+y,color);
}

void DrawBlock (int x, int y, int color) {
	DrawLine (0+x,0+y,40+x,0+y,color);
	DrawLine (40+x,0+y,40+x,40+y,color);
	DrawLine (40+x,40+y,0+x,40+y,color);
	DrawLine (0+x,40+y,0+x,0+y,color);
}

void DrawHero (int x, int y, int Verts, int InitAngle, int color) {
	int R=25;
	int PointX1=x+FloatToInt(R*sin(InitAngle));
	int PointY1=y-FloatToInt(R*cos(InitAngle));
	int PointX2=0;
	int PointY2=0;
	int i=0;
	for (i=0; i<Verts; i++) {
		InitAngle=(InitAngle+360/Verts)%360;
		PointX2=x+FloatToInt(R*sin(InitAngle));
		PointY2=y-FloatToInt(R*cos(InitAngle));
		DrawLine(PointX1,PointY1,PointX2,PointY2,color);
		PointX1=PointX2;
		PointY1=PointY2;
		CollideVerts[i][0]=PointX1;
		CollideVerts[i][1]=PointY1;

	}
}

void DrawBatut (int x, int y, int color) {
	DrawLine (x,y+20,x+43,y+20,color);
	DrawLine (x+43,y+20,x+43,y+43,color);
	DrawLine (x+43,y+43,x,y+43,color);
	DrawLine (x,y+43,x,y+20,color);
	DrawLine (x+10+2,y+35,x+30+2,y+35,color);
	DrawLine (x+10+2,y+35,x+20+2,y+27,color);
	DrawLine (x+20+2,y+27,x+30+2,y+35,color);

}
void DrawFlag (int x,int y, int color) {
	DrawLine (x,y,x,y+43,color);
	DrawLine (x,y+43,x+10,y+43,color);
	DrawLine (x,y,x+30,y,color);
	DrawLine (x+30,y,x+30,y+20,color);
	DrawLine (x,y+20,x+30,y+20,color);
	DrawText (x+10,y+4,"c",color);
}

void DrawScreen (int x,int y,int xsize, int ysize, void* image) {
	asm volatile("int $0x40"::"a"(7),"b"(image),"c"((xsize<<16)+ysize),"d"((x<<16)+y):"memory");
}

void DrawTitle (int x, int y, int color) {
	int i=0;
	//n
	for (i=0; i<=10; i++) {
		DrawLine (x,y+i,x+70,y+i,color);
		DrawLine (x+i,y,x+i,y+80, color);
		DrawLine (x+i+70,y+10,x+i+70,y+80,color);
	}
	//-
	for (i=0; i<=10; i++) {
		DrawLine (x+100,y+30+i,x+130,y+30+i,color);
	}
	//S
	for (i=0; i<=10; i++) {
		DrawLine (x+160,y+i,x+220, y+i, color);
		DrawLine (x+150+i,y+10,x+150+i,y+30,color);
		DrawLine (x+150,y+30+i,x+210,y+30+i,color);
		DrawLine (x+210+i,y+40,x+210+i,y+70,color);
		DrawLine (x+150,y+70+i,x+210,y+70+i,color);

	}
	//i
	for (i=0; i<=10; i++) {
		DrawLine (x+240,y+i,x+255,y+i,color);
		DrawLine (x+242+i,y+20,x+242+i,y+80,color);
	}
	//D
	for (i=0; i<=10; i++) {
		DrawLine (x+270,y+i,x+320,y+i,color);
		DrawLine (x+270+i,y,x+270+i,y+80,color);
		DrawLine (x+270,y+i+70,x+320,y+70+i,color);
		DrawLine (x+320+i,y+10,x+320+i,y+70,color);
	}
	//E
	for (i=0; i<=10; i++) {
		DrawLine (x+350,y+i,x+410,y+i, color);
		DrawLine (x+350,y+35+i,x+410,y+35+i,color);
		DrawLine (x+350,y+70+i,x+410,y+70+i,color);
	}
	//R
	 for (i=0; i<=10; i++) {
		DrawLine (x+430,y+i,x+480,y+i,color);
		DrawLine (x+430+i,y,x+430+i,y+80,color);
		DrawLine (x+480+i,y+10,x+480+i,y+40,color);
		DrawLine (x+430,y+40+i,x+480,y+40+i,color);
		DrawLine (x+480+i,y+52,x+480+i,y+80,color);
	}
}

void DrawPew (int x,int y,int frame,int color) {
	frame-=15;
	DrawLine (x,y-25-frame,x,y-30-frame,color);
	DrawLine (x,y+25+frame,x,y+30+frame,color);
	DrawLine (x+25+frame,y,x+30+frame,y,color);
	DrawLine (x-25-frame,y,x-30-frame,y,color);
	DrawLine (x+15+frame,y+15+frame,x+20+frame,y+20+frame,color);
	DrawLine (x-15-frame,y-15-frame,x-20-frame,y-20-frame,color);
	DrawLine (x-15-frame,y+15+frame,x-20-frame,y+20+frame,color);
	DrawLine (x+15+frame,y-15-frame,x+20+frame,y-20-frame,color);
}






