#include <kolibri.h>
#include <kos_heap.h>
#include <kos_file.h>

using namespace Kolibri;

const char header[] = "File open";
unsigned char* f_data = 0;

bool KolibriOnStart(TStartData &kos_start, TThreadData /*th*/)
{
	kos_start.Left = 10;
	kos_start.Top = 40;
	kos_start.Width = 300;
	kos_start.Height = 220;
	kos_start.WinData.WindowColor = 0xFFFFFF;
	kos_start.WinData.WindowType = 0x34; // 0x34 - fixed, 0x33 - not fixed
	kos_start.WinData.Title = header;

	return true;
}

void KolibriOnPaint(void)
{
	// If button have ID 1, this is close button
	DrawButton(2,0xf0f0f0, 10,10,50,20);
	DrawText(20,16,0,"1.txt");
	DrawButton(3,0xf0f0f0, 70,10,50,20);
	DrawText(80,16,0,"2.txt");

	if (f_data){
		long i=0, j, y=0;
		char buf[300];
		do{
			j=0;
			while(f_data[i] && f_data[i]!=13 && f_data[i]!=10){
				buf[j]=f_data[i];
				i++;
				j++;
			};
			buf[j]=0;
			DrawText(10,45+9*y,0,buf);
			y++;
			while(f_data[i]==13 || f_data[i]==10) i++;
		}while(f_data[i]);
	}
}

void KolibriOnButton(long id, TThreadData /*th*/)
{
	FileInfoBlock* file;
	unsigned long int k;

	switch(id){
	case 2:
		file = FileOpen("1.txt");
		if (!file){
			SetWindowCaption("Error open file '1.txt'");
			break;
		}
		k = FileGetLength(file);
		if (k > 0){
			if(f_data) Free(f_data);
			f_data = (unsigned char*)Alloc(k);
			if (f_data){
				if (FileRead(file, f_data, k) != k){
					Free(f_data); f_data = 0;
				}
				else{
					SetWindowCaption("1.txt");
					Redraw(1);
				}
			}
		}
		FileClose(file);
		break;
	case 3:
		file = FileOpen("2.txt");
		if (!file){
			SetWindowCaption("Error open file '2.txt'");
			break;
		}
		k = FileGetLength(file);
		if (k > 0){
			if(f_data) Free(f_data);
			f_data = (unsigned char*)Alloc(k);
			if (f_data){
				if (FileRead(file, f_data, k) != k){
					Free(f_data); f_data = 0;
				}
				else{
					SetWindowCaption("2.txt");
					Redraw(1);
				}
			}
		}
		FileClose(file);
		//break;
	};
}

bool KolibriOnClose(TThreadData /*th*/) {
	if(f_data) {Free(f_data); f_data = 0;}
	return true;
}