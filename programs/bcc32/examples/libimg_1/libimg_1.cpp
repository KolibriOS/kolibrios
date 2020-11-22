#include <kolibri.h>
#include <kos_heap.h>
#include <kos_file.h>
#include <load_lib.h>
#include <l_libimg.h>

using namespace Kolibri;

const char header[] = "Image";
unsigned char* img_d = 0;
long img_w, img_h;
char library_path[2048];

namespace Kolibri{
	char CurrentDirectoryPath[2048];
}

bool KolibriOnStart(TStartData &kos_start, TThreadData /*th*/)
{
	kos_start.Left = 10;
	kos_start.Top = 40;
	kos_start.Width = 280;
	kos_start.Height = 200;
	kos_start.WinData.WindowColor = 0xFFFFFF;
	kos_start.WinData.WindowType = 0x33; // 0x34 - fixed, 0x33 - not fixed
	kos_start.WinData.Title = header;
	LoadLibrary("libimg.obj", library_path, "/sys/lib/libimg.obj", &import_libimg);
	return true;
}

void KolibriOnPaint(void)
{
	// If button have ID 1, this is close button
	DrawButton(2,0xf0f0f0, 10,10,50,20);
	DrawText(20,16,0,"Open");
	if(img_d) PutImage(img_d,10,40,img_w,img_h);
}

void KolibriOnButton(long id, TThreadData /*th*/)
{
	FileInfoBlock* file;
	long int k;

	switch(id){
	case 2:
		file = FileOpen("1.png");
		if (!file){
			SetWindowCaption("Error open file '1.png'");
			break;
		}
		k = FileGetLength(file);
		if (k > 0){
			if(img_d) Free(img_d);
			img_d = (unsigned char*)Alloc(k);
			if (img_d){
				if (FileRead(file, img_d, k) != k){
					Free(img_d); img_d = 0;
				}
				else{
					Image* img;
					img = img_decode(img_d,k,0);
					img_w = img->Width;
					img_h = img->Height;
					img_d = (unsigned char*)ReAlloc(img_d, 3*img_w*img_h);
					//if (!img_d){ ... }
					img_to_rgb2(img,img_d);
					img_destroy(img);
					SetWindowCaption("1.png");
					Redraw(1);
				}
			}
		}
		FileClose(file);
		//break;
	};
}

bool KolibriOnClose(TThreadData /*th*/) {
	if(img_d) {Free(img_d); img_d = 0;}
	return true;
}