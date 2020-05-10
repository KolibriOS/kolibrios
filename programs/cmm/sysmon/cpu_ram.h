//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

sensor cpu;
sensor ram;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void DrawIconWithText(dword _x, _y, _icon, _title)
{
	DrawIcon16(_x, _y, sc.work, _icon);
	DrawBar(_x+ICONGAP, _y, WIN_CONTENT_W - ICONGAP - _x, 20, sc.work);
	WriteText(_x+ICONGAP, _y, 0x90, sc.work_text, _title);
}

void CPUnRAM__Main()
{
	dword cpu_frequency = GetCpuFrequency()/1000;
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON);
	goto _GENERAL_REDRAW_2;
	loop()
	{
		WaitEventTimeout(25);
		switch(EAX & 0xFF)
		{
			case evButton: Sysmon__ButtonEvent(); break;
			case evKey: Sysmon__KeyEvent(); break;
			case evReDraw: 
				_GENERAL_REDRAW_2: 
				Sysmon__DefineAndDrawWindow(); 
				cpu.set_size(WIN_PAD, WIN_CONTENT_Y+25, WIN_CONTENT_W, 100);
				ram.set_size(WIN_PAD, WIN_CONTENT_Y+170, WIN_CONTENT_W, 23);
			default:
				MonitorCpu();
				MonitorRam();
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

int pos=0;
void MonitorCpu()
{
	static dword cpu_stack[1980*3];
	int i;
	if (!cpu.w) return;

	cpu_stack[pos] = GetCpuLoad(cpu.h);
	if (cpu_stack[pos]<=2) || (cpu_stack[pos]>cpu.h) cpu_stack[pos]=2;
	
	sprintf(#param, "CPU load %i%%", cpu_stack[pos]);
	DrawIconWithText(WIN_PAD, cpu.y - 25, 48, #param);
	
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

void MonitorRam()
{
	ram.draw_progress(GetFreeRAM()*ram.w/GetTotalRAM());
	sprintf(#param, "RAM usage: %i Mb free of %i Mb", GetFreeRAM()/1024, GetTotalRAM()/1024);
	DrawIconWithText(WIN_PAD, ram.y - 25, 51, #param);
}