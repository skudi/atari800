/* (C) 2001  Vasyl Tsvirkunov */
/* Based on Win32 port by  Krzysztof Nikiel */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "config.h"
#include "main.h"
#include "screen.h"
#include "keyboard.h"
#include "sound.h"

extern int atari_main(int argc, char **argv);

LPTSTR myname = TEXT("Pocket Atari");
char* mynameb = "Pocket Atari";
HWND hWndMain;
HINSTANCE myInstance;

static char **gargv = NULL;
static int gargc = 0;

void __cdecl atari_exit(int code)
{
#ifdef MULTITHREADED
	ExitThread(code);
#else
	MsgPump();
#endif
	exit(code);
}

DWORD WINAPI vloop(LPVOID p)
{
	srand(time(0));
	atari_main(gargc, gargv);
	atari_exit(0);
	return 0;
}

static long FAR PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static POINT lastClick;
	static PAINTSTRUCT ps;
	
	switch (message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		hitbutton((short)wParam);
		return 0;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		releasebutton((short)wParam);
		return 0;
	case WM_LBUTTONDOWN:
		lastClick.x = LOWORD(lParam);
		lastClick.y = HIWORD(lParam);
		ClientToScreen(hWnd, &lastClick);
		tapscreen((short)lastClick.x, (short)lastClick.y);
		SetCapture(hWnd);
		return 0;
	case WM_LBUTTONUP:
		untapscreen((short)lastClick.x, (short)lastClick.y);
		ReleaseCapture();
		return 0;
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		
	case WM_SETFOCUS:
	case WM_ACTIVATE:
		gr_resume();
		return 0;
	case WM_KILLFOCUS:
	case WM_HIBERNATE:
		gr_suspend();
		return 0;
	}
#ifdef MULTITHREADED
	return DefWindowProc(hWnd, message, wParam, lParam);
#else
	return 0;
#endif
}

#ifndef MULTITHREADED
void MsgPump()
{
	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		WindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
}
#endif


static BOOL initwin(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASS wc;
	
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL; //LoadIcon(hInstance, IDI_APPLICATION);
	wc.hCursor = NULL;
	wc.hbrBackground = GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = myname;
	RegisterClass(&wc);
	
	hWndMain = CreateWindow(myname,
		myname,
		WS_VISIBLE,
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		NULL,
		NULL,
		hInstance,
		NULL);
	
	if(!hWndMain)
		return 1;
	
	SetWindowPos(hWndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#ifdef MULTITHREADED
	MSG msg;
	HANDLE hth;
#endif
	
	int i;
	static int argc = 0;
	static char args[0x400];
	static char *argv[100];
	
	myInstance = hInstance;
	if(initwin(hInstance, nCmdShow))
	{
		return 1;
	}
	
	if(lpCmdLine)
		WideCharToMultiByte(CP_ACP, 0, lpCmdLine, -1, args, 0x400, NULL, NULL);
	else
		args[0] = 0;
	argv[argc++] = mynameb;
	for(i = 0; i < (sizeof(args) - 1) && args[i]; i++)
	{
		while(args[i] == ' ' && i < (sizeof(args) - 1))
			i++;
		if(args[i] && i < (sizeof(args) - 1))
		{
			argv[argc++] = &args[i];
			if((argc + 1) >= (sizeof(argv) / sizeof(argv[0])))
				break;
		}
		while(args[i] != ' ' && args[i] && i < (sizeof(args) - 1))
			i++;
		args[i] = 0;
	}
	argv[argc] = NULL;
	
	gargv = argv;
	gargc = argc;
	
#ifdef MULTITHREADED
	if((hth = CreateThread(NULL, 0x10000, vloop, NULL, 0, NULL)) == NULL)
		return 1;
	
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	/* wait for the other thread to exit */
	WaitForSingleObject((HANDLE)hth, INFINITE);
	return msg.wParam;
#else
	vloop(NULL);
	return 0;
#endif
}