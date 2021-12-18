#define MEMSIZE 1024*20
#define ENTRY_POINT #main

#include "../lib/fs.h"
#include "../lib/mem.h"
#include "../lib/obj/libimg.h"
#include "../lib/patterns/rgb.h"

void main()
{
	libimg_image icons32;
	libimg_image icons16;
	libimg_image icons16w;
	unsigned int size32;
	unsigned int size16;
	char* shared_i32;
	char* shared_i16;
	char* shared_i16w;

	mem_init();
	load_dll(libimg, #libimg_init, 1);
	@SetEventMask(EVM_DESKTOPBG);

	icons32.load("/sys/icons32.png"); size32 = icons32.h * 32 * 4;
	icons16.load("/sys/icons16.png"); size16 = icons16.h * 18 * 4;

	shared_i32 = memopen("ICONS32", size32, SHM_CREATE+SHM_WRITE);
	memmov(shared_i32, icons32.imgsrc, size32);
	img_destroy stdcall(icons32.image);

	shared_i16 = memopen("ICONS18", size16, SHM_CREATE + SHM_WRITE);
	memmov(shared_i16, icons16.imgsrc, size16);
	img_destroy stdcall(icons16.image);

	shared_i16w = memopen("ICONS18W", size16, SHM_CREATE + SHM_WRITE);

UPDATE_ICONS18WORK:
	$push sc.work
	sc.get();
	$pop eax
	if (sc.work != EAX) {
		icons16w.load("/sys/icons16.png");
		icons16w.replace_2colors(0xffFFFfff, sc.work, 0xffCACBD6, MixColors(sc.work, 0, 200));
		memmov(shared_i16w, icons16w.imgsrc, size16);
		img_destroy stdcall(icons16w.image);
		icons16w.image = NULL;
	}

	loop() IF(WaitEvent()==evDesktop) GOTO UPDATE_ICONS18WORK;
}
