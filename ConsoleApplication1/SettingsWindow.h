#pragma once

#include "Settings.h"
#include <Windows.h>

typedef void(*SettingsWindowCallback)(void);

class SettingsWindow
{
public:
    SettingsWindow(Settings *settings, SettingsWindowCallback reconnectHub);
    ~SettingsWindow();
    void show(HINSTANCE hInstance);
private:
    Settings *settings;
    static INT_PTR CALLBACK StaticAppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR CALLBACK AppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void populateUIFromSettings(HWND hDlg);
    void populateSettingsFromUI(HWND hDlg);
    void testConnection(HWND hDlg);
    string GetWindowString(HWND hDlg, int item);
    void SetWindowString(HWND hDlg, int item, string str);
    void SetWindowString(HWND hDlg, int item, int i);
    SettingsWindowCallback reconnectHub;
};

