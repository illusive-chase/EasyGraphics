#if defined(__linux) || defined(__linux__)
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <cstdint>
#include <linux/input.h>
#include "SystemIO.hh"


struct LinuxRender {
public:
	friend class RenderImpl;	
private:
	fb_var_screeninfo vinfo = {};
	fb_fix_screeninfo finfo = {};
	long screensize = 0;
	long width = 0, height = 0;
	char* fbp = nullptr;
	char* mbp = nullptr;
	int fp = 0;
	int fevent0 = 0;
	int fmouse0 = 0;
	int mouse_click_status[3] = {};
	int mouse_move_status[3] = {};

	LinuxRender() {
		fp = open("/dev/fb0", O_RDWR);
		if (fp < 0) printf("Error: Fail to open device\n"), exit(1);
		fevent0 = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
		if (fevent0 < 0) printf("Error: Fail to open device\n"), exit(1);
		fmouse0 = open("/dev/input/mouse0", O_RDONLY | O_NONBLOCK);
		if (fmouse0 < 0) printf("Error: Fail to open device\n"), exit(1);
		if (ioctl(fp, FBIOGET_FSCREENINFO, &finfo)) printf("Error: Fail to read fixed infor\n"), exit(1);
		if (ioctl(fp, FBIOGET_VSCREENINFO, &vinfo)) printf("Error: Fail to read variable info\n"), exit(1);
		screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
		width = vinfo.xres;
		height = vinfo.yres;
		if (width != 800 || height != 480) printf("Error: Unexpected screeb size (%u, %u) instead of (800, 480)\n", width, height), exit(1);
		if (vinfo.bits_per_pixel != 32) printf("Error: Unexpected bits per pixel %u instead of 32\n", vinfo.bits_per_pixel), exit(1);
		fbp = (char*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
		if (fbp == MAP_FAILED) printf("Error: Failed to map framebuffer to memory\n"), exit(1);
		mbp = new char[screensize];
	}

	~LinuxRender() {
		munmap(fbp, screensize);
		close(fp);
		close(fevent0);
		close(fmouse0);
		delete[] mbp;
	}

	static LinuxRender& instance() {
		static LinuxRender content;
		return content;
	}

	void sync() {
		memcpy(fbp, mbp, screensize);
		memset(mbp, 0, screensize);
	}

	int* check_mouse_click() {
		input_event ts = {};
		mouse_click_status[0] = 0;
		while (read(fevent0, &ts, sizeof(ts)) >= 0) {
			if (ts.type == EV_ABS) {
				if (ts.code == ABS_X) mouse_click_status[1] = ts.value * 800 / 1024;
				else if (ts.code == ABS_Y) mouse_click_status[2] = ts.value * 480 / 600;
			} else if (ts.type == EV_KEY && ts.code == BTN_TOUCH) {
				mouse_click_status[0] = ts.value + 1;
			}
		}
		return mouse_click_status;
	}

	int* check_mouse_move() {
		uint8_t buff[3] = {};
		memset(mouse_move_status, 0, sizeof(mouse_move_status));
		while (read(fmouse0, buff, sizeof(buff)) >= 0) {
			mouse_move_status[0] = 1;
			mouse_move_status[1] += static_cast<int>(static_cast<int8_t>(buff[1]));
			mouse_move_status[2] += static_cast<int>(static_cast<int8_t>(buff[2]));
		}
		return mouse_move_status;
	}


};


int RenderImpl::Width() {
	return LinuxRender::instance().width;
}

int RenderImpl::Height() {
	return LinuxRender::instance().height;
}

uint8_t* RenderImpl::Data() {
	return reinterpret_cast<uint8_t*>(LinuxRender::instance().mbp);
}

void RenderImpl::Render() {
	LinuxRender::instance().sync();
}

int* RenderImpl::MouseClick() {
	return LinuxRender::instance().check_mouse_click();
}

int* RenderImpl::MouseMove() {
	return LinuxRender::instance().check_mouse_move();
}

#else
#if defined(_WIN32) || defined(WIN32) || defined(WIN64)

#include "SystemIO.hh"
#include "windows/resource.h"
#include "windows/framework.h"
#include <Windows.h>
#include <cstdio>

#if 1
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif
#define MAX_LOADSTRING 100

struct WinRender {
public:
    friend struct RenderImpl;

private:
    HDC hdc;
    HDC mdc;
    HBITMAP hbmp;
    BITMAPINFO bmi = {};
    void* content;
    HINSTANCE hInst;
    HWND      gHWND;
    WCHAR szTitle[MAX_LOADSTRING];
    WCHAR szWindowClass[MAX_LOADSTRING];
    int mouse_click_status[3] = {};

