/* Rocket Forces
 * Filename: objects.h
 * Version 0.1
 * Copyright (c) Serial 2007
 */


void ppx(int x, int y, int color);
void pline(int x1, int y1, int x2, int y2, int color);
void draw4(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int color);


class cBuilding
{
public:
	void Draw(int x, int y, int color)
	{
		draw4(x, y, x+10, y-5, x+20, y, x, y, color);
		draw4(x, y, x+20, y, x+20, y+11, x, y+11, color);
		draw4(x+3, y+3, x+8, y+3, x+8, y+8, x+3, y+8, color);
		draw4(x+15, y+3, x+20, y+3, x+20, y+11, x+15, y+11, color);
	}
};

class cCursor
{
public:
	cCursor()
	{
		cx = -1;
		cy = -1;
	}
	void Delete(int color)
	{
		ppx(cx, cy, color);

		ppx(cx-3, cy, color);
		ppx(cx-4, cy, color);
		ppx(cx-5, cy, color);
		ppx(cx, cy-3, color);
		ppx(cx, cy-4, color);
		ppx(cx, cy-5, color);
		ppx(cx-1, cy-3, color);
		ppx(cx-2, cy-3, color);
		ppx(cx-3, cy-3, color);
		ppx(cx-3, cy-2, color);
		ppx(cx-3, cy-1, color);
		ppx(cx+1, cy-3, color);

		ppx(cx+3, cy, color);
		ppx(cx+4, cy, color);
		ppx(cx+5, cy, color);
		ppx(cx, cy+3, color);
		ppx(cx, cy+4, color);
		ppx(cx, cy+5, color);
		ppx(cx+1, cy+3, color);
		ppx(cx+2, cy+3, color);
		ppx(cx+3, cy+3, color);
		ppx(cx+3, cy+2, color);
		ppx(cx+3, cy+1, color);
		ppx(cx+1, cy-3, color);
		ppx(cx-1, cy+3, color);
	}
	void Draw(int x, int y, int color)
	{
		if (cx!=x || cy!=y)
		{
			Delete(BG_COLOR);
			cx = x;
			cy = y;
			ppx(cx, cy, color);

			ppx(cx-3, cy, color);
			ppx(cx-4, cy, color);
			ppx(cx-5, cy, color);
			ppx(cx, cy-3, color);
			ppx(cx, cy-4, color);
			ppx(cx, cy-5, color);
			ppx(cx-1, cy-3, color);
			ppx(cx-2, cy-3, color);
			//ppx(cx-3, cy-3, color);
			ppx(cx-3, cy-2, color);
			ppx(cx-3, cy-1, color);
			ppx(cx+1, cy-3, color);

			ppx(cx+3, cy, color);
			ppx(cx+4, cy, color);
			ppx(cx+5, cy, color);
			ppx(cx, cy+3, color);
			ppx(cx, cy+4, color);
			ppx(cx, cy+5, color);
			ppx(cx+1, cy+3, color);
			ppx(cx+2, cy+3, color);
			//ppx(cx+3, cy+3, color);
			ppx(cx+3, cy+2, color);
			ppx(cx+3, cy+1, color);
			ppx(cx+1, cy-3, color);
			ppx(cx-1, cy+3, color);
		}
	}
private:
	int cx, cy;
};

