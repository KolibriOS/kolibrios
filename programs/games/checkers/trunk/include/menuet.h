#ifndef __MENUET_H_INCLUDED_
#define __MENUET_H_INCLUDED_

#include <me_lib.h>

// Menuet interface.

namespace Menuet   // All menuet functions, types and data are nested in the (Menuet) namespace.
{
	const char *DebugPrefix = "User program: ";

	struct TWindowData   // Data for drawing a window.
	{
		unsigned short WindowType, HeaderType;
		unsigned long WindowColor, HeaderColor, BorderColor, TitleColor;
		const char *Title;
	};

	struct TStartData   // This structure is used only for MenuetOnStart function.
	{
		unsigned short Left, Width, Top, Height; // Initial window rectangle.
		TWindowData WinData;
	};

	typedef void **TThreadData;   // Thread data are the fast identifier of thread, contains user dword in
			//_ the zero element and stack beginning (or zero if it is unknown) in the first element.
			//_ The stack will be deleted from dynamic memory at the finish of the thread if stack beginning is not zero.

	struct TMutex;   // Simple mutex can be locked only once at a time.
#define MENUET_MUTEX_INIT {}   // Simple mutex initializer, cat be redefined in a realization of the library

	struct TRecMutex;   // Recursive mutex can be locked many times by a single thread at a time.
#define MENUET_REC_MUTEX_INIT {}   // Recursive mutex initializer, cat be redefined in a realization of the library

	// Some functions have two forms: the fast form with (thread_data) parameter and the form without it.
	// Note: pass only thread data of current thread as (thread_data) parameter to these functions.

	void Main();   // Main function is called at program startup.
	void* ThreadMain(void *user = 0, void *stack_begin = 0);
			// Called at thread startup, (user) is placed in thread data as a user dword,
			//_ (stack_begin) is placed in thread data as a stack beginning.
			//_ Return new value of stack beginning that can be changed in the thread data.
	void GetWindowData(TWindowData &win_data);   // Write current window data to (win_data).
	void GetWindowData(TWindowData &win_data, TThreadData thread_data);
	void SetWindowData(const TWindowData &win_data);   // Replace current window data by (win_data).
	void SetWindowData(const TWindowData &win_data, TThreadData thread_data);
	void CloseWindow();   // Close current window when returning to message loop.
	void CloseWindow(TThreadData thread_data);
	void Redraw(int frame = 0);   // Redraw current window immediately, if (frame) is positive redraw the frame too,
	void Redraw(int frame, TThreadData thread_data);   //_ if (frame) is negative redraw only invalideted window.
	void Invalidate(int frame = 0);   // Redraw current window when no message will be is the queue,
	void Invalidate(int frame, TThreadData thread_data);   //_ if (frame) is positive redraw the frame too,
														   //_ if (frame) is negative do nothing.
	void MoveWindow(const int window_rect[/* 4 */]);   // Move and resize current window.

	void Abort();   // Abnormally terminate a program.
	void ExitProcess();   // Exit from the process, don't call any destructors of global varyables
	void ExitThread();   // Exit from the current thread
	void ExitThread(TThreadData thread_data);
	void ReturnMessageLoop();   // Return to the message loop of the thread. Exit from the thread
	void ReturnMessageLoop(TThreadData thread_data);   //_ if it is called from (MenuetOnStart).

	void Delay(unsigned int time);   // Delay the execution of the program during (time) hundredth seconds.
	unsigned int Clock();   // Return the time from starting of the system to this moment in hundredth of seconds.
	int GetPackedTime();   // Return the current time of day in binary-decimal format 0x00SSMMHH.
	void GetTime(int t[/* 3 */]);   // Write the current time to (t): t[0] = second, t[1] = minute, t[2] = hour.
	int GetPackedDate();   // Return the current date in binary-decimal format 0x00YYDDMM.
	void GetDate(int d[/* 3 */]);   // Write the current date to (d): d[0] = day, d[1] = month, d[2] = year.
	void GetTimeDate(int t[/* 6 */]);   // Write the current time and date to (t): t[0] = second,
										//_ t[1] = minute, t[2] = hour, t[3] = day, t[4] = month, t[5] = year.
	void ReadCommonColors(unsigned int colors[/* 10 */]);   // Writes standart window colors to (colors).
	unsigned int GetProcessInfo(unsigned int *use_cpu, char process_name[/* 13 */], unsigned int *use_memory,
								unsigned int *pid, int window_rect[/* 4 */], unsigned int pid_for = -1);
								// Write (pid_for) process information to variables parameters points, return
								//_ the number of processes. (pid_for) equal to (-1) means current process.
	unsigned int GetPid();   // Return the current thread identifier (pid).
	unsigned int GetPid(TThreadData thread_data);
	TThreadData GetThreadData();   // Return the thread data of the current thread.
	TThreadData GetThreadData(unsigned int pid);   // Return the thread data of the thread with the given pid.

