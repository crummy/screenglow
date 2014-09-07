#include "stdafx.h"
#include "TaskbarIcon.h"

#include "resource.h"

#define WM_RIGHTCLICKMENUMSG (WM_USER + 1)


TaskbarIcon::TaskbarIcon(TaskbarCallback settingsCallback, TaskbarCallback aboutCallback, TaskbarCallback quitCallback)
{
    openSettingsWindow = settingsCallback;
    openAboutWindow = aboutCallback;
    quitApp = quitCallback;
}


TaskbarIcon::~TaskbarIcon()
{
}

// thank you http://www.codeproject.com/Articles/4768/Basic-use-of-Shell-NotifyIcon-in-Win
void TaskbarIcon::show(HINSTANCE hInstance) {
    this->hInstance = hInstance;

    HWND hWnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_EMPTY), NULL, TaskbarIcon::StaticAppDlgProc, (LPARAM)this);

    ZeroMemory(&niData, sizeof(NOTIFYICONDATA));
    niData.cbSize = sizeof(NOTIFYICONDATA);
    niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    niData.hIcon = (HICON)LoadImage(hInstance,
        MAKEINTRESOURCE(IDI_ICON),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);
    niData.hWnd = hWnd;
    niData.uCallbackMessage = WM_RIGHTCLICKMENUMSG;
    lstrcpyn(niData.szTip, _T("ScreenGlow is now running"), sizeof(niData.szTip) / sizeof(TCHAR));
    Shell_NotifyIcon(NIM_ADD, &niData);
    if (niData.hIcon && DestroyIcon(niData.hIcon)) {
        niData.hIcon = NULL;
    }
}

void TaskbarIcon::ShowContextMenu(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);
    //HMENU hMenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_TASKBARMENU));
    HMENU hMenu = CreatePopupMenu();
    if (hMenu) {
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, ID_SETTINGS, _T("Settings"));
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, ID_ABOUT, _T("About"));
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, ID_EXIT, _T("Exit"));
        //SetForegroundWindow(hWnd);
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
        DestroyMenu(hMenu);
    }
}

BOOL TaskbarIcon::OnInitDialog(HWND hWnd) {
    HMENU hMenu = GetSystemMenu(hWnd, FALSE);
    if (hMenu) {
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenu, MF_STRING, ID_SETTINGS, _T("Settings"));
    }
    HICON hIcon = (HICON)LoadImage(hInstance,
        MAKEINTRESOURCE(IDI_ICON),
        IMAGE_ICON,
        0,
        0,
        LR_SHARED | LR_DEFAULTSIZE);
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    return TRUE;
}

INT_PTR CALLBACK TaskbarIcon::StaticAppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_INITDIALOG) {
        SetProp(hDlg, L"Taskbar Icon", (HANDLE)lParam);
    }
    else if (uMsg == WM_NCDESTROY) {
        SetProp(hDlg, L"Taskbar Icon", 0);
    }
    TaskbarIcon *pThis = (TaskbarIcon*)GetProp(hDlg, L"Taskbar Icon");
    return pThis ? pThis->AppDlgProc(hDlg, uMsg, wParam, lParam) : FALSE;
}

INT_PTR CALLBACK TaskbarIcon::AppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int wmId;
    int wmEvent;

    switch (uMsg) {
    case WM_RIGHTCLICKMENUMSG:
        switch (lParam) {
        case WM_LBUTTONDBLCLK:
            // turn light on/off
            break;
        case WM_RBUTTONDOWN:
        case WM_CONTEXTMENU:
            ShowContextMenu(hDlg);
        }
        break;
    case WM_SYSCOMMAND:
        // show about box?
        break;
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        switch (wmId) {
        case ID_ABOUT:
            (*openAboutWindow)();
            break;
        case ID_SETTINGS:
            (*openSettingsWindow)();
            break;
        case ID_EXIT:
            (*quitApp)();
            DestroyWindow(hDlg);
            break;
        }
    case WM_INITDIALOG:
        //return OnInitDialog(hDlg); // TODO: why is this not needed?
        break;
    case WM_CLOSE:
        DestroyWindow(hDlg);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        Shell_NotifyIcon(NIM_DELETE, &niData);
        break;
    }
    return DefWindowProc(hDlg, uMsg, wParam, lParam);
}