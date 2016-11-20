int verthoriz=0;
int shownums=1;

char str[MAX_PATH];
word attr[MAX_PATH];

printstrat(int x,y,size,_text,_attr)
{
	int i;
	for (i=0; i<size; i++)
	{
		con_set_flags stdcall (ESBYTE[i*2+_attr]);
		con_set_cursor_pos stdcall (x+i,y);
		con_write_string stdcall (_text+i, 1);		
	}
}


drawline(int x,y,width,linex,s1,s2,ln)
{
	int i;
	int s1l;
	int s2l;
	int c1,c2;
	char temp[10];
	int x2,width2;


	s1l=-1;
	s2l=-1;
	if(s1!=-1)s1l=strlen(s1);
	if(s2!=-1)s2l=strlen(s2);

	FillMemory(#attr,width*2,0x1F);
	str[0]='║'; str[x+width-1]='║';

	if(s1==-1)
	{
		FillMemory(#str[1],width-2,'·');
		FillMemory(#attr[2],width-2*2,7);
	}else{//строка присутствует
		x2=1;
		width2=width;
		if(shownums)
		{
			wsprintf(#temp,"%6d",ln+1);
			for(i=0;i<6;i++)
			{
				str[i+1]=temp[i];
				attr[i+1]=0x19;
			}
			str[7]='│';
			x2+=7;
			width2-=7;
		}
		if(s1>0)&&(s2>0)&&(!strcmp(s1,s2))
		{//строки совпадают
			for(i=0;i<width2-2;i++)
				if(i+linex<s1l)str[x2+i]=DSBYTE[s1+i+linex];else str[x2+i]=' ';
		}else
		{
			for(i=0;i<width2-2;i++)
			{
				if(i+linex<s1l)c1=DSBYTE[s1+i+linex]; else c1=' ';
				if(i+linex<s2l)c2=DSBYTE[s2+i+linex]; else c2=' ';
				str[x2+i]=c1;
				if(c1==c2)attr[x2+i]=0x4F;else attr[x2+i]=0x4E;
			}
		}
	}
	printstrat(x,y,width,#str,#attr);
}

drawborder(dword x1,y1,xs1,ys1,fname,line)
char temp[MAX_PATH];
int  titlelen;
{
	int i,j;

	FillMemory(#str,xs1,'═');
	FillMemory(#attr,xs1*2,0x1F);

	str[0]='╚'; str[xs1-1]='╝';
	printstrat(x1,y1+ys1-1,xs1,#str,#attr); //нижняя строчка

	str[0]='╔'; str[xs1-1]='╗';
	titlelen=xs1-4;
	if(!shownums)titlelen-=8;
	if(strlen(fname)>titlelen)wsprintf(#temp," ...%s ",strlen(fname)+fname-titlelen+3);
		else wsprintf(#temp," %s ",fname);
	for(i=0;temp[i];i++){ str[i+1]=temp[i]; attr[i+1]=0x1E; }
	if(!shownums)
	{
		wsprintf(#temp,"[%6d]",line);
		CopyMemory(xs1-9+#str,#temp,8);
		FillMemory(xs1-8*2+#attr,12,0x19);
	}
	printstrat(x1,y1,xs1,#str,#attr);
}

printpanel(dword line,linex ,x1,y1,xs1,ys1 ,x2,y2,xs2,ys2)
{
	int i,j;
	int s,d,sl,dl;
	int line1,line2;

	for(j=line;j<srcfilenums.Count;j++){ s=srcfilenums.At(j); if(s!=-1)BREAK; }
	if(s==-1)s=srcfilelines.Count-1;
	drawborder(x1,y1,xs1,ys1,srcfilename,s+1);

	for(j=line;j<dstfilenums.Count;j++){ d=dstfilenums.At(j); if(d!=-1)BREAK; }
	if(d==-1)d=dstfilelines.Count-1;
	drawborder(x2,y2,xs2,ys2,dstfilename,d+1);
	for(j=0;j<ys1-2;j++)
	{
		if(j+line<srcfilenums.Count)s=srcfilenums.At(j+line); else s=-1;
		line1=s;
		if(s!=-1)s=srcfilelines.At(s);
		if(j+line<dstfilenums.Count)d=dstfilenums.At(j+line); else d=-1;
		line2=d;
		if(d!=-1)d=dstfilelines.At(d);
		drawline(x1,y1+j+1,xs1,linex,s,d,line1);
		drawline(x2,y2+j+1,xs2,linex,d,s,line2);
	}
}


ifinit()
int linenum=0;
int linex=0;
int ys;
int sbi_x, sbi_y;
{
	load_dll(libConsole, #con_init, 0);
	con_init stdcall (122, 40, 122, 800, #window_title);

	linenum=diffs.At(0);

	sbi_x = 122;
	sbi_y = 800;

	sbi_y--;
	ys=sbi_y-2;

	if(linenum+ys>srcfilenums.Count)linenum=srcfilenums.Count-ys;
	if(linenum<0) linenum=0; 

@redraw:
	if(!verthoriz)
	{
		printpanel(linenum,linex
							,0,0
							,sbi_x/2 ,sbi_y
							,sbi_x/2 ,0
							,sbi_x/2 ,sbi_y
		);
		ys=sbi_y-2;
	}
	else
	{
		printpanel(linenum,linex
							,0,0
							,sbi_x ,sbi_y/2
							,0            ,sbi_y/2
							,sbi_x ,sbi_y/2
		);
		if(sbi_y&1)
		{
			FillMemory(#str,sbi_x,'░');
			FillMemory(#attr,sbi_x*2,0);
			printstrat(0,sbi_y-1,sbi_x,#str,#attr);
		}
		ys=sbi_y/2-2;
	}

	if (!ys) goto redraw;
	else {
		con_set_cursor_pos stdcall (2,0);
		ExitProcess();
	}
}