	void* GetPicture(unsigned short &width, unsigned short &height);
	void* GetPicture(unsigned short &width, unsigned short &height, TThreadData thread_data);
			// Return the picture on the window and write its width and height to (width) and (height).
	void SetPicture(void *picture, unsigned short width, unsigned short height);
	void SetPicture(void *picture, unsigned short width, unsigned short height, TThreadData thread_data);
			// Replace the picture on the window by the given picture with the given width and height.
	void GetBorderHeader(unsigned short &border_size, unsigned short &header_size);
	void GetBorderHeader(unsigned short &border_size, unsigned short &header_size, TThreadData thread_data);
			// Write the border thickness to (border_size) and header height to (header_size).
	void GetClientSize(unsigned short &width, unsigned short &height);
	void GetClientSize(unsigned short &width, unsigned short &height, TThreadData thread_data);
			// Write the client area width and height to (width) and (height) parameters.
	void GetClientSize(unsigned short &width, unsigned short &height, int win_width, int win_height);
	void GetClientSize(unsigned short &width, unsigned short &height, int win_width, int win_height, TThreadData thread_data);
			// Write the client area size of window with the width (win_width)
			//_ and height (win_height) to (width) and (height) parameters.
	void GetScreenSize(unsigned short &width, unsigned short &height);
			// Write the screen width and height to (width) and (height) parameters.

	void InitMutex(TMutex *mutex);   // Initialize the simple mutex.
	void InitRecMutex(TRecMutex *mutex);   // Initialize the recursive mutex.
	bool TryLock(TMutex *mutex);   // Try to lock the mutex without waitting, return true if lock.
	bool TryLock(TRecMutex *mutex);
	bool TryLock(TRecMutex *mutex, TThreadData thread_data);
	bool TryLock(TRecMutex *mutex, unsigned int pid);
	void Lock(TMutex *mutex);   // Lock mutex and wait for it if this necessary.
	void Lock(TRecMutex *mutex);
	void Lock(TRecMutex *mutex, TThreadData thread_data);
	void Lock(TRecMutex *mutex, unsigned int pid);
	bool LockTime(TMutex *mutex, int time);
	bool LockTime(TRecMutex *mutex, int time);   // Lock mutex and wait for it during (time) hundredth seconds.
	bool LockTime(TRecMutex *mutex, int time, TThreadData thread_data);
	bool LockTime(TRecMutex *mutex, int time, unsigned int pid);
	void UnLock(TMutex *mutex);   // Unlock mutex
	void UnLock(TRecMutex *mutex);
	void UnLock(TRecMutex *mutex, TThreadData thread_data);
	void UnLock(TRecMutex *mutex, unsigned int pid);

	void DebugPutChar(char c);   // Put the character to the debug board.
	void DebugPutString(const char *s);   // Put the string to the debug board.
	int GetKey();   // Return key pressed by user or -1 if no key was pressed.
	int GetMouseButton();   // Return buttons pressed: 0 - no buttons, 1 - left button, 2 - right button, 3 - both buttons.
	void GetMousePosition(short &x, short &y, bool absolute = false);
			// Write mouse position to (x) and (y): absolute if (absolute) is true and relative the window otherwise.
	void GetMousePosPicture(short &x, short &y);

	int GetThreadNumber();   // Return the number of threads currently executing.
	bool WasThreadCreated();   // Return true if there was created at least one thread except the main thred.
	unsigned int CreateThread(void *user = 0, unsigned int stack_size = 0, void *stack_end = 0);
			// Create a new thread with the user dword (user) and stack pointer (stack_end).
			//_ If (stack_end) is zero, create stack in dynamic memory of size (stack_size) or
			//_ the same size as the main thread if (stack_size) less that 4096. Set the beginning
			//_ of the stack if (stack_end) is zero or (stack_size) is not zero, in this case stack
			//_ will be deleted automaticaly from dynamic memory at the finish of the thread.
}

