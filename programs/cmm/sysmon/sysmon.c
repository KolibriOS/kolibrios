/*
 * System Monitor
 * version 0.7
 * Author: Leency
*/

#define MEMSIZE 4096*10

#include "../lib/io.h"
#include "../lib/gui.h"

#include "../lib/obj/libio.h"
#include "../lib/obj/libimg.h"
#include "../lib/obj/libini.h"

//===================================================//
//                                                   //
//                      SENSOR                       //
//                                                   //
//===================================================//

#define MIN_PB_BLOCK_W 19
#define LOAD_BG 0xFFFfff
#define LOAD_ACTIVE 0x4C52FF

struct sensor {
	int x,y,w,h;
	void set_size();
	void draw_wrapper();
	void draw_progress();
};

void sensor::set_size(dword _x, _y, _w, _h)
{
	x=_x+2; 
	y=_y;
	w=_w;
	h=_h;
}

void sensor::draw_wrapper()
{
	DrawRectangle(x-1, y-1, w+1, h+1, system.color.work_graph);
	DrawRectangle3D(x-2, y-2, w+3, h+3, system.color.work_dark, system.color.work_light);
}

void sensor::draw_progress(dword progress_w, active_value, bg_value, mesure)
{
	if (progress_w < MIN_PB_BLOCK_W) progress_w = MIN_PB_BLOCK_W;
	if (progress_w > w-MIN_PB_BLOCK_W) progress_w = w-MIN_PB_BLOCK_W;

	DrawBar(x, y, w-progress_w, h, LOAD_ACTIVE);
	sprintf(#param, "%i%s", active_value, mesure);
	WriteText(w-progress_w- calc(strlen(#param)*8) /2 + x, h/2-7+y, 0x90, LOAD_BG, #param);

	DrawBar(x+w-progress_w, y, progress_w, h, LOAD_BG);
	sprintf(#param, "%i%s", bg_value, mesure);
	WriteText(-progress_w - calc(strlen(#param)*8)/2 + w+x, h/2-7+y, 0x90, LOAD_ACTIVE, #param);
}

//===================================================//
//                                                   //
//                    GetSizeDir                     //
//                                                   //
//===================================================//

BDVK file_info_dirsize;
dword dir_count;
dword file_count;
dword size_dir;

void GetDirSizeAndCountFiles(dword way)
{
	dir_count=0;
	file_count=0;
	size_dir=0;
	GetDirSizeAndCountFiles_loop(way);
}

void GetDirSizeAndCountFiles_loop(dword way)
{
	dword dirbuf, fcount, i, filename;
	dword cur_file;
	if (dir_exists(way))
	{
		cur_file = malloc(4096);
		// In the process of recursive descent, memory must be allocated dynamically, 
		// because the static memory -> was a bug !!! But unfortunately pass away to sacrifice speed.
		GetDir(#dirbuf, #fcount, way, DIRS_ONLYREAL);
		for (i=0; i<fcount; i++)
		{
			filename = i*304+dirbuf+72;
			sprintf(cur_file,"%s/%s",way,filename);
			
			if (TestBit(ESDWORD[filename-40], 4) )
			{
				dir_count++;
				GetDirSizeAndCountFiles_loop(cur_file);
			}
			else
			{
				GetFileInfo(cur_file, #file_info_dirsize);
				size_dir += file_info_dirsize.sizelo;
				file_count++;
			}
		}
		free(cur_file);
		free(dirbuf);
	}
}

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#define CPU_STACK 440
dword cpu_stack[CPU_STACK];

sensor cpu;
sensor ram;
sensor rd;
sensor tmp[10];

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{
	proc_info Form;
	dword cpu_frequency = GetCpuFrequency()/1000;
	int id;

	incn y;

	load_dll(libio, #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libini, #lib_init,1);
	
	loop()
	{
		WaitEventTimeout(25);
		switch(EAX & 0xFF)
		{
			case evButton:
				if (GetButtonID()) ExitProcess();
				break;
		  
			case evKey:
				GetKeys();
				if (key_scancode == SCAN_CODE_ESC) ExitProcess();
				break;
			 
			case evReDraw:
				#define LEFT 25
				#define ICONGAP 45
				system.color.get();
				DefineAndDrawWindow(150, 100, CPU_STACK+LEFT+LEFT+4+9, 480 + skin_height + 4, 0x34, system.color.work, "System Monitor",0);
				GetProcessInfo(#Form, SelfInfo);

				y.n = 0;
				WriteTextB(LEFT+ICONGAP, y.inc(20), 0x90, system.color.work_text, "CPU load");
				DrawIcon32(LEFT, y.n, system.color.work, 37);

				if (cpu_frequency < 1000) sprintf(#param, "CPU frequency: %i Hz", cpu_frequency);
				else sprintf(#param, "CPU frequency: %i MHz", cpu_frequency/1000);
				WriteText(LEFT+ICONGAP, y.inc(20), 0x90, system.color.work_text, #param);
				cpu.set_size(LEFT, y.inc(25), CPU_STACK, 100);
				cpu.draw_wrapper();

				WriteTextB(LEFT+ICONGAP, y.inc(cpu.h + 25), 0x90, system.color.work_text, "RAM usage");
				DrawIcon32(LEFT, y.n, system.color.work, 36);
				sprintf(#param, "Total RAM: %i MiB", GetTotalRAM()/1024);
				WriteText(LEFT+ICONGAP, y.inc(20), 0x90, system.color.work_text, #param);
				ram.set_size(LEFT, y.inc(25), CPU_STACK, 25);
				ram.draw_wrapper();

				WriteTextB(LEFT+ICONGAP, y.inc(ram.h + 25), 0x90, system.color.work_text, "System RAM Disk usage");
				DrawIcon32(LEFT, y.n, system.color.work, 3);
				WriteText(LEFT+ICONGAP, y.inc(20), 0x90, system.color.work_text, "Fixed size: 1.44 MiB");
				rd.set_size(LEFT, y.inc(25), CPU_STACK, 25);
				rd.draw_wrapper();

				WriteTextB(LEFT+ICONGAP, y.inc(ram.h + 25), 0x90, system.color.work_text, "Virtual drive usage");
				DrawIcon32(LEFT, y.n, system.color.work, 50);
				WriteText(LEFT+ICONGAP, y.inc(20), 0x90, system.color.work_text, "TMP Disk 0 size: 49 MiB");
				tmp[0].set_size(LEFT, y.inc(25), CPU_STACK, 25);
				tmp[0].draw_wrapper();

			default:
				MonitorCpu();

				ram.draw_progress(
					GetFreeRAM()*ram.w/GetTotalRAM(), 
					GetTotalRAM()-GetFreeRAM()/1024, 
					GetFreeRAM()/1024,
					"M"
					);

				GetDirSizeAndCountFiles("/rd/1");
				size_dir += 32*512; //add FAT table size
				size_dir += file_count*512/2; //add MAGIC NUMBER
				size_dir /= 1024; //convert to KiB
				size_dir= 1440 - size_dir;
				rd.draw_progress(
					size_dir*rd.w/1440,
					1440-size_dir,
					size_dir,
					"K"
					);

				GetDirSizeAndCountFiles("/tmp0/1");
				size_dir += 32*512; //add FAT table size
				size_dir += file_count*512/2; //add MAGIC NUMBER
				size_dir /= 1024*1024; //convert to MiB
				size_dir= 49 - size_dir;
				tmp[0].draw_progress(
					size_dir*tmp[0].w/49,
					49-size_dir,
					size_dir,
					"M"
					);
		}
	}
}

dword GetCpuLoad(dword max_h)
{
	dword idle;
	dword CPU_SEC = GetCpuFrequency() >> 20 + 1;
	dword IDLE_SEC = GetCpuIdleCount() >> 20 * max_h;

	EAX = IDLE_SEC;
	EBX = CPU_SEC;
	$cdq
	$div ebx
	idle = EAX;

	return max_h - idle;
}

//===================================================//
//                                                   //
//                     MONITORS                      //
//                                                   //
//===================================================//

int pos=0;
void MonitorCpu()
{
	int i;
	if (!cpu.w) return;

	cpu_stack[pos] = GetCpuLoad(cpu.h);
	if (cpu_stack[pos]<=2) || (cpu_stack[pos]>cpu.h) cpu_stack[pos]=2;
	
	DrawBar(cpu.x+cpu.w-30, cpu.y-25, 30, 20, system.color.work);
	sprintf(#param, "%i%%", cpu_stack[pos]);
	WriteText(cpu.x+cpu.w-calc(strlen(#param)*8), cpu.y-25, 0x90, system.color.work_text, #param);

	for (i=0; i<CPU_STACK; i+=2) {
		DrawBar(i+cpu.x, cpu.y, 1, cpu.h-cpu_stack[i], LOAD_BG);
		DrawBar(i+cpu.x, cpu.h-cpu_stack[i]+cpu.y, 1, cpu_stack[i], LOAD_ACTIVE);

		DrawBar(i+1+cpu.x, cpu.y, 1, cpu.h, LOAD_BG);
	}

	pos++;
	if (pos>=CPU_STACK) {
		pos = CPU_STACK-1;
		for (i=0; i<pos; i++) {
			cpu_stack[i] = cpu_stack[i+1];
		}
	}
}
