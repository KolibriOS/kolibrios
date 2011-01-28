#include <windows.h>
#include <string.h>
#include <process.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <deque.h>

#include <menuet.h>
#include <me_heap.h>
#include <me_file.h>

using namespace Menuet;
using namespace std;

const char file_prefix[] = "";
bool WasThreadCreatedBool = false;

struct TExceptToMessageLoop
{
	TExceptToMessageLoop() {}
};

struct TThreadDataStruct
{
	void *user;
	void *stack_begin;
	TWindowData *win_data;
	HWND hwnd;
	int flag;
	unsigned int win_time, me_time;
	void *picture;
	unsigned int picture_width, picture_height;
	deque<unsigned char> *keys;
	unsigned int bmp_data_length;
	unsigned int *bmp_data;
	unsigned int mouse_state;
};

TThreadDataStruct /*__thread*/ ThreadDataStruct;
int nCmdShow;
HINSTANCE hInstance;
const char szWindowClass[] = "Menuet window";

void FinalizeThreadData()
{
	if (ThreadDataStruct.keys)
	{
		delete ThreadDataStruct.keys;
		ThreadDataStruct.keys = 0;
	}
	if (ThreadDataStruct.bmp_data)
	{
		delete[] ThreadDataStruct.bmp_data;
		ThreadDataStruct.bmp_data = 0;
	}
}

unsigned int CalculateNewTime()
{
	unsigned int t = GetTickCount();
	unsigned int dt = (unsigned int)(t - ThreadDataStruct.win_time) / 10U;
	ThreadDataStruct.me_time += dt;
	ThreadDataStruct.win_time += dt * 10;
	return t;
}