// Function, defined outside.

bool MenuetOnStart(Menuet::TStartData &me_start, Menuet::TThreadData thread_data);
			// Window will be created iff return value is true.
bool MenuetOnClose(Menuet::TThreadData thread_data);     // Window will be closed iff return value is true.
int MenuetOnIdle(Menuet::TThreadData thread_data);       // Return the time to wait next message.
void MenuetOnSize(int window_rect[/* 4 */], Menuet::TThreadData thread_data);  // When the window is resized.
void MenuetOnKeyPress(Menuet::TThreadData thread_data);  // When user press a key.
void MenuetOnMouse(Menuet::TThreadData thread_data);     // When user move a mouse.

#ifdef __MENUET__

namespace Menuet
{
// Structures.

	struct TMutex   // Simple mutex can be locked only once at a time.
	{
		unsigned int mut;
	};
#undef  MENUET_MUTEX_INIT
#define MENUET_MUTEX_INIT {0x40}   // Simple mutex initializer, cat be redefined in a realization of the library

	struct TRecMutex   // Recursive mutex can be locked many times by a single thread at a time.
	{
		unsigned int mut, pid;
	};
#undef  MENUET_REC_MUTEX_INIT
#define MENUET_REC_MUTEX_INIT {0x20,-1}   // Recursive mutex initializer, cat be redefined in a realization of the library

// Global variables.

	volatile TThreadData _ThreadTable[256];
	volatile unsigned int _ThreadScanCount[2] = {0, 0};
	volatile int _ThreadNumber = 1;
	volatile int _ExitProcessNow = 0;
	TMutex _ThreadMutex = MENUET_MUTEX_INIT;
	unsigned int _ThreadSavedBegProc[4];

// Inline functions.

	inline void GetWindowData(TWindowData &win_data) {GetWindowData(win_data, GetThreadData());}

	inline void SetWindowData(const TWindowData &win_data) {SetWindowData(win_data, GetThreadData());}

	inline void CloseWindow() {CloseWindow(GetThreadData());}

	inline void Redraw(int frame) {Redraw(frame, GetThreadData());}

	inline void Invalidate(int frame) {Invalidate(frame, GetThreadData());}

	inline void* GetPicture(unsigned short &width, unsigned short &height)
	{
		return GetPicture(width, height, GetThreadData());
	}

	inline void SetPicture(void *picture, unsigned short width, unsigned short height)
	{
		SetPicture(picture, width, height, GetThreadData());
	}

	inline void GetBorderHeader(unsigned short &border_size, unsigned short &header_size)
	{
		GetBorderHeader(border_size, header_size, GetThreadData());
	}

	inline void GetClientSize(unsigned short &width, unsigned short &height)
	{
		unsigned int pid;
		int rect[4];
		GetProcessInfo(0, 0, 0, &pid, rect);
		GetClientSize(width, height, rect[2], rect[3], GetThreadData(pid));
	}

	inline void GetClientSize(unsigned short &width, unsigned short &height, TThreadData thread_data)
	{
		int rect[4];
		GetProcessInfo(0, 0, 0, 0, rect);
		GetClientSize(width, height, rect[2], rect[3], thread_data);
	}

	inline void GetClientSize(unsigned short &width, unsigned short &height, int win_width, int win_height)
	{
		GetClientSize(width, height, win_width, win_height, GetThreadData());
	}

	inline void GetTimeDate(int t[/* 6 */]) {GetTime(t); GetDate(t + 3);}

	inline void InitMutex(TMutex *mutex) {mutex->mut = 0;}

	inline void InitRecMutex(TRecMutex *mutex) {mutex->mut = 0; mutex->pid = -1;}

	inline bool TryLock(TRecMutex *mutex) {return TryLock(mutex, GetPid());}

	inline bool TryLock(TRecMutex *mutex, TThreadData thread_data) {return TryLock(mutex, GetPid(thread_data));}

	inline void Lock(TRecMutex *mutex) {Lock(mutex, GetPid());}

	inline void Lock(TRecMutex *mutex, TThreadData thread_data) {Lock(mutex, GetPid(thread_data));}

	inline bool LockTime(TRecMutex *mutex, int time) {return LockTime(mutex, time, GetPid());}