    WinRender() {
        int status = RegisterWindouws();
        if (status) exit(status);
        hdc = GetDC(gHWND);
        mdc = CreateCompatibleDC(hdc);
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = 800;
        bmi.bmiHeader.biHeight = 480;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 800 * 480 * 4;
        hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&content, 0, 0);
        if (hbmp) SelectObject(mdc, hbmp);
    }

    ~WinRender() { DeleteObject(hbmp); DeleteDC(mdc); ReleaseDC(gHWND, hdc); }

    ATOM                MyRegisterClass(HINSTANCE hInstance);
    BOOL                InitInstance(HINSTANCE, int);
    int                 RegisterWindouws();
    void                GetWinMessage();
    static LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
    static INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

    static WinRender& instance() {
        static WinRender r;
        return r;
    }

};

// 此代码模块中包含的函数的前向声明:



int WinRender::RegisterWindouws() {
    // 初始化全局字符串
    HINSTANCE hInstance = ::GetModuleHandle(NULL);
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EASYGRAPHICS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, SW_SHOW)) {
        return FALSE;
    }
    return 0;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM WinRender::MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EASYGRAPHICS));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_EASYGRAPHICS);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL WinRender::InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    gHWND = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, 0, 800 + 15, 480 + 60, nullptr, nullptr, hInstance, nullptr);

    if (!gHWND)
    {
        return FALSE;
    }

    ShowWindow(gHWND, nCmdShow);
    UpdateWindow(gHWND);

    return TRUE;
}


void WinRender::GetWinMessage() {
    HACCEL hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_EASYGRAPHICS));

    MSG msg;

    bool peek = false;

    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        peek = true;
        if (msg.message == WM_QUIT) exit(0);
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    if (!peek) mouse_click_status[0] = 0;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WinRender::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int mouse_status = -1;
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 分析菜单选择:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(instance().hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_LBUTTONDOWN:
    {
        mouse_status = 2;
        instance().mouse_click_status[1] = LOWORD(lParam);
        instance().mouse_click_status[2] = HIWORD(lParam);
    }
    break;
    case WM_LBUTTONUP:
    {
        mouse_status = 1;
        instance().mouse_click_status[1] = LOWORD(lParam);
        instance().mouse_click_status[2] = HIWORD(lParam);
    }
    break;
    case WM_MOUSEMOVE:
    {
        if (wParam != MK_LBUTTON)
            mouse_status = instance().mouse_click_status[0] == 2 ? 1 : 0;
        else
            mouse_status = 2;
        instance().mouse_click_status[1] = LOWORD(lParam);
        instance().mouse_click_status[2] = HIWORD(lParam);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    if (mouse_status < 0) instance().mouse_click_status[0] = 0;
    else instance().mouse_click_status[0] = mouse_status;
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK WinRender::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


int RenderImpl::Width() {
    return 800;
}


int RenderImpl::Height() {
    return 480;
}

uint8_t* RenderImpl::Data() {
    return reinterpret_cast<uint8_t*>(WinRender::instance().content);
}

void RenderImpl::Render() {
    StretchBlt(WinRender::instance().hdc, 0, 0, 800, 480, WinRender::instance().mdc, 0, 479, 800, -480, SRCCOPY);
    memset(WinRender::instance().content, 0, WinRender::instance().bmi.bmiHeader.biSizeImage);
}

int* RenderImpl::MouseClick() {
    WinRender::instance().GetWinMessage();
    return WinRender::instance().mouse_click_status;
}

int* RenderImpl::MouseMove() {
    return nullptr;
}


#endif



#endif