:struct child_window
{
	dword window_loop_pointer;
	dword id;
	char stak[4096];
	void create();
	bool thread_exists();
};

:void child_window::create()
{
	id = CreateThread(window_loop_pointer, #stak+4092);
}

:bool child_window::thread_exists()
{
	dword proc_slot = GetProcessSlot(id);
	if (proc_slot) {
		ActivateWindow(proc_slot);
		return true;
	}
	return false;
}