	inline bool LockTime(TRecMutex *mutex, int time, TThreadData thread_data)
				{return LockTime(mutex, time, GetPid(thread_data));}

	inline void UnLock(TRecMutex *mutex) {UnLock(mutex, GetPid());}

	inline void UnLock(TRecMutex *mutex, TThreadData thread_data) {UnLock(mutex, GetPid(thread_data));}

	inline int GetThreadNumber() {return _ThreadNumber;}

// Constants from fasm.

#include <me_func.inc>

// Functions.

	unsigned char _HashByte(unsigned int value);
	unsigned short _HashWord(unsigned int value);
	unsigned int _HashDword(unsigned int value);

	void _GetStartData(TStartData &start_data, TThreadData thread_data)
	{
		start_data.Left = (unsigned short)((unsigned long)thread_data[MENUET_THREAD_DATA_X] >> 16);
		start_data.Width = (unsigned short)((unsigned long)thread_data[MENUET_THREAD_DATA_X]);
		start_data.Top = (unsigned short)((unsigned long)thread_data[MENUET_THREAD_DATA_Y] >> 16);
		start_data.Height = (unsigned short)((unsigned long)thread_data[MENUET_THREAD_DATA_Y]);
		GetWindowData(start_data.WinData, thread_data);
	}

	void _SetStartData(const TStartData &start_data, TThreadData thread_data)
	{
		(unsigned long&)thread_data[MENUET_THREAD_DATA_X] =
					((unsigned int)start_data.Left << 16) | start_data.Width;
		(unsigned long&)thread_data[MENUET_THREAD_DATA_Y] =
					((unsigned int)start_data.Top << 16) | start_data.Height;
		SetWindowData(start_data.WinData, thread_data);
	}

	void _ApplyCommonColors(TWindowData &win_data)
	{
		unsigned int colors[10];
		ReadCommonColors(colors);
		win_data.WindowColor = colors[5];
		win_data.HeaderColor = colors[1];
		win_data.BorderColor = colors[0];
		win_data.TitleColor = colors[4];
	}

	void _SetValueFunctionPriority(void *beg, int n)
	{
		int k, i;
		unsigned char num[256];
		for (i = 0; i < 256; i++) num[i] = 0;
		for (k = 0; k < n; k++)
		{
			i = ((unsigned char*)beg + 6*k)[1];
			((unsigned char*)beg + 6*k)[0] = num[i];
			if (num[i] != 255) num[i]++;
		}
	}

	void _CallFunctionPriority(void *beg, void *end, bool reverse = false)
	{
		struct _Local
		{
			static int cmp(void *beg, int i, int j)
			{
				unsigned char *x = (unsigned char*)beg + 6*i;
				unsigned char *y = (unsigned char*)beg + 6*j;
				if (*(unsigned short*)x < *(unsigned short*)y) return -1;
				if (*(unsigned short*)x > *(unsigned short*)y) return 1;
				return 0;
			}

			static void swap(void *beg, int i, int j)
			{
				unsigned char *x = (unsigned char*)beg + 6*i;
				unsigned char *y = (unsigned char*)beg + 6*j;
				short s;
				int t;
				s = *(short*)x; *(short*)x = *(short*)y; *(short*)y = s;
				x += 2; y += 2;
				t = *(int*)x; *(int*)x = *(int*)y; *(int*)y = t;
			}

			static void call(void *beg, int i)
			{
				unsigned char *x = (unsigned char*)beg + 6*i;
				(*(void(**)())(x+2))();
			}
		};

		if (!beg || !end || end <= beg) return;
		int i, j, k, m, n;
		n = ((unsigned char*)end - (unsigned char*)beg) / 6;
		if (n <= 0) return;
		_SetValueFunctionPriority(beg, n);
		m = n; k = n;
		while (m > 1)
		{
			if (k > 0) k--;
			else _Local::swap(beg, 0, --m);
			j = k;
			for (;;)
			{
				 i = j;
				 if (2*i + 1 >= m) break;
				 if (_Local::cmp(beg, 2*i + 1, j) > 0) j = 2*i + 1;
				 if (2*i + 2 < m && _Local::cmp(beg, 2*i + 2, j) > 0) j = 2*i + 2;
				 if (i == j) break;
				 _Local::swap(beg, i, j);
			}
		}
		if (!reverse)
		{
			for (k = 0; k < n; k++) _Local::call(beg, k);
		}
		else
		{
			for (k = n-1; k >= 0; k--) _Local::call(beg, k);
		}
	}

