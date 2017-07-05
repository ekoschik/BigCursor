#pragma once

#include "resource.h"
#include "windowsx.h"
#include <string>
#include <list>

extern HINSTANCE ghinst;
extern HWND ghwnd;

#define FATEL_ERROR(msg) \
    MessageBox(ghwnd, msg, L"ERROR", MB_OK); \
    PostQuitMessage(1);

class OPTION_BOX
{
public:
    OPTION_BOX(HCURSOR _hcur, std::wstring _name, bool _system) :
        hcur(_hcur), name(_name), system(_system) {}

    const HCURSOR hcur = nullptr;
    const std::wstring name;
    const bool system = false;
    RECT rcPos = {};

    OPTION_BOX* ptrNext = nullptr;
    OPTION_BOX* ptrPrev = nullptr;
};

extern std::list<OPTION_BOX*> optionsList;
void InitCursors();

__inline UINT GetDpiForWindow(HWND hwnd)
{
    typedef UINT(WINAPI *fnGetDpiForWindow)(HWND);
    static fnGetDpiForWindow pfn = nullptr;
    if (!pfn) {
        static HMODULE hModUser32 = GetModuleHandle(_T("user32.dll"));
        pfn = (fnGetDpiForWindow)GetProcAddress(hModUser32, "GetDpiForWindow");
        if (!pfn) {
            return GetDeviceCaps(GetDC(NULL), LOGPIXELSX);
        }
    }
    return pfn(hwnd);
}