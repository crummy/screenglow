#pragma once

#include <Windows.h>

class AboutWindow
{
public:
    AboutWindow();
    ~AboutWindow();
    void show(HINSTANCE hInstance);
private:
    static INT_PTR CALLBACK StaticAppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR CALLBACK AppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