	bool _CallStart(TThreadData thread_data, void *init = 0, void *init_end = 0)
	{
		struct _TThreadDataTemplate
		{
			unsigned int data[12];
		};
		static const _TThreadDataTemplate _ThreadDataTemplate =
			{{3, 0x00320100, 0x00320100, 0x33FFFFFF, 0x806060FF, 0x00000000, 0x00FFFF40, 0, 0, 0, -1, -1}};

		unsigned int pid = GetPid();
		volatile TThreadData *thread_table_item;
		Lock(&_ThreadMutex);
		if (_ExitProcessNow) ExitProcess();
		thread_table_item = &_ThreadTable[_HashByte(pid)];
		thread_data[MENUET_THREAD_DATA_NEXT] = (void*)*thread_table_item;
		(unsigned int&)thread_data[MENUET_THREAD_DATA_PID] = pid;
		*(_TThreadDataTemplate*)(thread_data + MENUET_THREAD_DATA_FLAG) = _ThreadDataTemplate;
		*thread_table_item = thread_data;
		UnLock(&_ThreadMutex);
		if (_ExitProcessNow) ExitProcess();
		_CallFunctionPriority(init, init_end, false);
		TStartData start_data;
		_GetStartData(start_data, thread_data);
//		_ApplyCommonColors(start_data.WinData);
		(unsigned int&)thread_data[MENUET_THREAD_DATA_FLAG] |= 0x40000000;
		thread_data[MENUET_THREAD_DATA_TITLE] = (void*)(&start_data);
		if (!MenuetOnStart(start_data, thread_data)) return false;
		(unsigned int&)thread_data[MENUET_THREAD_DATA_FLAG] &= ~0x40000000;
		_SetStartData(start_data, thread_data);
		return true;
	}

	void _RemoveThreadData(TThreadData thread_data, void *exit = 0, void *exit_end = 0)
	{
		_CallFunctionPriority(exit, exit_end, true);
		volatile TThreadData *thread_table_item;
		Lock(&_ThreadMutex);
		if (_ExitProcessNow) ExitProcess();
		thread_table_item = &_ThreadTable[_HashByte(GetPid(thread_data))];
		while (*thread_table_item)
		{
			if (*thread_table_item == thread_data)
			{
				*thread_table_item = (TThreadData)thread_data[MENUET_THREAD_DATA_NEXT];
				break;
			}
			thread_table_item = (TThreadData*)(*thread_table_item + MENUET_THREAD_DATA_NEXT);
		}
		UnLock(&_ThreadMutex);
		if (_ExitProcessNow) ExitProcess();
	}

	void GetWindowData(TWindowData &win_data, TThreadData thread_data)
	{
		if ((unsigned int)thread_data[MENUET_THREAD_DATA_FLAG] & 0x40000000)
		{
			win_data = ((TStartData*)thread_data[MENUET_THREAD_DATA_TITLE])->WinData;
			return;
		}
		win_data.WindowType = (unsigned short)((unsigned int)thread_data[MENUET_THREAD_DATA_C_WINDOW] >> 24);
		win_data.HeaderType = (unsigned short)((unsigned int)thread_data[MENUET_THREAD_DATA_C_HEADER] >> 24);
		win_data.WindowColor = (unsigned int)thread_data[MENUET_THREAD_DATA_C_WINDOW] & 0xFFFFFF;
		win_data.HeaderColor = (unsigned int)thread_data[MENUET_THREAD_DATA_C_HEADER] & 0xFFFFFF;
		win_data.BorderColor = (unsigned int)thread_data[MENUET_THREAD_DATA_C_BORDER] & 0xFFFFFF;
		win_data.TitleColor = (unsigned int)thread_data[MENUET_THREAD_DATA_C_TITLE] & 0xFFFFFF;
		win_data.Title = (char*)thread_data[MENUET_THREAD_DATA_TITLE];
	}

