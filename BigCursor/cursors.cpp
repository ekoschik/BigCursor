
#include "stdafx.h"
#include "CursorsTest.h"
using namespace std;

extern BYTE XORmaskCursor[128];
extern BYTE ANDmaskCursor[128];

OPTION_BOX* CreateCustomCursorOption(int index)
{
    HCURSOR hcur = nullptr;
    wstring name;

    switch (index)
    {
    case 0:
        hcur = CreateCursor(
            ghinst, 19, 2, 32, 32, ANDmaskCursor, XORmaskCursor);
        name = L"CreateCursor32";
        break;
    case 1:
        hcur = LoadIcon(ghinst, MAKEINTRESOURCE(IDI_BIGCURSOR));
        name = L"LoadIcon";
        break;
    case 2:
        hcur = (HCURSOR)LoadImage(
            ghinst, MAKEINTRESOURCE(IDI_BIGCURSOR), IMAGE_ICON,
            600, 600, LR_DEFAULTCOLOR);
        name = L"LoadImage(600x600)";
        break;
    case 3:
        hcur = LoadCursor(ghinst, MAKEINTRESOURCE(IDC_DIMSTEST_32_48));
        name = L"DimsTest-32_48";
        break;
    case 4:
        hcur = LoadCursor(ghinst, MAKEINTRESOURCE(IDC_DIMSTEST_48_64));
        name = L"DimsTest-48_64";
        break;
    case 5:
        hcur = LoadCursor(ghinst, MAKEINTRESOURCE(IDC_DIMSTEST_64_128));
        name = L"DimsTest-64_128";
        break;
    case 6:
        hcur = LoadCursor(ghinst, MAKEINTRESOURCE(IDC_DIMSTEST_32));
        name = L"DimsTest-32";
        break;
    case 7:
        hcur = LoadCursor(ghinst, MAKEINTRESOURCE(IDC_DIMSTEST_64));
        name = L"DimsTest-64";
        break;
    case 8:
        hcur = LoadCursor(ghinst, MAKEINTRESOURCE(IDC_DIMSTEST_128));
        name = L"DimsTest-128";
        break;
    case 9:
        hcur = LoadCursor(ghinst, MAKEINTRESOURCE(IDC_DIMSTEST_256));
        name = L"DimsTest-256";
        break;
    case 10:
        hcur = LoadCursor(ghinst, MAKEINTRESOURCE(IDC_DIMSTEST_42_123_199_234));
        name = L"DimsTest-42_123_199_234";
        break;
    case 11:
        hcur = LoadCursor(ghinst, MAKEINTRESOURCE(IDC_CREATIONOFADAM));
        name = L"Creation of Adam (LoadCursor)";
        break;
    case 12:
        hcur = (HCURSOR)LoadImage(
            ghinst, MAKEINTRESOURCE(IDC_CREATIONOFADAM), IMAGE_CURSOR,
            600, 600, LR_DEFAULTCOLOR);
        name = L"Creation of Adam (LoadImage)";
        break;
    default:
        return nullptr;
    }

    if (hcur == nullptr)
    {
        FATEL_ERROR(L"CreateCustomCursorForIndex hcur == null!");
    }
    
    return new OPTION_BOX(hcur, name, false);
}

OPTION_BOX* CreateSysCursorOption(int i)
{
    HCURSOR hcur = nullptr;
    wstring name;

    switch (i)
    {
    case 0:
        name = L"IDC_ARROW";
        hcur = LoadCursor(nullptr, IDC_ARROW);
        break;
    case 1:
        name = L"IDC_HAND";
        hcur = LoadCursor(nullptr, IDC_HAND);
        break;
    case 2:
        name = L"IDC_CROSS";
        hcur = LoadCursor(nullptr, IDC_CROSS);
        break;
    case 3:
        name = L"IDC_HELP";
        hcur = LoadCursor(nullptr, IDC_HELP);
        break;
    case 4:
        name = L"IDC_IBEAM";
        hcur = LoadCursor(nullptr, IDC_IBEAM);
        break;
    case 5:
        name = L"IDC_WAIT";
        hcur = LoadCursor(nullptr, IDC_WAIT);
        break;
    case 6:
        name = L"IDC_NO";
        hcur = LoadCursor(nullptr, IDC_NO);
        break;
    case 7:
        name = L"IDC_APPSTARTING";
        hcur = LoadCursor(nullptr, IDC_APPSTARTING);
        break;
    case 8:
        name = L"IDC_SIZEALL";
        hcur = LoadCursor(nullptr, IDC_SIZEALL);
        break;
    case 9:
        name = L"IDC_SIZENESW";
        hcur = LoadCursor(nullptr, IDC_SIZENESW);
        break;
    case 10:
        name = L"IDC_SIZENS";
        hcur = LoadCursor(nullptr, IDC_SIZENS);
        break;
    case 11:
        name = L"IDC_SIZENWSE";
        hcur = LoadCursor(nullptr, IDC_SIZENWSE);
        break;
    case 12:
        name = L"IDC_SIZEWE";
        hcur = LoadCursor(nullptr, IDC_SIZEWE);
        break;
    case 13:
        name = L"IDC_UPARROW";
        hcur = LoadCursor(nullptr, IDC_UPARROW);
        break;
    default:
        return nullptr;
    }

    if (hcur == nullptr)
    {
        FATEL_ERROR(L"CreateSysCursorOption hcur == null!");
        return nullptr;
    }

    return new OPTION_BOX(hcur, name, true);
}