void DrawPicture(HDC hdc)
{
	TRecMutex xm;
	InitRecMutex(&xm);
	Lock(&xm);
	UnLock(&xm);

	int w = ThreadDataStruct.picture_width, h = ThreadDataStruct.picture_height;
	RECT rect;
	if (!ThreadDataStruct.picture || !ThreadDataStruct.hwnd || w <= 0 || h <= 0) return;
	if (GetClientRect(ThreadDataStruct.hwnd, &rect))
	{
		rect.right -= rect.left; rect.left = 0;
		rect.bottom -= rect.top; rect.top = 0;
		if (rect.right <= 0 || rect.bottom <= 0) return;
		if (w > rect.right) w = rect.right;
		if (h > rect.bottom) h = rect.bottom;
	}
	if (!ThreadDataStruct.bmp_data || ThreadDataStruct.bmp_data_length < w * h)
	{
		if (ThreadDataStruct.bmp_data) delete[] ThreadDataStruct.bmp_data;
		ThreadDataStruct.bmp_data_length = w * h;
		ThreadDataStruct.bmp_data = new unsigned int[ThreadDataStruct.bmp_data_length];
	}
	int i;
	unsigned char *p = (unsigned char*)ThreadDataStruct.picture;
	for (i = 0; i < w * h; i++)
	{
		ThreadDataStruct.bmp_data[i] = ((unsigned int)p[0]) +
				((unsigned int)p[1] << 8) + ((unsigned int)p[2] << 16);
		p += 3;
	}
	HBITMAP bitmap = CreateBitmap(w, h, 1, 32, ThreadDataStruct.bmp_data);
	if (bitmap)
	{
		HDC memdc = CreateCompatibleDC(hdc);
		if (memdc)
		{
			SelectObject(memdc, bitmap);
			BitBlt(hdc, 0, 0, w, h, memdc, 0, 0, SRCCOPY);
			DeleteObject(memdc);
		}
		DeleteObject(bitmap);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int timeout = 20;
	unsigned int t;
	PAINTSTRUCT ps;
	HDC hdc;
	if (hWnd == ThreadDataStruct.hwnd && ThreadDataStruct.flag != -1)
	{
		int window_rect[4];
		switch (message)
		{
		case WM_CREATE:
			SetTimer(hWnd, 0, timeout, NULL);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 0);
			return 0;
		case WM_TIMER:
			t = CalculateNewTime();
			while (MenuetOnIdle((TThreadData)(&ThreadDataStruct)) == 0 &&
					GetTickCount() - t + 2 < timeout);
			return 0;
		case WM_MOUSEMOVE:
			MenuetOnMouse((TThreadData)(&ThreadDataStruct));
			return 0;
		case WM_LBUTTONDOWN:
			if (!ThreadDataStruct.mouse_state) SetCapture(hWnd);
			ThreadDataStruct.mouse_state |= 1;
			MenuetOnMouse((TThreadData)(&ThreadDataStruct));
			return 0;
		case WM_LBUTTONUP:
			if (ThreadDataStruct.mouse_state & 1)
			{
				ThreadDataStruct.mouse_state &= ~1;
				if (!ThreadDataStruct.mouse_state) ReleaseCapture();
				MenuetOnMouse((TThreadData)(&ThreadDataStruct));
			}
			return 0;
		case WM_RBUTTONDOWN:
			if (!ThreadDataStruct.mouse_state) SetCapture(hWnd);
			ThreadDataStruct.mouse_state |= 2;
			MenuetOnMouse((TThreadData)(&ThreadDataStruct));
			return 0;
		case WM_RBUTTONUP:
			if (ThreadDataStruct.mouse_state & 2)
			{
				ThreadDataStruct.mouse_state &= ~2;
				if (!ThreadDataStruct.mouse_state) ReleaseCapture();
				MenuetOnMouse((TThreadData)(&ThreadDataStruct));
			}
			return 0;
		case WM_CAPTURECHANGED:
			if (ThreadDataStruct.mouse_state)
			{
				ThreadDataStruct.mouse_state = 0;
				MenuetOnMouse((TThreadData)(&ThreadDataStruct));
			}
			return 0;
		//case WM_SYSKEYDOWN: case WM_KEYDOWN:
		case WM_CHAR:
			ThreadDataStruct.keys->push_back((unsigned char)wParam);
			MenuetOnKeyPress((TThreadData)(&ThreadDataStruct));
			return 0;
		case WM_SIZE:
			GetProcessInfo(0, 0, 0, 0, window_rect);
			MenuetOnSize(window_rect, (TThreadData)(&ThreadDataStruct));
			InvalidateRect(hWnd, 0, 0);
			return 0;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			if (ThreadDataStruct.picture) DrawPicture(hdc);
			EndPaint(hWnd, &ps);
			return 0;
		case WM_CLOSE:
			if (MenuetOnClose((TThreadData)(&ThreadDataStruct)))
			{
				ThreadDataStruct.flag = -1;
			}
			else return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

ATOM MyRegisterClass()
{
	HBRUSH background = CreateSolidBrush(RGB(0, 0, 0));
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= 0;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= background;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= 0;
	ATOM ret = RegisterClassEx(&wcex);
	DeleteObject(background);
	return ret;
}

HWND InitInstance(int x, int y, int w, int h)
{
	HWND hWnd;
	MyRegisterClass();
	DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, exstyle = 0;
	hWnd = CreateWindowEx(exstyle, szWindowClass, ThreadDataStruct.win_data->Title, style,
				x, y, w, h, NULL, NULL, hInstance, NULL);
	if (!hWnd) return NULL;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return hWnd;
}

int ThreadMainProc(void *user)
{
	ThreadMain(user, 0);
	return 0;
}

namespace Menuet
{
	void Main() {ThreadMain();}

	void* ThreadMain(void *user, void *stack_begin)
	{
		TStartData start_data;
		ThreadDataStruct.user = user;
		ThreadDataStruct.stack_begin = stack_begin;
		ThreadDataStruct.win_data = &start_data.WinData;
		ThreadDataStruct.hwnd = 0;
		ThreadDataStruct.flag = 0;
		ThreadDataStruct.win_time = GetTickCount();
		ThreadDataStruct.me_time = ThreadDataStruct.win_time / 10;
		ThreadDataStruct.keys = new deque<unsigned char>;
		ThreadDataStruct.bmp_data_length = 0;
		ThreadDataStruct.bmp_data = 0;
		ThreadDataStruct.mouse_state = 0;
		start_data.Left = 50; start_data.Width = 256;
		start_data.Top = 50; start_data.Height = 256;
		start_data.WinData.WindowType = 0x03;
		start_data.WinData.HeaderType = 0x80;
		start_data.WinData.WindowColor = 0xFFFFFF;
		start_data.WinData.HeaderColor = 0x6060FF;
		start_data.WinData.BorderColor = 0x000000;
		start_data.WinData.TitleColor = 0xFFFF40;
		start_data.WinData.Title = 0;
		if (MenuetOnStart(start_data, (TThreadData)(&ThreadDataStruct)))
		{
			while (ThreadDataStruct.flag < 0)
			{
				ThreadDataStruct.flag &= ~0x80000000;
				if (MenuetOnClose((TThreadData)(&ThreadDataStruct)))
				{
					ThreadDataStruct.flag = -1;
					break;
				}
			}
			if (ThreadDataStruct.flag >= 0)
			{
				assert((ThreadDataStruct.hwnd = InitInstance(start_data.Left,
							start_data.Top, start_data.Width, start_data.Height)) != NULL);
				assert(SendMessage(ThreadDataStruct.hwnd, WM_CREATE, 0, 0) == 0);
				MSG msg;
				HACCEL hAccelTable = 0;
				while (GetMessage(&msg, NULL, 0, 0))
				{
					if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
			}
			assert(ThreadDataStruct.flag == -1);
		}
		FinalizeThreadData();
		return ThreadDataStruct.stack_begin;
	}

	void GetWindowData(TWindowData &win_data)
	{
		win_data = *ThreadDataStruct.win_data;
	}

	void GetWindowData(TWindowData &win_data, TThreadData thread_data)
	{
		win_data = *((TThreadDataStruct*)thread_data)->win_data;
	}

	void SetWindowData(const TWindowData &win_data)
	{
		*ThreadDataStruct.win_data = win_data;
		if (ThreadDataStruct.hwnd)
		{
			SetWindowText(ThreadDataStruct.hwnd, ThreadDataStruct.win_data->Title);
			InvalidateRect(ThreadDataStruct.hwnd, NULL, FALSE);
		}
	}

	void SetWindowData(const TWindowData &win_data, TThreadData thread_data)
	{
		*((TThreadDataStruct*)thread_data)->win_data = win_data;
		if (((TThreadDataStruct*)thread_data)->hwnd)
		{
			SetWindowText(((TThreadDataStruct*)thread_data)->hwnd,
						((TThreadDataStruct*)thread_data)->win_data->Title);
			InvalidateRect(((TThreadDataStruct*)thread_data)->hwnd, NULL, FALSE);
		}
	}

	void CloseWindow()
	{
		if (ThreadDataStruct.hwnd)
		{
			SendMessage(ThreadDataStruct.hwnd, WM_CLOSE, 0, 0);
		}
		else ThreadDataStruct.flag |= 0x80000000;
	}

	void CloseWindow(TThreadData thread_data)
	{
		if (((TThreadDataStruct*)thread_data)->hwnd)
		{
			SendMessage(((TThreadDataStruct*)thread_data)->hwnd, WM_CLOSE, 0, 0);
		}
		else ((TThreadDataStruct*)thread_data)->flag |= 0x80000000;
	}

	void Redraw(int /*frame*/)
	{
		if (ThreadDataStruct.hwnd)
		{
			InvalidateRect(ThreadDataStruct.hwnd, NULL, FALSE);
			SendMessage(ThreadDataStruct.hwnd, WM_PAINT, 0, 0);
		}
	}

	void Redraw(int /*frame*/, TThreadData thread_data)
	{
		if (((TThreadDataStruct*)thread_data)->hwnd)
		{
			InvalidateRect(((TThreadDataStruct*)thread_data)->hwnd, NULL, FALSE);
			SendMessage(((TThreadDataStruct*)thread_data)->hwnd, WM_PAINT, 0, 0);
		}
	}

	void Invalidate(int /*frame*/)
	{
		if (ThreadDataStruct.hwnd)
		{
			InvalidateRect(ThreadDataStruct.hwnd, NULL, FALSE);
		}
	}

	void Invalidate(int /*frame*/, TThreadData thread_data)
	{
		if (((TThreadDataStruct*)thread_data)->hwnd)
		{
			InvalidateRect(((TThreadDataStruct*)thread_data)->hwnd, NULL, FALSE);
		}
	}

	void MoveWindow(const int window_rect[/* 4 */])
	{
		if (!ThreadDataStruct.hwnd) return;
		RECT rect;
		if (window_rect[0] == -1 || window_rect[1] == -1 ||
			window_rect[2] == -1 || window_rect[3] == -1)
		{
			if (!GetWindowRect(ThreadDataStruct.hwnd, &rect)) return;
			::MoveWindow(ThreadDataStruct.hwnd,
					(window_rect[0] == -1) ? rect.left : window_rect[0],
					(window_rect[1] == -1) ? rect.top : window_rect[1],
					(window_rect[2] == -1) ? (rect.right - rect.left) : window_rect[2],
					(window_rect[3] == -1) ? (rect.bottom - rect.top) : window_rect[3], TRUE);
		}
		else
		{
			::MoveWindow(ThreadDataStruct.hwnd, window_rect[0],
					window_rect[1], window_rect[2], window_rect[3], TRUE);
		}
	}

	void Abort()
	{
		if (ThreadDataStruct.hwnd) KillTimer(ThreadDataStruct.hwnd, 0);
		abort();
	}

	void ExitProcess() {::ExitProcess(0);}

	void ExitThread() {FinalizeThreadData(); ::ExitThread(0);}

	void ExitThread(TThreadData) {FinalizeThreadData(); ::ExitThread(0);}

	void ReturnMessageLoop()
	{
		TExceptToMessageLoop ex;
		throw(ex);
	}

	void ReturnMessageLoop(TThreadData)
	{
		TExceptToMessageLoop ex;
		throw(ex);
	}

	void Delay(unsigned int time) {Sleep(time * 10);}

	unsigned int Clock() {CalculateNewTime(); return ThreadDataStruct.me_time;}

	int GetPackedTime()
	{
		SYSTEMTIME time;
		GetSystemTime(&time);
		int t;
		t = (time.wSecond / 10) * 16 + (time.wSecond % 10);
		t = (time.wMinute / 10) * 16 + (time.wMinute % 10) + (t << 8);
		t = (time.wHour / 10) * 16 + (time.wHour % 10) + (t << 8);
		return t;
	}

	void GetTime(int t[/* 3 */])
	{
		SYSTEMTIME time;
		GetSystemTime(&time);
		t[0] = time.wSecond;
		t[1] = time.wMinute;
		t[2] = time.wHour;
	}

	int GetPackedDate()
	{
		SYSTEMTIME time;
		GetSystemTime(&time);
		int t;
		t = ((time.wYear / 10) % 10) * 16 + (time.wYear % 10);
		t = (time.wDay / 10) * 16 + (time.wDay % 10) + (t << 8);
		t = (time.wMonth / 10) * 16 + (time.wMonth % 10) + (t << 8);
		return t;
	}

	void GetDate(int d[/* 3 */])
	{
		SYSTEMTIME time;
		GetSystemTime(&time);
		d[0] = time.wDay;
		d[1] = time.wMonth;
		d[2] = time.wYear;
	}

	void GetTimeDate(int t[/* 6 */])
	{
		SYSTEMTIME time;
		GetSystemTime(&time);
		t[0] = time.wSecond;
		t[1] = time.wMinute;
		t[2] = time.wHour;
		t[3] = time.wDay;
		t[4] = time.wMonth;
		t[5] = time.wYear;
	}

	void ReadCommonColors(unsigned int colors[/* 10 */])
	{
		int i;
		for (i = 0; i < 10; i++) colors[i] = 0;
	}

	unsigned int GetProcessInfo(unsigned int *use_cpu, char process_name[/* 13 */], unsigned int *use_memory,
								unsigned int *pid, int window_rect[/* 4 */], unsigned int pid_for)
	{
		if (use_cpu) *use_cpu = 0;
		if (process_name) strcpy(process_name, "noname");
		if (use_memory) *use_memory = 0;
		if (pid)
		{
			if ((pid_for | 15) == -1) pid_for = getpid();
			*pid = pid_for;
		}
		if (window_rect)
		{
			RECT rect;
			if (ThreadDataStruct.hwnd && GetWindowRect(ThreadDataStruct.hwnd, &rect))
			{
				window_rect[0] = rect.left;
				window_rect[1] = rect.top;
				window_rect[2] = rect.right - rect.left;
				window_rect[3] = rect.bottom - rect.top;
			}
			else
			{
				window_rect[0] = 0; window_rect[1] = 0;
				window_rect[2] = 0; window_rect[3] = 0;
			}
		}
		return 1;
	}

	unsigned int GetPid() {return GetCurrentThreadId();}

	unsigned int GetPid(TThreadData /*thread_data*/) {return GetCurrentThreadId();}

	TThreadData GetThreadData() {return (TThreadData)(&ThreadDataStruct);}

	TThreadData GetThreadData(unsigned int /*pid*/) {return (TThreadData)(&ThreadDataStruct);}

	void* GetPicture(unsigned short &width, unsigned short &height)
	{
		width = (unsigned short)ThreadDataStruct.picture_width;
		height = (unsigned short)ThreadDataStruct.picture_height;
		return ThreadDataStruct.picture;
	}

	void* GetPicture(unsigned short &width, unsigned short &height, TThreadData thread_data)
	{
		width = (unsigned short)((TThreadDataStruct*)thread_data)->picture_width;
		height = (unsigned short)((TThreadDataStruct*)thread_data)->picture_height;
		return ((TThreadDataStruct*)thread_data)->picture;
	}
	
	void SetPicture(void *picture, unsigned short width, unsigned short height)
	{
		ThreadDataStruct.picture_width = width;
		ThreadDataStruct.picture_height = height;
		ThreadDataStruct.picture = picture;
		if (ThreadDataStruct.hwnd)
		{
			InvalidateRect(ThreadDataStruct.hwnd, NULL, FALSE);
		}
	}

	void SetPicture(void *picture, unsigned short width, unsigned short height, TThreadData thread_data)
	{
		((TThreadDataStruct*)thread_data)->picture_width = width;
		((TThreadDataStruct*)thread_data)->picture_height = height;
		((TThreadDataStruct*)thread_data)->picture = picture;
		if (((TThreadDataStruct*)thread_data)->hwnd)
		{
			InvalidateRect(((TThreadDataStruct*)thread_data)->hwnd, NULL, FALSE);
		}
	}

	void GetBorderHeader(unsigned short &border_size, unsigned short &header_size)
	{
		border_size = (unsigned short)GetSystemMetrics(SM_CXFRAME);
		header_size = (unsigned short)(GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION));
	}

	void GetBorderHeader(unsigned short &border_size, unsigned short &header_size, TThreadData /*thread_data*/)
	{
		border_size = (unsigned short)GetSystemMetrics(SM_CXFRAME);
		header_size = (unsigned short)(GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION));
	}

	void GetClientSize(unsigned short &width, unsigned short &height)
	{
		if (!ThreadDataStruct.hwnd) {width = 0; height = 0; return;}
		RECT rect;
		GetClientRect(ThreadDataStruct.hwnd, &rect);
		width = (unsigned short)(rect.right - rect.left);
		height = (unsigned short)(rect.bottom - rect.top);
	}

	void GetClientSize(unsigned short &width, unsigned short &height, TThreadData thread_data)
	{
		if (!((TThreadDataStruct*)thread_data)->hwnd) {width = 0; height = 0; return;}
		RECT rect;
		GetClientRect(((TThreadDataStruct*)thread_data)->hwnd, &rect);
		width = (unsigned short)(rect.right - rect.left);
		height = (unsigned short)(rect.bottom - rect.top);
	}

	void GetClientSize(unsigned short &width, unsigned short &height, int win_width, int win_height)
	{
		win_width -= 2*GetSystemMetrics(SM_CXFRAME);
		win_height -= 2*GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);
		if (win_width < 0) win_width = 0;
		if (win_height < 0) win_height = 0;
		width = (unsigned short)win_width; height = (unsigned short)win_height;
	}

	void GetClientSize(unsigned short &width, unsigned short &height,
						int win_width, int win_height, TThreadData /*thread_data*/)
	{
		win_width -= 2*GetSystemMetrics(SM_CXFRAME);
		win_height -= 2*GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);
		if (win_width < 0) win_width = 0;
		if (win_height < 0) win_height = 0;
		width = (unsigned short)win_width; height = (unsigned short)win_height;
	}

	void GetScreenSize(unsigned short &width, unsigned short &height)
	{
		width = (unsigned short)GetSystemMetrics(SM_CXFULLSCREEN);
		height = (unsigned short)GetSystemMetrics(SM_CYFULLSCREEN);
	}

	TMutex::TMutex() {mut = (unsigned int)CreateMutex(NULL, FALSE, NULL);}

	TMutex::~TMutex() {if (mut) {CloseHandle((HANDLE)mut); mut = 0;}}

	TRecMutex::TRecMutex() {mut = (unsigned int)CreateMutex(NULL, FALSE, NULL);}

	TRecMutex::~TRecMutex() {if (mut) {CloseHandle((HANDLE)mut); mut = 0;}}

	void InitMutex(TMutex *mutex) {if (!mutex->mut) *mutex = TMutex();}

	void InitRecMutex(TRecMutex *mutex) {if (!mutex->mut) *mutex = TRecMutex();}

	bool TryLock(TMutex *mutex)
	{
		DWORD ret = WaitForSingleObject((HANDLE)mutex->mut, 0);
		return ret == WAIT_OBJECT_0 || ret == WAIT_ABANDONED;
	}

	bool TryLock(TRecMutex *mutex, unsigned int /*pid*/)
	{
		DWORD ret = WaitForSingleObject((HANDLE)mutex->mut, 0);
		return ret == WAIT_OBJECT_0 || ret == WAIT_ABANDONED;
	}

	bool TryLock(TRecMutex *mutex)
	{
		DWORD ret = WaitForSingleObject((HANDLE)mutex->mut, 0);
		return ret == WAIT_OBJECT_0 || ret == WAIT_ABANDONED;
	}

	bool TryLock(TRecMutex *mutex, TThreadData /*thread_data*/)
	{
		DWORD ret = WaitForSingleObject((HANDLE)mutex->mut, 0);
		return ret == WAIT_OBJECT_0 || ret == WAIT_ABANDONED;
	}

	void Lock(TMutex *mutex)
	{
		WaitForSingleObject((HANDLE)mutex->mut, INFINITE);
	}

	void Lock(TRecMutex *mutex, unsigned int /*pid*/)
	{
		WaitForSingleObject((HANDLE)mutex->mut, INFINITE);
	}

	void Lock(TRecMutex *mutex)
	{
		WaitForSingleObject((HANDLE)mutex->mut, INFINITE);
	}

	void Lock(TRecMutex *mutex, TThreadData /*thread_data*/)
	{
		WaitForSingleObject((HANDLE)mutex->mut, INFINITE);
	}

	bool LockTime(TMutex *mutex, unsigned int time)
	{
		DWORD ret = WaitForSingleObject((HANDLE)mutex->mut, time * 10);
		return ret == WAIT_OBJECT_0 || ret == WAIT_ABANDONED;
	}

	bool LockTime(TRecMutex *mutex, unsigned int time, unsigned int /*pid*/)
	{
		DWORD ret = WaitForSingleObject((HANDLE)mutex->mut, time * 10);
		return ret == WAIT_OBJECT_0 || ret == WAIT_ABANDONED;
	}

	bool LockTime(TRecMutex *mutex, unsigned int time)
	{
		DWORD ret = WaitForSingleObject((HANDLE)mutex->mut, time * 10);
		return ret == WAIT_OBJECT_0 || ret == WAIT_ABANDONED;
	}

	bool LockTime(TRecMutex *mutex, unsigned int time, TThreadData /*thread_data*/)
	{
		DWORD ret = WaitForSingleObject((HANDLE)mutex->mut, time * 10);
		return ret == WAIT_OBJECT_0 || ret == WAIT_ABANDONED;
	}

	void UnLock(TMutex *mutex)
	{
		ReleaseMutex((HANDLE)mutex->mut);
	}

	void UnLock(TRecMutex *mutex, unsigned int /*pid*/)
	{
		ReleaseMutex((HANDLE)mutex->mut);
	}

	void UnLock(TRecMutex *mutex)
	{
		ReleaseMutex((HANDLE)mutex->mut);
	}

	void UnLock(TRecMutex *mutex, TThreadData /*thread_data*/)
	{
		ReleaseMutex((HANDLE)mutex->mut);
	}

	void DebugPutChar(char c)
	{
		DWORD num_written;
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), &c, 1, &num_written, NULL);
	}

	void DebugPutString(const char *s)
	{
		DWORD num_written;
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), s, strlen(s), &num_written, NULL);
	}

	int GetKey()
	{
		if (ThreadDataStruct.keys->empty()) return -1;
		else
		{
			unsigned char c = ThreadDataStruct.keys->front();
			ThreadDataStruct.keys->pop_front();
			return c;
		}
	}

	int GetMouseButton()
	{
		return ThreadDataStruct.mouse_state;
	}

	void GetMousePosition(short &x, short &y, bool absolute)
	{
		POINT point;
		if (!GetCursorPos(&point)) {x = -1; y = -1;}
		else if (absolute) {x = (short)point.x; y = (short)point.y;}
		else
		{
			RECT rect;
			if (!ThreadDataStruct.hwnd || !GetWindowRect(ThreadDataStruct.hwnd, &rect))
			{
				x = -1; y = -1;
			}
			else
			{
				x = (short)(point.x - rect.left);
				y = (short)(point.y - rect.top);
			}
		}
	}

	void GetMousePosPicture(short &x, short &y)
	{
		POINT point;
		if (!GetCursorPos(&point)) {x = -1; y = -1;}
		else if (!ThreadDataStruct.hwnd || !ScreenToClient(ThreadDataStruct.hwnd, &point))
		{
			x = -1; y = -1;
		}
		else
		{
			x = (short)point.x;
			y = (short)point.y;
		}
	}

	bool WasThreadCreated() {return WasThreadCreatedBool;}
	
	unsigned int CreateThread(void *user, unsigned int stack_size, void* /*stack_end*/)
	{
		unsigned long pid = -1;
		WasThreadCreatedBool = true;
		if (!::CreateThread(NULL, stack_size, (LPTHREAD_START_ROUTINE)ThreadMainProc, user, 0, &pid))
		{
			return -1;
		}
		return pid;
	}

	unsigned int StrLen(const char *str) {return ::strlen(str);}

	char *StrCopy(char *dest, const char *src) {return ::strcpy(dest, src);}

	void *MemCopy(void *dest, const void *src, unsigned int n) {return ::memcpy(dest, src, n);}

	void *MemSet(void *s, char c, unsigned int n) {return ::memset(s, c, n);}

	double Floor(double x) {return floor(x);}

	void *Alloc(unsigned int size) {return malloc(size);}

	void *ReAlloc(void *mem, unsigned int size) {return realloc(mem, size);}
	
	void Free(void *mem) {free(mem);}

	TFileData FileOpen(const char *name, unsigned int /*buffer_length*/)
	{
		if (!name || !name[0]) return 0;
		TFileData file_data = (TFileData)Alloc(sizeof(unsigned int) +
					strlen(file_prefix) + strlen(name) + 1);
		if (!file_data) return 0;
		file_data->data = 0;
		strcpy((char*)file_data + sizeof(unsigned int), file_prefix);
		strcat((char*)file_data + sizeof(unsigned int), name);
		return file_data;
	}

	int FileClose(TFileData file_data)
	{
		if (!file_data) return -1;
		if (file_data->data) CloseHandle((HANDLE)file_data->data);
		Free(file_data);
		return 0;
	}

	bool FileEof(TFileData file_data)
	{
		unsigned int pos;
		if (FileTestRead(file_data) < 0) return false;
		pos = SetFilePointer((HANDLE)file_data->data, 0, NULL, FILE_CURRENT);
		if (pos == -1) return false;
		return pos >= GetFileSize((HANDLE)file_data->data, NULL);
	}

	unsigned int FileGetPosition(TFileData file_data)
	{
		unsigned int pos;
		if (FileTestRead(file_data) < 0) return 0;
		pos = SetFilePointer((HANDLE)file_data->data, 0, NULL, FILE_CURRENT);
		return (pos == -1) ? 0 : pos;
	}

	void FileSetPosition(TFileData file_data, unsigned int pos)
	{
		if (FileTestRead(file_data) < 0) return;
		SetFilePointer((HANDLE)file_data->data, pos, NULL, FILE_BEGIN);
	}

	void FileReset(TFileData file_data)
	{
		if (!file_data || !file_data->data) return;
		FlushFileBuffers((HANDLE)file_data->data);
	}

	unsigned int FileGetLength(TFileData file_data)
	{
		if (FileTestRead(file_data) < 0) return -1;
		return GetFileSize((HANDLE)file_data->data, NULL);
	}

	int FileTestRead(TFileData file_data)
	{
		if (!file_data) return -1;
		if (!file_data->data)
		{
			file_data->data = (unsigned int)CreateFile((char*)file_data + sizeof(unsigned int),
					GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
			if (!file_data->data) return -512;
		}
		return 0;
	}

	int FileRead(TFileData file_data, void *mem, int size)
	{
		if (!file_data || !mem || size <= 0) return -1;
		int res = FileTestRead(file_data);
		if (res < 0) return res;
		if (!ReadFile((HANDLE)file_data->data, mem, size, (unsigned long*)&res, NULL))
		{
			return -512;
		}
		return (res >= 0) ? res : (-1);
	}
}

bool CheckAllocConsole(LPSTR lpCmdLine)
{
	char Console[] = "-console";
	int ConsoleL = ::strlen(Console);
	char *s;
	for (s = lpCmdLine; *s; s++)
	{
		if ((s == lpCmdLine || isspace(s[-1])) && memcmp(s, Console, ConsoleL) == 0 &&
			(!s[ConsoleL] || isspace(s[ConsoleL])))
		{
			AllocConsole();
			SetConsoleTitle("Debug Console");
			return true;
		}
	}
	return false;
}

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE /*hPrevInstance*/,
					 LPSTR     lpCmdLine,
					 int       nCmdShow)
{
	::nCmdShow = nCmdShow;
	::hInstance = hInstance;
	CheckAllocConsole(lpCmdLine);
	Main();
	//::ExitThread(0);
	return 0;
}

