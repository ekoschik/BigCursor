
#include "stdafx.h"
#include "CursorsTest.h"
using namespace std;

const int cxButton = 120;
const int cyButton = 30;
const int buf = 5;
const int txtSize = 15;
int base = 0;

RECT rcTxtSystem = {};
RECT rcTxtCustom = {};
HFONT hfont = nullptr;
HFONT hfontBold = nullptr;

const COLORREF rgbButtonSystem = RGB(66, 170, 244);
const COLORREF rgbButtonCustom = RGB(232, 65, 244);
const COLORREF rgbButtonSelect = RGB(65, 244, 112);
const COLORREF rgbButtonHover = RGB(65, 205, 244);

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

    static HBRUSH hbrButtonSystem = CreateSolidBrush(rgbButtonSystem);
    static HBRUSH hbrButtonCustom = CreateSolidBrush(rgbButtonCustom);
    static HBRUSH hbrButtonSelect = CreateSolidBrush(rgbButtonSelect);
    static HBRUSH hbrButtonHover = CreateSolidBrush(rgbButtonHover);

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

void UpdateFonts()
{
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

    hfontBold = CreateFont((int)(1.5 * SCALE_FOR_DPI(txtSize)),
        0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, L"Consolas");
}