#pragma once

#include <Windows.h>

// A dumb window that just appears, shows static content, and closes when you quit.

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