void InitCursors()
{
    // Create all of the system cursors
    int i = 0;
    OPTION_BOX* option = nullptr;
    while ((option = CreateSysCursorOption(i++)) != nullptr)
    {
        optionsList.push_back(option);
    }

    // Create all of the custom cursors
    i = 0;
    while ((option = CreateCustomCursorOption(i++)) != nullptr)
    {
        optionsList.push_back(option);
    }

    // Link all options together in a circular double linked list
    OPTION_BOX* ptrFirst = nullptr;
    OPTION_BOX* ptrPrev = nullptr;
    OPTION_BOX* ptr = nullptr;
    for (auto it = optionsList.begin(); it != optionsList.end(); ++it)
    {
        ptr = *it;

        if (ptrFirst == nullptr)
        {
            ptrFirst = ptrPrev = ptr;
        }
        else
        {
            ptrPrev->ptrNext = ptr;
            ptr->ptrPrev = ptrPrev;
            ptrPrev = ptr;
        }
    }

    ptrFirst->ptrPrev = ptr;
    ptr->ptrNext = ptrFirst;
}

BYTE ANDmaskCursor[] =
{
    0xFF, 0xFC, 0x3F, 0xFF,
    0xFF, 0xC0, 0x1F, 0xFF,
    0xFF, 0x00, 0x3F, 0xFF,
    0xFE, 0x00, 0xFF, 0xFF,
    0xF7, 0x01, 0xFF, 0xFF,
    0xF0, 0x03, 0xFF, 0xFF,
    0xF0, 0x03, 0xFF, 0xFF,
    0xE0, 0x07, 0xFF, 0xFF,
    0xC0, 0x07, 0xFF, 0xFF,
    0xC0, 0x0F, 0xFF, 0xFF,
    0x80, 0x0F, 0xFF, 0xFF,
    0x80, 0x0F, 0xFF, 0xFF,
    0x80, 0x07, 0xFF, 0xFF,
    0x00, 0x07, 0xFF, 0xFF,
    0x00, 0x03, 0xFF, 0xFF,
    0x00, 0x00, 0xFF, 0xFF,
    0x00, 0x00, 0x7F, 0xFF,
    0x00, 0x00, 0x1F, 0xFF,
    0x00, 0x00, 0x0F, 0xFF,
    0x80, 0x00, 0x0F, 0xFF,
    0x80, 0x00, 0x07, 0xFF,
    0x80, 0x00, 0x07, 0xFF,
    0xC0, 0x00, 0x07, 0xFF,
    0xC0, 0x00, 0x0F, 0xFF,
    0xE0, 0x00, 0x0F, 0xFF,
    0xF0, 0x00, 0x1F, 0xFF,
    0xF0, 0x00, 0x1F, 0xFF,
    0xF8, 0x00, 0x3F, 0xFF,
    0xFE, 0x00, 0x7F, 0xFF,
    0xFF, 0x00, 0xFF, 0xFF,
    0xFF, 0xC3, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF 
};

BYTE XORmaskCursor[] =
{
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x03, 0xC0, 0x00,
    0x00, 0x3F, 0x00, 0x00,
    0x00, 0xFE, 0x00, 0x00,
    0x0E, 0xFC, 0x00, 0x00,
    0x07, 0xF8, 0x00, 0x00,
    0x07, 0xF8, 0x00, 0x00,
    0x0F, 0xF0, 0x00, 0x00,
    0x1F, 0xF0, 0x00, 0x00,
    0x1F, 0xE0, 0x00, 0x00,
    0x3F, 0xE0, 0x00, 0x00,
    0x3F, 0xE0, 0x00, 0x00,
    0x3F, 0xF0, 0x00, 0x00,
    0x7F, 0xF0, 0x00, 0x00,
    0x7F, 0xF8, 0x00, 0x00,
    0x7F, 0xFC, 0x00, 0x00,
    0x7F, 0xFF, 0x00, 0x00,
    0x7F, 0xFF, 0x80, 0x00,
    0x7F, 0xFF, 0xE0, 0x00,
    0x3F, 0xFF, 0xE0, 0x00,
    0x3F, 0xC7, 0xF0, 0x00,
    0x3F, 0x83, 0xF0, 0x00,
    0x1F, 0x83, 0xF0, 0x00,
    0x1F, 0x83, 0xE0, 0x00,
    0x0F, 0xC7, 0xE0, 0x00,
    0x07, 0xFF, 0xC0, 0x00,
    0x07, 0xFF, 0xC0, 0x00,
    0x01, 0xFF, 0x80, 0x00,
    0x00, 0xFF, 0x00, 0x00,
    0x00, 0x3C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00 
};
