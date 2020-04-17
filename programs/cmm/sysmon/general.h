//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

dword cpu_stack[1980*3];

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

void General__Main()
{
	dword cpu_frequency;
	incn y;
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON);

	cpu_frequency = GetCpuFrequency()/1000;
	GetTmpDiskSizes();

	goto _GENERAL_REDRAW;
	
	loop()
	{
		WaitEventTimeout(25);
		switch(EAX & 0xFF)
		{
			case evButton:
				Sysmon__ButtonEvent(GetButtonID());
				break;
		  
			case evKey:
				GetKeys();
				if (key_scancode == SCAN_CODE_ESC) ExitProcess();
				break;
			 
			case evReDraw:
				_GENERAL_REDRAW:
				if (!Sysmon__DefineAndDrawWindow()) break;

				y.n = WIN_CONTENT_Y;
				if (cpu_frequency < 1000) sprintf(#param, "CPU frequency: %i Hz", cpu_frequency);
				else sprintf(#param, "CPU frequency: %i MHz", cpu_frequency/1000);
				DrawBlockHeader(WIN_PAD, y.inc(0), 37, "CPU load", #param);
				cpu.set_size(WIN_PAD, y.inc(45), WIN_CONTENT_W, 100);

				sprintf(#param, "Total RAM: %i MiB", GetTotalRAM()/1024);
				DrawBlockHeader(WIN_PAD, y.inc(cpu.h + 25), 36, "RAM usage", #param);
				ram.set_size(WIN_PAD, y.inc(45), WIN_CONTENT_W, 23);

				DrawBlockHeader(WIN_PAD, y.inc(ram.h + 25), 3, "System RAM Disk usage", "Fixed size: 1.44 MiB");
				rd.set_size(WIN_PAD, y.inc(45), WIN_CONTENT_W, 23);

				sprintf(#param, "TMP Disk 0 size: %i MiB", tmp_size[0]);
				DrawBlockHeader(WIN_PAD, y.inc(rd.h + 25), 50, "Virtual drive usage", #param);
				tmp[0].set_size(WIN_PAD, y.inc(45), WIN_CONTENT_W, 23);

			default:
				MonitorCpu();

				//MonitorRam();
				ram.draw_progress(
					GetFreeRAM()*ram.w/GetTotalRAM(), 
					GetTotalRAM()-GetFreeRAM()/1024, 
					GetFreeRAM()/1024,
					"M"
					);
				DrawBar(ram.x+ram.w-96, ram.y-25, 96, 20, sc.work);
				sprintf(#param, "%i KiB", GetTotalRAM()-GetFreeRAM());
				WriteText(ram.x+ram.w-calc(strlen(#param)*8), ram.y-25, 0x90, sc.work_text, #param);

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
	#define ICONGAP 45
	WriteTextB(_x+ICONGAP, _y, 0x90, sc.work_text, _title);
	DrawIcon32(_x, _y, sc.work, _icon);
	WriteText(_x+ICONGAP, _y+20, 0x90, sc.work_text, _subtitle);	
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
	
	DrawBar(cpu.x+cpu.w-30, cpu.y-25, 30, 20, sc.work);
	sprintf(#param, "%i%%", cpu_stack[pos]);
	WriteText(cpu.x+cpu.w-calc(strlen(#param)*8), cpu.y-25, 0x90, sc.work_text, #param);

	for (i=0; i<WIN_CONTENT_W; i+=2) {
		DrawBar(i+cpu.x, cpu.y, 1, cpu.h-cpu_stack[i], PROGRESS_BG);
		DrawBar(i+cpu.x, cpu.h-cpu_stack[i]+cpu.y, 1, cpu_stack[i], LOAD_CPU);

		DrawBar(i+1+cpu.x, cpu.y, 1, cpu.h, PROGRESS_BG);
	}

	pos++;
	if (pos>=WIN_CONTENT_W) {
		pos = WIN_CONTENT_W-1;
		for (i=0; i<pos; i++) {
			cpu_stack[i] = cpu_stack[i+1];
		}
	}
}