	void SetWindowData(const TWindowData &win_data, TThreadData thread_data)
	{
		if ((unsigned int)thread_data[MENUET_THREAD_DATA_FLAG] & 0x40000000)
		{
			((TStartData*)thread_data[MENUET_THREAD_DATA_TITLE])->WinData = win_data;
			return;
		}
		(unsigned int&)thread_data[MENUET_THREAD_DATA_C_WINDOW] =
					((unsigned int)win_data.WindowType << 24) | (win_data.WindowColor & 0xFFFFFF);
		(unsigned int&)thread_data[MENUET_THREAD_DATA_C_HEADER] =
					((unsigned int)win_data.HeaderType << 24) | (win_data.HeaderColor & 0xFFFFFF);
		(unsigned int&)thread_data[MENUET_THREAD_DATA_C_BORDER] = win_data.BorderColor & 0xFFFFFF;
		(unsigned int&)thread_data[MENUET_THREAD_DATA_C_TITLE] = win_data.TitleColor & 0xFFFFFF;
		thread_data[MENUET_THREAD_DATA_TITLE] = (void*)win_data.Title;
		Invalidate(1, thread_data);
	}

	void CloseWindow(TThreadData thread_data)
	{
		(unsigned int&)thread_data[MENUET_THREAD_DATA_FLAG] |= 0x80000000;
	}

	void Invalidate(int frame, TThreadData thread_data)
	{
		if (frame < 0) return;
		(unsigned int&)thread_data[MENUET_THREAD_DATA_FLAG] |= (frame ? 3 : 1);
	}

	void* GetPicture(unsigned short &width, unsigned short &height, TThreadData thread_data)
	{
		width = (unsigned short)((unsigned int)thread_data[MENUET_THREAD_DATA_SZ_PICT] >> 16);
		height = (unsigned short)((unsigned int)thread_data[MENUET_THREAD_DATA_SZ_PICT]);
		return (void*)thread_data[MENUET_THREAD_DATA_PICTURE];
	}

	void SetPicture(void *picture, unsigned short width, unsigned short height, TThreadData thread_data)
	{
		thread_data[MENUET_THREAD_DATA_PICTURE] = (void*)picture;
		(unsigned int&)thread_data[MENUET_THREAD_DATA_SZ_PICT] =
					(width == 0 || height == 0) ? 0 : (((unsigned int)width << 16) | height);
		Invalidate(0, thread_data);
	}

	int _GetSkinHeader();

	void GetBorderHeader(unsigned short &border_size, unsigned short &header_size, TThreadData thread_data)
	{
		int win_type = ((unsigned int)thread_data[MENUET_THREAD_DATA_FLAG] & 0x40000000) ?
			((TStartData*)thread_data[MENUET_THREAD_DATA_TITLE])->WinData.WindowType :
		   ((unsigned int)thread_data[MENUET_THREAD_DATA_C_WINDOW] >> 24);
		border_size = MENUET_BORDER_SIZE;
		header_size = short(((win_type & 15) == 3) ? _GetSkinHeader() : MENUET_HEADER_SIZE);
	}

	void GetClientSize(unsigned short &width, unsigned short &height,
						int win_width, int win_height, TThreadData thread_data)
	{
		const int MAX_SIZE = 32767;
		unsigned short border_size, header_size;
		GetBorderHeader(border_size, header_size, thread_data);
		win_width -= 2 * border_size;
		win_height -= border_size + header_size;
		if (win_width < 0) win_width = 0;
		else if (win_width > MAX_SIZE) win_width = MAX_SIZE;
		if (win_height < 0) win_height = 0;
		else if (win_height > MAX_SIZE) win_height = MAX_SIZE;
		width = (unsigned short)win_width;
		height = (unsigned short)win_height;
	}

	void GetMousePosPicture(short &x, short &y)
	{
		unsigned short dx, dy;
		GetMousePosition(x, y);
		GetBorderHeader(dx, dy);
		x -= dx; y -= dy;
	}
}

#else   // def __MENUET__

namespace Menuet
{
	struct TMutex
	{
		unsigned int mut;

		TMutex();
		~TMutex();
	};
#undef  MENUET_MUTEX_INIT
#define MENUET_MUTEX_INIT  TMutex()

	struct TRecMutex
	{
		unsigned int mut;

		TRecMutex();
		~TRecMutex();
	};
#undef  MENUET_REC_MUTEX_INIT
#define MENUET_REC_MUTEX_INIT  TRecMutex()
}

#endif  // else: def __MENUET__

#endif  // ndef __MENUET_H_INCLUDED_

