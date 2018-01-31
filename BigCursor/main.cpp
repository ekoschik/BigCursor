
#include "stdafx.h"
#include "CursorsTest.h"
using namespace std;

HINSTANCE ghinst;
HWND ghwnd;
int dpi = 96;

std::list<OPTION_BOX*> optionsList;
OPTION_BOX* HoverOption = nullptr;
OPTION_BOX* SelectedOption = nullptr;

const SIZE szStart = { 1000, 800 };
const int cxdrag = 5; //escape distance to disambiguate drags and clicks
const int wheelDragSpeed = 15; // pixels to scroll per wheel increment

void SetCurrentCursor()
{
    if (SelectedOption != nullptr)
    {
        SetCursor((HoverOption != nullptr) ?
            HoverOption->hcur : SelectedOption->hcur);
    }
}

bool TestMouseMessageForDrag(POINT pt, UINT message)
{
    static bool waiting = false;
    static bool inDrag = false;
    static POINT ptDragStart = {};
    static POINT ptDrag = {};

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

    if (message == WM_MOUSEMOVE)
    {
        if (waiting && (abs(pt.y - ptDragStart.y) > SCALE_FOR_DPI(cxdrag)))
        {
            inDrag = true;
        }

        if (inDrag)
        {
            base += pt.y - ptDrag.y;
            ptDrag = pt;
            SetLayoutRects();
            InvalidateRect(ghwnd, nullptr, true);
            return true;
        }
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

void HitTestMouseMessage(POINT pt, UINT message)
{
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

void HandleMoveClick(int x, int y, UINT message)
{
    POINT pt = { x, y };
    if (!TestMouseMessageForDrag(pt, message))
    {
        HitTestMouseMessage(pt, message);
    }
}

void HandleMouseWheel(bool down, bool ctrl)
{
    if (ctrl)
    {
        base += (down ? -1 : 1) * SCALE_FOR_DPI(wheelDragSpeed);
        SetLayoutRects();
        InvalidateRect(ghwnd, nullptr, true);
    }
    else if (SelectedOption != nullptr)
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

    UpdateFonts();
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
            HandleMouseWheel(delta < 0, !!(LOWORD(wParam) & MK_CONTROL));
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
            RtlCopyMemory(&wp, pwp, sizeof(WINDOWPOS));

            // Don't re-layout unless the size changed
            SetLayoutRects();
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

HWND CreateMainWindow()
{
    LPCWSTR szWndClass = L"class";
    LPCWSTR szWndTitle = L"Cursors Test App";

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
        return nullptr;
    }

    return CreateWindow(
        szWndClass, szWndTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, ghinst, nullptr);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
    ghinst = hInstance;

    InitCursors();

    if (CreateMainWindow() == nullptr)
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
