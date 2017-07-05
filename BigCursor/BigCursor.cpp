
#include "stdafx.h"
#include "BigCursor.h"
#include <cmath>
using namespace std;

HINSTANCE ghinst;
HWND ghwnd;
std::list<OPTION_BOX*> optionsList;

const SIZE szStart = { 1000, 800 };
const POINT ptStart = { 50, 50 };

OPTION_BOX* HoverOption = nullptr;
OPTION_BOX* SelectedOption = nullptr;

int dpi = 96;
HFONT hfont = nullptr;
HFONT hfontBold = nullptr;
#define SCALE_FOR_DPI(val) MulDiv(val, dpi, 96)

const int cxButton = 120;
const int cyButton = 30;
const int buf = 5;
const int txtSize = 15;
RECT rcTxtSystem = {};
RECT rcTxtCustom = {};

int base = 0;
bool waiting = false;
bool inDrag = false;
POINT ptDragStart = {};
POINT ptDrag = {};
const int wheelDragSpeed = 15;

COLORREF rgbButtonSystem = RGB(66, 170, 244);
COLORREF rgbButtonCustom = RGB(232, 65, 244);
COLORREF rgbButtonSelect = RGB(65, 244, 112);
COLORREF rgbButtonHover = RGB(65, 205, 244);
HBRUSH hbrButtonSystem;
HBRUSH hbrButtonCustom;
HBRUSH hbrButtonSelect;
HBRUSH hbrButtonHover;

void SetLayoutRects()
{
    const int cxSystem = SCALE_FOR_DPI(cxButton);
    const int cxCustom = 2 * cxSystem + buf;
    const int cy = SCALE_FOR_DPI(cyButton);

    // set 'system:' text pos in top right (+buf, +base)
    RECT rc = { buf, buf + base, buf + cxSystem, buf + cy + base };
    rcTxtSystem = rc;

    // move down
    rc.top = rc.bottom + buf;
    rc.bottom = rc.top + cy;

    OPTION_BOX* option = optionsList.front();
    while (option != nullptr && option->system)
    {
        CopyRect(&option->rcPos, &rc);

        option = option->ptrNext;
        if (option != nullptr)
        {
            // move right
            rc.left = rc.right + buf;
            rc.right = rc.left + cxSystem;

            CopyRect(&option->rcPos, &rc);
        }

        // reset to right edge and move down
        rc.left = buf;
        rc.right = rc.left + cxSystem;
        rc.top = rc.bottom + buf;
        rc.bottom = rc.top + cy;

        option = option->ptrNext;
    }

    // set 'custom' text pos
    rcTxtCustom = rc;

    rc.right = rc.left + cxCustom;
    while (option != nullptr && option != optionsList.front())
    {
        rc.top = rc.bottom + buf;
        rc.bottom = rc.top + cy;
        CopyRect(&option->rcPos, &rc);
        option = option->ptrNext;
    }

    InvalidateRect(ghwnd, nullptr, true);
}