class cExplode
{
public:
	cExplode()
	{
		cx = -1;
		cy = -1;
	}
	void Enable(int cx, int cy)
	{
		step = 1;
		cExplode::cx = cx;
		cExplode::cy = cy;
	}
	int IsEnabled(void)
	{
		if (cx==-1 && cy==-1)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	void Draw(int r, int color)
	{
		int d=3-2*r, x=-1, y=r;
		while (x++<y)
		{
			ppx(cx+x, cy+y, color);
			ppx(cx+x, cy-y, color);
			ppx(cx-x, cy+y, color);
			ppx(cx-x, cy-y, color);
			ppx(cx+y, cy+x, color);
			ppx(cx+y, cy-x, color);
			ppx(cx-y, cy+x, color);
			ppx(cx-y, cy-x, color);
			if (d<0)
			{
				d += 4*x+6;
			}
			else
			{
				d += 4*(x-y)+10;
				y--;
			}
		}
	}
	void DrawNext(int color)
	{
		if (step>=EXP_RAD)
		{
			Disable(BG_COLOR);
		}
		else
		{
			Draw(step, BG_COLOR);
			Draw(++step, color);
		}
	}
	void Disable(int color)
	{
		Draw(step, color);
		cx = -1;
		cy = -1;
	}
	int cx, cy;
	int step;
};

class cRocket
{
public:
	cRocket()
	{
		cx = -1;
		cy = -1;
	}
	void Enable(int x, int y, int width, int height, int cx, int cy)
	{
		coord[0][0] = x;
		coord[0][1] = y;
		coord[1][0] = x+width;
		coord[1][1] = y;
		coord[2][0] = x+width;
		coord[2][1] = y+height;
		coord[3][0] = x;
		coord[3][1] = y+height;
		cRocket::cx = cx;
		cRocket::cy = cy;
		for (int j=0; j<4; j++)
		{
			dist[j] = sqrt((double)((coord[j][0]-cx)*(coord[j][0]-cx)+(coord[j][1]-cy)*(coord[j][1]-cy)));
			fi[j] = acos((coord[j][0]-cx)/dist[j]);
			if (coord[j][1]<cy) fi[j]*=-1;
			fire[j][0] = -1;
			fire[j][1] = -1;
		}
	}
	int IsEnabled(void)
	{
		if (cx==-1 && cy==-1)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	void Draw(int color)
	{
		draw4(coord[0][0], coord[0][1], coord[1][0], coord[1][1], coord[2][0], coord[2][1], coord[3][0], coord[3][1], color);
		if (fire[3][0]!=-1 || fire[3][1]!=-1)
		{
			ppx(fire[3][0], fire[3][1], BG_COLOR);
		}
	}
	void DrawAngle(int mx, int my, int color)
	{
		// Delete old rectangle
		Draw(BG_COLOR);

		// Draw new rectangle
		double a;
		long int dx = (long int) (cx-mx)*(cx-mx);
		long int dy = (long int) (cy-my)*(cy-my);
		if (cx!=mx || cy!=my)
		{
			a = -M_PI/2+acos((cx-mx)/sqrt((double)(dx+dy)));
			for (int j=0; j<4; j++)
			{
				coord[j][0] = round_int((double)(cx+dist[j]*cos(a+fi[j])));
				coord[j][1] = round_int((double)(cy+dist[j]*sin(a+fi[j])));
			}
		}
		draw4(coord[0][0], coord[0][1], coord[1][0], coord[1][1], coord[2][0], coord[2][1], coord[3][0], coord[3][1], color);
		for (int i=3; i>0; i--)
		{
			fire[i][0] = fire[i-1][0];
			fire[i][1] = fire[i-1][1];
		}
		fire[0][0] = cx;
		fire[0][1] = cy;
		for (int i=1; i<4; i++)
		{
			if (fire[i][0]!=-1 || fire[i][1]!=-1) ppx(fire[i][0], fire[i][1], SMOKE_COLOR);
		}
	}
	void Disable(int color)
	{
		for (int i=0; i<4; i++)
		{
			ppx(fire[i][0], fire[i][1], color);
			fire[i][0] = 0;
			fire[i][1] = 0;
		}
		Draw(color);
		cx = -1;
		cy = -1;
	}
	int cx, cy;
	protected:
	int coord[4][2];
	double dist[4];
	double fi[4];
	int fire[4][2];
};

class cBomb: public cRocket
{
public:
	void Draw(int color)
	{
		draw4(coord[0][0], coord[0][1], coord[1][0], coord[1][1], coord[2][0], coord[2][1], coord[3][0], coord[3][1], color);
		ppx(cx-1, coord[2][1]+1, color);
		ppx(cx, coord[2][1]+1, color);
		ppx(cx+1, coord[2][1]+1, color);
		for (int i=1; i<4; i++)
		{
			if (fire[i][0]!=-1 || fire[i][1]!=-1)
			{
				if (i==1)
				{
					ppx(fire[i][0]-1, fire[i][1], BG_COLOR);
					ppx(fire[i][0]+1, fire[i][1], BG_COLOR);
				}
				else
				{
					ppx(fire[i][0], fire[i][1], BG_COLOR);
				}
			}
		}
	}
	void DrawAngle(int mx, int my, int color)
	{
		// Delete old rectangle
		Draw(BG_COLOR);

		// Draw new rectangle
		for (int j=0; j<4; j++)
		{
			coord[j][1] += B_SPEED;
		}
		draw4(coord[0][0], coord[0][1], coord[1][0], coord[1][1], coord[2][0], coord[2][1], coord[3][0], coord[3][1], color);
		ppx(cx-1, coord[2][1]+1, color);
		ppx(cx, coord[2][1]+1, color);
		ppx(cx+1, coord[2][1]+1, color);
		for (int i=3; i>0; i--)
		{
			fire[i][0] = fire[i-1][0];
			fire[i][1] = fire[i-1][1];
		}
		fire[0][0] = cx;
		fire[0][1] = cy;
		for (int i=1; i<4; i++)
		{
			if (fire[i][0]!=-1 || fire[i][1]!=-1)
			{
				if (i==1)
				{
					ppx(fire[i][0]-1, fire[i][1], SMOKE_COLOR);
					ppx(fire[i][0]+1, fire[i][1], SMOKE_COLOR);
				}
				else
				{
					ppx(fire[i][0], fire[i][1], SMOKE_COLOR);
				}
			}
		}
	}
	void Disable(int color)
	{
		for (int i=0; i<4; i++)
		{
			ppx(fire[i][0], fire[i][1], color);
			fire[i][0] = 0;
			fire[i][1] = 0;
		}
		Draw(color);
		cx = -1;
		cy = -1;
	}
};

class cGun: public cRocket
{
public:
	void Enable(int x, int y, int width, int height, int cx, int cy)
	{
		old_mx = -1;
		old_my = -1;
		coord[0][0] = x;
		coord[0][1] = y;
		coord[1][0] = x+width;
		coord[1][1] = y;
		coord[2][0] = x+width;
		coord[2][1] = y+height;
		coord[3][0] = x;
		coord[3][1] = y+height;
		cRocket::cx = cx;
		cRocket::cy = cy;
		for (int j=0; j<4; j++)
		{
			dist[j] = sqrt((double)((coord[j][0]-cx)*(coord[j][0]-cx)+(coord[j][1]-cy)*(coord[j][1]-cy)));
			fi[j] = acos((coord[j][0]-cx)/dist[j]);
			if (coord[j][1]<cy) fi[j]*=-1;
		}
	}
	void DrawAngle(int mx, int my, int color)
	{
		if (old_mx!=mx || old_my!=my)
		{
			// Delete old rectangle
			old_mx = mx;
			old_my = my;
			Draw(BG_COLOR);

			// Draw new rectangle
			double a;
			long int dx = (long int)(cx-mx)*(cx-mx);
			long int dy = (long int)(cy-my)*(cy-my);
			if (my<=cy && (cx!=mx || cy!=my))
			{
				a = -M_PI/2+acos((cx-mx)/sqrt((double)(dx+dy)));
				for (int j=0; j<4; j++)
				{
					coord[j][0] = round_int((double)(cx+dist[j]*cos(a+fi[j])));
					coord[j][1] = round_int((double)(cy+dist[j]*sin(a+fi[j])));
				}
			}
			draw4(coord[0][0], coord[0][1], coord[1][0], coord[1][1], coord[2][0], coord[2][1], coord[3][0], coord[3][1], color);
			draw4(cx-14, cy+2, cx+14, cy+2, cx+14, cy+18, cx-14, cy+18, color);
		}
	}
protected:
	int old_mx, old_my;
};

class cCross
{
public:
	cCross()
	{
		x = -1;
		y = -1;
	}
	void Enable(int x, int y)
	{
		cCross::x = x;
		cCross::y = y;
		size = 2;
	}
	int IsEnabled(void)
	{
		if (x == -1 && y == -1)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	void Draw(int color)
	{
		pline(x-size, y, x+size, y, color);
		pline(x, y-size, x, y+size, color);
		ppx(x, y, BG_COLOR);
	}
	void Disable(int color)
	{
		pline(x-size, y, x+size, y, color);
		pline(x, y-size, x, y+size, color);
		x = -1;
		y = -1;
	}
	int x, y;
protected:
	int size;
};


void ppx(int x, int y, int color)
{
	if (x >= 0 && x < WINDOW_WIDTH &&  y >= 0 && y < WINDOW_HEIGHT) kos_PutPixel(x, y, color);
}

void pline(int x1, int y1, int x2, int y2, int color)
{
	kos_DrawLine(x1, y1, x2, y2, color);
}


void draw4(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int color)
{
	pline(x1, y1, x2, y2, color);
	pline(x2, y2, x3, y3, color);
	pline(x3, y3, x4, y4, color);
	pline(x4, y4, x1, y1, color);
}
