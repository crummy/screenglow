#pragma once

#include <Windows.h>

typedef void(*TaskbarCallback)(void);

class TaskbarIcon
{
public:
    TaskbarIcon(TaskbarCallback settingsCallback, TaskbarCallback aboutCallback, TaskbarCallback quitCallback);
    ~TaskbarIcon();
    void show(HINSTANCE hInstance);  
private:
    HINSTANCE hInstance;
    void ShowContextMenu(HWND hWnd);
    BOOL OnInitDialog(HWND hWnd);
    static INT_PTR CALLBACK StaticAppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR CALLBACK AppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    NOTIFYICONDATA niData;
    TaskbarCallback openSettingsWindow;
    TaskbarCallback openAboutWindow;
    TaskbarCallback quitApp;
};

