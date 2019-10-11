// KeepAspect.cpp
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: MIT
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#define COLUMNS 3
#define ROWS 2

INT GetHeightFromWidth(INT cx)
{
    return cx * ROWS / COLUMNS;
}

INT GetWidthFromHeight(INT cy)
{
    return cy * COLUMNS / ROWS;
}

BOOL OnSizing(HWND hwnd, DWORD fwSide, LPRECT prc)
{
    RECT rc;
    DWORD style = GetWindowStyle(hwnd);
    DWORD exstyle = GetWindowExStyle(hwnd);
    BOOL bMenu = FALSE;

    SetRectEmpty(&rc);
    AdjustWindowRectEx(&rc, style, bMenu, exstyle);
    INT dx0 = rc.left, dy0 = rc.top;
    INT dx1 = rc.right, dy1 = rc.bottom;

    rc = *prc;
    rc.left -= dx0;
    rc.top -= dy0;
    rc.right -= dx1;
    rc.bottom -= dy1;

    int x = rc.left;
    int y = rc.top;
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;
    int cx2, cy2;
    bool f = float(cy) / cx > float(ROWS) / COLUMNS;

    switch (fwSide)
    {
    case WMSZ_TOP:
    case WMSZ_BOTTOM:
        cx2 = GetWidthFromHeight(cy);
        rc.left = x + cx / 2 - cx2 / 2;
        rc.right = x + cx / 2 + cx2 / 2;
        break;
    case WMSZ_LEFT:
    case WMSZ_RIGHT:
        cy2 = GetHeightFromWidth(cx);
        rc.top = y + cy / 2 - cy2 / 2;
        rc.bottom = y + cy / 2 + cy2 / 2;
        break;
    case WMSZ_TOPLEFT:
        if (f)
        {
            cx2 = GetWidthFromHeight(cy);
            rc.left = x + cx - cx2;
        }
        else
        {
            cy2 = GetHeightFromWidth(cx);
            rc.top = y + cy - cy2;
        }
        break;
    case WMSZ_TOPRIGHT:
        if (f)
        {
            cx2 = GetWidthFromHeight(cy);
            rc.right = x + cx2;
        }
        else
        {
            cy2 = GetHeightFromWidth(cx);
            rc.top = y + cy - cy2;
        }
        break;
    case WMSZ_BOTTOMLEFT:
        if (f)
        {
            cx2 = GetWidthFromHeight(cy);
            rc.left = x + cx - cx2;
        }
        else
        {
            cy2 = GetHeightFromWidth(cx);
            rc.bottom = y + cy2;
        }
        break;
    case WMSZ_BOTTOMRIGHT:
        if (f)
        {
            cx2 = GetWidthFromHeight(cy);
            rc.right = x + cx2;
        }
        else
        {
            cy2 = GetHeightFromWidth(cx);
            rc.bottom = y + cy2;
        }
        break;
    }

    AdjustWindowRectEx(&rc, style, bMenu, exstyle);
    *prc = rc;

    InvalidateRect(hwnd, NULL, TRUE);

    return TRUE;
}

BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    RECT rc;
    GetWindowRect(hwnd, &rc);
    OnSizing(hwnd, WMSZ_BOTTOMRIGHT, &rc);
    MoveWindow(hwnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

    return TRUE;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        EndDialog(hwnd, id);
        break;
    }
}

void OnPaint(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    INT cx = rc.right;
    INT cy = rc.bottom;

    PAINTSTRUCT ps;
    if (HDC hdc = BeginPaint(hwnd, &ps))
    {
        PatBlt(hdc, 0, 0, cx, cy, WHITENESS);
        SelectObject(hdc, GetStockObject(BLACK_PEN));

        for (int q = 1; q < ROWS; ++q)
        {
            for (int p = 1; p < COLUMNS; ++p)
            {
                int x = cx * p / COLUMNS;
                MoveToEx(hdc, x, 0, NULL);
                LineTo(hdc, x, cy);
            }
        }
        for (int p = 1; p < COLUMNS; ++p)
        {
            for (int q = 1; q < ROWS; ++q)
            {
                int y = cy * q / ROWS;
                MoveToEx(hdc, 0, y, NULL);
                LineTo(hdc, cx, y);
            }
        }

        TCHAR szText[64];
        wsprintf(szText, TEXT("%d : %d"), cx, cy);
        TextOut(hdc, 0, 0, szText, lstrlen(szText));

        EndPaint(hwnd, &ps);
    }
}

BOOL OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    return TRUE;
}

void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
    RECT rc;
    DWORD style = GetWindowStyle(hwnd);
    DWORD exstyle = GetWindowExStyle(hwnd);
    BOOL bMenu = FALSE;

    SetRectEmpty(&rc);
    rc.right = 256;
    rc.bottom = rc.right * ROWS / COLUMNS;
    AdjustWindowRectEx(&rc, style, bMenu, exstyle);

    lpMinMaxInfo->ptMinTrackSize.x = rc.right - rc.left;
    lpMinMaxInfo->ptMinTrackSize.y = rc.bottom - rc.top;
}

void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    InvalidateRect(hwnd, NULL, TRUE);
}

INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hwnd, WM_GETMINMAXINFO, OnGetMinMaxInfo);
        HANDLE_MSG(hwnd, WM_SIZE, OnSize);
        case WM_SIZING:
        {
            if (OnSizing(hwnd, (DWORD)wParam, (LPRECT)lParam))
            {
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
                return TRUE;
            }
            break;
        }
    }
    return 0;
}

int WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        int         nCmdShow)
{
    InitCommonControls();
    DialogBox(hInstance, MAKEINTRESOURCE(100), NULL, DialogProc);
    return 0;
}
