#pragma once

#include "Settings.h"
#include <Windows.h>

class SettingsWindow
{
public:
    SettingsWindow(Settings *settings);
    ~SettingsWindow();
    void show(HINSTANCE hInstance);
private:
    Settings *settings;
    static BOOL CALLBACK StaticAppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL CALLBACK AppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

