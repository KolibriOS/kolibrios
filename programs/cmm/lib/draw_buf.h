
dword buf_data;

struct DrawBufer {
	int bufx, bufy, bufw, bufh;

	void Init();
	void Show();
	void Fill();
	void Skew();
	void DrawBar();
	void PutPixel();
	void AlignCenter();
	void AlignRight();
};

void DrawBufer::Init(int i_bufx, i_bufy, i_bufw, i_bufh)
{
	bufx = i_bufx;
	bufy = i_bufy;
	bufw = i_bufw; 
	bufh = i_bufh;
	free(buf_data);
	buf_data = malloc(bufw * bufh * 4 + 8); //+1 for good luck
	ESDWORD[buf_data] = bufw;
	ESDWORD[buf_data+4] = bufh;
}

void DrawBufer::Show()
{
	PutPaletteImage(buf_data+8, bufw, bufh, bufx, bufy, 32,0);
}

void DrawBufer::Fill(dword fill_color)
{
	int i;
	int max_i = bufw * bufh * 4 + buf_data + 8;
	for (i=buf_data+8; i<max_i; i+=4) ESDWORD[i] = fill_color;
}

void DrawBufer::DrawBar(dword x, y, w, h, color)
{
	int i, j;
	for (j=0; j<h; j++)
	{
		for (i = y+j*bufw+x*4+8+buf_data; i<y+j*bufw+x+w*4+8+buf_data; i+=4) ESDWORD[i] = color;
	}
}

void DrawBufer::PutPixel(dword x, y, color)
{
	int pos = y*bufw+x*4+8+buf_data;
	ESDWORD[pos] = color;
}

char shift[]={8,8,4,4};
void DrawBufer::Skew(dword x, y, w, h)
{
	int i, j;
	for (j=0; j<=3; j++)
	{
		for (i = y+j*bufw+x+w+h*4; i>y+j*bufw+x+h-12*4 ; i-=4)
								ESDWORD[buf_data+i+8] = ESDWORD[-shift[j]+buf_data+i+8];
	}
}

void DrawBufer::AlignRight(dword x,y,w,h, content_width)
{
	int i, j, l;
	int content_left = w - content_width / 2;
	for (j=0; j<h; j++)
	{
		for (i=j*w+w-x*4, l=j*w+content_width+x*4; (i>=j*w+content_left*4) && (l>=j*w*4); i-=4, l-=4)
		{
			ESDWORD[buf_data+8+i] >< ESDWORD[buf_data+8+l];
		}
	}
}

void DrawBufer::AlignCenter(dword x,y,w,h, content_width)
{
	int i, j, l;
	int content_left = w - content_width / 2;
	for (j=0; j<h; j++)
	{
		for (i=j*w+content_width+content_left*4, l=j*w+content_width+x*4; (i>=j*w+content_left*4) && (l>=j*w*4); i-=4, l-=4)
		{
			ESDWORD[buf_data+8+i] >< ESDWORD[buf_data+8+l];
		}
	}
}



