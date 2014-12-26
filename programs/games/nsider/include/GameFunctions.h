void Update() {
	int x=0;
	int y=0;
	int X0=0;
	int X1=0;
	int CurColor=0;

	//LINE DRAW
	if (DRAW_TECH==1) {
		//Buffer to Buffer
		for (x = 0; x < Width; x++) {
			for (y = 0; y < Height; y++) {
				BufferCarry2[x][y] = BufferDraw[x][y];
			}
		}
		//Convert Analyx to Drawmask Matrix
		for (x=0; x<Width; x++) {
			for (y=0; y<Height; y++) {
				if (Analyx[x][y]==BufferCarry2[x][y]) {
					Analyx[x][y]=1;
				} else {
					Analyx[x][y]=0;
				}
			}
		}

		for (x=0; x<Width; x++) {
			for (y=0; y<Height; y++) {
				if (Analyx[x][y]==1) continue;
				CurColor=BufferCarry2[x][y];
				//try vertical line
				int LineY=y;
				while (LineY<Height && BufferCarry2[x][LineY]==CurColor) LineY++;
				LineY--;

				//try horizontal line
				int LineX=x;
				while (LineX<Width && BufferCarry2[LineX][y]==CurColor) LineX++;
				LineX--;

				//chosing the greatest area and draw
				int LineYLen=LineY-y+1;
				int LineXLen=LineX-x+1;

				if (LineXLen>=LineYLen) {
					_ksys_line(x+OffsetX,y+OffsetY,LineX+OffsetX,y+OffsetY,CurColor);
					int i=0;
					for (i=x; i<=LineX; i++) Analyx[i][y]=1;
					continue;
				}
				_ksys_line(x+OffsetX,y+OffsetY,x+OffsetX,LineY+OffsetY,CurColor);
				int i=0;
				for (i=y; i<=LineY; i++) Analyx[x][i]=1;
			}
		}

		//From Buffer to Analyx and Reset Buffer
		for (x = 0; x < Width; x++) {
			for (y = 0; y < Height; y++) {
				Analyx[x][y]=BufferCarry2[x][y];
				BufferDraw[x][y]=GLOBAL_BACKGROUNDCOLOR;
			}
		}
	}
	//FULL FRAME DRAW
	if (DRAW_TECH==0) {
		for (x = 0; x < Width; x++) {
			for (y = 0; y < Height; y++) {
				BufferCarry[y][x][0] = BufferDraw[x][y]%256;
				BufferDraw[x][y]/=256;
				BufferCarry[y][x][1]=BufferDraw[x][y]%256;
				BufferDraw[x][y]/=256;
				BufferCarry[y][x][2]=BufferDraw[x][y]%256;
				BufferDraw[x][y]=GLOBAL_BACKGROUNDCOLOR;
			}
		}
		DrawScreen (OffsetX,OffsetY,Width,Height,BufferCarry);
	}
	//Drawing Level
	int i=0;
	for (i=0; i<Objects; i++) {
		if (DataBase[i][0]==1) {
			DrawBlock (DataBase[i][1],DataBase[i][2],GLOBAL_BLOCKCOLOR);
		}
		if (DataBase[i][0]==2) {
			DrawPit (DataBase[i][1],DataBase[i][2],GLOBAL_PITCOLOR);
		}
		if (DataBase[i][0]==3) {
			DrawBatut (DataBase[i][1],DataBase[i][2],GLOBAL_BATUTCOLOR);
		}
		if (DataBase[i][0]==4) {
			DrawFlag (DataBase[i][1],DataBase[i][2],GLOBAL_FLAGCOLOR);
		}
		DataBase[i][1]-=GLOBAL_SPEED;
		if (Objects>0 && DataBase[0][1]<120) isRestart=1;
	}
	CleanDataBase();

}

void Jump () {
	if (Key==' ') HeroFly=-13;
}


char CheckCollision() {
	int i=0;
	for (i=0; i<Objects; i++) {
		if (Abs(DataBase[i][1]-HeroX)>60) continue;
		int j=0;
		for (j=0;j<HeroSides; j++) {
			int PointX=CollideVerts[j][0];
			int PointY=CollideVerts[j][1];
			if (DataBase[i][0]==2 && PointX>=DataBase[i][1]+6 && PointX<=DataBase[i][1]+30-6 && PointY>=DataBase[i][2]+10 && PointY<=DataBase[i][2]+48) {
				return 1;
			}
			if (DataBase[i][0]==3 && PointX>=DataBase[i][1] && PointX<=DataBase[i][1]+43 && PointY>=DataBase[i][2]+25 && PointY<=DataBase[i][2]+48) {
				HeroFly=-25;
				return 0;
			}
			if (DataBase[i][0]==4 && PointX>=DataBase[i][1] && PointX<=DataBase[i][1]+43 && PointY>=DataBase[i][2] && PointY<=DataBase[i][2]+48) {
				DrawText (10,510,"checkpoint",GLOBAL_FLAGCOLOR);
				GLOBAL_CHECKPOINT=CurrentCheck;
				SPAWN_Y=DataBase[i][2];
			}

			if (DataBase[i][0]==1 && PointX>=DataBase[i][1] && PointX<=DataBase[i][1]+43) {
				if (PointY>=DataBase[i][2] && PointY<=DataBase[i][2]+43) {
					if (HeroY>=DataBase[i][2] && HeroX<=DataBase[i][1]+6) {
						return 1;
					}
					if (HeroFly<0) {
						return 1;
					}
					//correcting edges
					HeroAngle=0;
					if (HeroSides%2==0) HeroAngle=360/HeroSides/2;
					HeroY=DataBase[i][2]-DeltaH[HeroSides-3];
					HeroFly=0;
					Jump();
					return 0;
				 } else {
					if (DataBase[i][2]-HeroY<HeroFly && HeroFly>0 && DataBase[i][2]-HeroY>0) HeroFly=DataBase[i][2]-HeroY;
				 }
			 }
		}
	}
	if (HeroY<500-DeltaH[HeroSides-3]) {
		HeroFly++;
	} else {
		HeroFly=0;
		HeroAngle=0;
		if (HeroSides%2==0) HeroAngle=360/HeroSides/2;
		HeroY=500-DeltaH[HeroSides-3];
		Jump();
	}
	return 0;
}
