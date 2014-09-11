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
    static INT_PTR CALLBACK StaticAppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR CALLBACK AppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void populateUIFromSettings(HWND hDlg);
    void populateSettingsFromUI(HWND hDlg);
    void testConnection(HWND hDlg);
};