void Draw(HDC hdc)
{
    SetBkMode(hdc, TRANSPARENT);

    // Draw the heading text
    SelectFont(hdc, hfontBold);
    static wstring strSystem = L"system:";
    static wstring strCustom = L"custom:";
    DrawText(hdc, strSystem.c_str(), strSystem.length(), &rcTxtSystem, DT_VCENTER | DT_SINGLELINE);
    DrawText(hdc, strCustom.c_str(), strCustom.length(), &rcTxtCustom, DT_VCENTER | DT_SINGLELINE);

    // Draw each button
    for (auto it = optionsList.begin(); it != optionsList.end(); ++it)
    {
        OPTION_BOX* option = *it;

        SelectObject(hdc, 
            ((option == SelectedOption) ? hbrButtonSelect :
             ((option == HoverOption) ? hbrButtonHover :
             (option->system ? hbrButtonSystem : hbrButtonCustom))));
        
        Rectangle(hdc,
            option->rcPos.left,
            option->rcPos.top,
            option->rcPos.right,
            option->rcPos.bottom);

        SelectFont(hdc, hfont);
        DrawText(hdc, option->name.c_str(), option->name.length(),
            &option->rcPos, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // TODO: to the right of the buttons, maybe print current cursor name,
    // and even all the info I can read out from it (GetCursorInfo...)

}

void SetCurrentCursor()
{
    if (SelectedOption == nullptr)
    {
        FATEL_ERROR(L"Why is SelectedOption null?");
        return;
    }

    SetCursor((HoverOption != nullptr) ?
        HoverOption->hcur : SelectedOption->hcur);
}

bool TestMouseMessageForDrag(POINT pt, UINT message)
{
    if (message == WM_LBUTTONDBLCLK)
    {
        base = 0;
        SetLayoutRects();
        InvalidateRect(ghwnd, nullptr, true);
        inDrag = waiting = false;
    }

    if (!waiting && message == WM_LBUTTONDOWN)
    {
        ptDragStart = ptDrag = pt;
        waiting = true;
        return true;
    }

    if (waiting && message == WM_MOUSEMOVE)
    {
        if (abs(pt.y - ptDragStart.y) > 5) // don't drag on a click
        {
            inDrag = true;
        }
    }

    if (inDrag && message == WM_MOUSEMOVE)
    {
        base += pt.y - ptDrag.y;
        ptDrag = pt;
        SetLayoutRects();
        InvalidateRect(ghwnd, nullptr, true);
        return true;
    }

    if (message == WM_LBUTTONUP)
    {
        waiting = false;
        if (inDrag)
        {
            inDrag = false;
            return true;
        }
    }

    inDrag = false;
    return false;
}

void HandleMoveClick(int x, int y, UINT message)
{
    POINT pt = { x, y };

    if (TestMouseMessageForDrag(pt, message))
    {
        return;
    }

    for (auto it = optionsList.begin(); it != optionsList.end(); ++it)
    {
        OPTION_BOX* option = *it;
        bool repaint = false;

        if (PtInRect(&option->rcPos, pt))
        {
            if (message == WM_MOUSEMOVE)
            {
                repaint = (HoverOption != option);
                HoverOption = option;
            }
            else if (message == WM_LBUTTONUP)
            {
                repaint = (SelectedOption != option);
                SelectedOption = option;
            }

            if (repaint)
            {
                InvalidateRect(ghwnd, NULL, TRUE);
            }

            return;
        }
    }

    if ((message == WM_MOUSEMOVE) && HoverOption != nullptr)
    {
        HoverOption = nullptr;
        InvalidateRect(ghwnd, NULL, TRUE);
    }
}

void HandleMouseWheel(bool down, bool ctrl)
{
    if (ctrl)
    {
        base += (down ? -1 : 1) * wheelDragSpeed;
        SetLayoutRects();
        InvalidateRect(ghwnd, nullptr, true);
        return;
    }

    if (SelectedOption != nullptr)
    {
        SelectedOption = (down ? 
            SelectedOption ->ptrNext: SelectedOption->ptrPrev);

        SetLayoutRects();
        SetCurrentCursor();
        InvalidateRect(ghwnd, NULL, TRUE);
    }
}

void UpdateWindowDpi(int _dpi)
{
    dpi = _dpi;

    WCHAR buf[100] = {};
    swprintf(buf, 100, L"Window DPI: %i", dpi);
    SetWindowText(ghwnd, (LPCWSTR)&buf);

    if (hfont != nullptr)
    {
        DeleteFont(hfont);
    }

    if (hfontBold != nullptr)
    {
        DeleteFont(hfont);
    }

    hfont = CreateFont(SCALE_FOR_DPI(txtSize),
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Consolas");

    hfontBold = CreateFont(1.5 * SCALE_FOR_DPI(txtSize),
        0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, L"Consolas");
}

void HandleDpiChanged(int _dpi, PRECT prc)
{
    UpdateWindowDpi(_dpi);

    SetWindowPos(ghwnd,
        NULL,
        prc->left,
        prc->top,
        prc->right - prc->left,
        prc->bottom - prc->top,
        SWP_NOZORDER | SWP_NOACTIVATE);
}

void InitWindow()
{
    UpdateWindowDpi(GetDpiForWindow(ghwnd));

    SetWindowPos(ghwnd, nullptr, 0, 0,
        SCALE_FOR_DPI(szStart.cx), SCALE_FOR_DPI(szStart.cy),
        SWP_SHOWWINDOW | SWP_NOMOVE);

    if (optionsList.empty())
    {
        FATEL_ERROR(L"InitCursors left options list empty.");
        return;
    }

    SelectedOption = optionsList.front();

    SetLayoutRects();

    hbrButtonSystem = CreateSolidBrush(rgbButtonSystem);
    hbrButtonCustom = CreateSolidBrush(rgbButtonCustom);
    hbrButtonSelect = CreateSolidBrush(rgbButtonSelect);
    hbrButtonHover = CreateSolidBrush(rgbButtonHover);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        ghwnd = hwnd;
        InitWindow();
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Draw(hdc);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_SETCURSOR:
    {
        if (LOWORD(lParam) == HTCLIENT)
        {
            SetCurrentCursor();
            return 1;
        }
        break;
    }

    case WM_MOUSEWHEEL:
    {
        static int delta = 0;
        delta += GET_WHEEL_DELTA_WPARAM(wParam);
        if (delta >= 120 || delta <= -120)
        {
            HandleMouseWheel(delta < 0, LOWORD(wParam) & MK_CONTROL);
            delta = 0;
            return 1;
        }
        break;
    }

    case WM_WINDOWPOSCHANGED:
    {
        WINDOWPOS* pwp = (WINDOWPOS*)lParam;
        static WINDOWPOS wp = {};
        if ((pwp->cx != wp.cx) || (pwp->cy != wp.cy))
        {
            SetLayoutRects();
            RtlCopyMemory(&wp, pwp, sizeof(WINDOWPOS));
        }
        break;
    }

    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        HandleMoveClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), message);
        break;

    case WM_DPICHANGED:
        HandleDpiChanged(HIWORD(wParam), (RECT*)lParam);
        break;

    case WM_DESTROY:
        for (auto it = optionsList.begin(); it != optionsList.end(); ++it)
        {
            delete *it;
        }
        optionsList.clear();
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    ghinst = hInstance;
    LPCWSTR szWndClass = L"class";
    LPCWSTR szWndTitle = L"Cursors Test App";

    InitCursors();

    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = ghinst;
    wcex.hIcon = LoadIcon(ghinst, MAKEINTRESOURCE(IDI_BIGCURSOR));
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWndClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!RegisterClassEx(&wcex))
    {
        return 1;
    }

    HWND hwnd = CreateWindow(
        szWndClass, szWndTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, ghinst, nullptr);

    if (hwnd == nullptr)
    {
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
