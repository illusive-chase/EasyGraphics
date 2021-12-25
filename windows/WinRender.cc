#if defined(_WIN32) || defined(WIN32) || defined(WIN64)

#include "WinRender.hh"
#include "resource.h"
#include "framework.h"
#include <Windows.h>
#include <cstdio>

#if 0
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
                          CW_USEDEFAULT, 0, 900, 700, nullptr, nullptr, hInstance, nullptr);

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