/*
 * System Monitor
 * version 0.87
 * Author: Leency
*/

#define MEMSIZE 4096*10

#include "../lib/io.h"
#include "../lib/gui.h"
#include "../lib/fs.h"

#include "../lib/obj/libio.h"
#include "../lib/obj/libimg.h"
#include "../lib/obj/libini.h"

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

dword tmp_size[10];

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

	GetTmpDiskSizes();
	
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
				if (cpu_frequency < 1000) sprintf(#param, "CPU frequency: %i Hz", cpu_frequency);
				else sprintf(#param, "CPU frequency: %i MHz", cpu_frequency/1000);
				DrawBlockHeader(LEFT, y.inc(20), 37, "CPU load", #param);
				cpu.set_size(LEFT, y.inc(45), CPU_STACK, 100);

				sprintf(#param, "Total RAM: %i MiB", GetTotalRAM()/1024);
				DrawBlockHeader(LEFT, y.inc(cpu.h + 25), 36, "RAM usage", #param);
				ram.set_size(LEFT, y.inc(45), CPU_STACK, 23);

				DrawBlockHeader(LEFT, y.inc(ram.h + 25), 3, "System RAM Disk usage", "Fixed size: 1.44 MiB");
				rd.set_size(LEFT, y.inc(45), CPU_STACK, 23);

				sprintf(#param, "TMP Disk 0 size: %i MiB", tmp_size[0]);
				DrawBlockHeader(LEFT, y.inc(rd.h + 25), 50, "Virtual drive usage", #param);
				tmp[0].set_size(LEFT, y.inc(45), CPU_STACK, 23);

			default:
				MonitorCpu();

				//MonitorRam();
				ram.draw_progress(
					GetFreeRAM()*ram.w/GetTotalRAM(), 
					GetTotalRAM()-GetFreeRAM()/1024, 
					GetFreeRAM()/1024,
					"M"
					);
				DrawBar(ram.x+ram.w-96, ram.y-25, 96, 20, system.color.work);
				sprintf(#param, "%i KiB", GetTotalRAM()-GetFreeRAM());
				WriteText(ram.x+ram.w-calc(strlen(#param)*8), ram.y-25, 0x90, system.color.work_text, #param);

				//MonitorRd();
				dir_size.get("/rd/1");			
				dir_size.bytes += dir_size.files/2 + 32 * 512; //file attr size + FAT table size
				dir_size.bytes /= 1024; //convert to KiB
				dir_size.bytes = 1440 - dir_size.bytes; 
				rd.draw_progress(
					dir_size.bytes*rd.w/1440,
					1440 - dir_size.bytes,
					dir_size.bytes,
					"K"
					);

				//MonitorTmp();
				if (tmp_size[0]) {
					dir_size.get("/tmp0/1");
					dir_size.bytes += dir_size.files/2 + 32 * 512; //file attr size + FAT table size
					dir_size.bytes /= 1024*1024; //convert to MiB
					dir_size.bytes= tmp_size[0] - dir_size.bytes;
					tmp[0].draw_progress(
						dir_size.bytes*tmp[0].w/tmp_size[0],
						tmp_size[0] - dir_size.bytes,
						dir_size.bytes,
						"M"
						);					
				}
		}
	}
}

void DrawBlockHeader(dword _x, _y, _icon, _title, _subtitle)
{
	WriteTextB(_x+ICONGAP, _y, 0x90, system.color.work_text, _title);
	DrawIcon32(_x, _y, system.color.work, _icon);
	WriteText(_x+ICONGAP, _y+20, 0x90, system.color.work_text, _subtitle);	
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

dword GetDiskSize(dword disk_n)
{
	BDVK bdvk;
	char tmp_path[8];
	strcpy(#tmp_path, "/tmp0/1");
	tmp_path[4] = disk_n + '0';
	GetFileInfo(#tmp_path, #bdvk);		
	return bdvk.sizelo;
}
void GetTmpDiskSizes()
{
	char i;
	for (i=0; i<=9; i++)
	{
		tmp_size[i] = GetDiskSize(i) / 1024 / 1024;
	}
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
		DrawBar(i+cpu.x, cpu.y, 1, cpu.h-cpu_stack[i], PROGRESS_BG);
		DrawBar(i+cpu.x, cpu.h-cpu_stack[i]+cpu.y, 1, cpu_stack[i], LOAD_CPU);

		DrawBar(i+1+cpu.x, cpu.y, 1, cpu.h, PROGRESS_BG);
	}

	pos++;
	if (pos>=CPU_STACK) {
		pos = CPU_STACK-1;
		for (i=0; i<pos; i++) {
			cpu_stack[i] = cpu_stack[i+1];
		}
	}
}
