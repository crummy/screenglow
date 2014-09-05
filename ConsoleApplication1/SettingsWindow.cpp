#include "stdafx.h"
#include "SettingsWindow.h"
#include "resource.h"
#include <CommCtrl.h>

SettingsWindow::SettingsWindow(Settings *settings)
{
    this->settings = settings;
}


SettingsWindow::~SettingsWindow()
{
}

void SettingsWindow::show(HINSTANCE hInstance) {
    DialogBoxParam(hInstance, (LPCTSTR)IDD_SETTINGS, 0, SettingsWindow::StaticAppDlgProc, (LPARAM)this);
}

// A hack to allow putting AppDlgProc in a class.
// http://stackoverflow.com/questions/25678892/calling-a-win32-api-and-giving-a-callback-to-a-class-function
BOOL CALLBACK SettingsWindow::StaticAppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_INITDIALOG) {
        SetProp(hDlg, L"Settings Window", (HANDLE)lParam);
    }
    else if (uMsg == WM_NCDESTROY) {
        SetProp(hDlg, L"Settings Window", 0);
    }
    SettingsWindow *pThis = (SettingsWindow*)GetProp(hDlg, L"Settings Window");
    return pThis ? pThis->AppDlgProc(hDlg, uMsg, wParam, lParam) : FALSE;
}

BOOL CALLBACK SettingsWindow::AppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG: {
                            HWND lighthWnd = GetDlgItem(hDlg, IDC_LIGHTID);
                            string lightId = settings->getLightId();
                            wstring wLightId = wstring(lightId.begin(), lightId.end());
                            SetWindowText(lighthWnd, wLightId.c_str());

                            HWND iphWnd = GetDlgItem(hDlg, IDC_IPADDRESS);
                            string ipAddress = settings->getIPAddress();
                            wstring wIpAddress = wstring(ipAddress.begin(), ipAddress.end());
                            SetWindowText(iphWnd, wIpAddress.c_str());

                            HWND brightnessSliderhWnd = GetDlgItem(hDlg, IDC_MINBRIGHTNESSSLIDER);
                            SendMessage(brightnessSliderhWnd, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 255));
                            int brightnessMinimum = (int)settings->getBrightnessMinimum();
                            SendMessage(brightnessSliderhWnd, TBM_SETPOS, (WPARAM)brightnessMinimum, (LPARAM)MAKELONG(brightnessMinimum, 0));

                            HWND brightnessTexthWnd = GetDlgItem(hDlg, IDC_MINBRIGHTNESSTEXT);
                            string brightnessMinimumString = to_string(brightnessMinimum);
                            wstring wBrightnessMinimumString = wstring(brightnessMinimumString.begin(), brightnessMinimumString.end());
                            SetWindowText(brightnessTexthWnd, wBrightnessMinimumString.c_str());

                            HWND captureSliderhWnd = GetDlgItem(hDlg, IDC_CAPTURESLIDER);
                            SendMessage(captureSliderhWnd, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(30, 3000));
                            int captureInterval = (int)settings->getCaptureIntervalMs();
                            SendMessage(captureSliderhWnd, TBM_SETPOS, (WPARAM)captureInterval, (LPARAM)MAKELONG(captureInterval, 0));

                            HWND captureTexthWnd = GetDlgItem(hDlg, IDC_CAPTURETEXT);
                            string captureIntervalString = to_string(captureInterval);
                            wstring wCaptureIntervalString = wstring(captureIntervalString.begin(), captureIntervalString.end());
                            SetWindowText(captureTexthWnd, wCaptureIntervalString.c_str());

                            return 1;
                            break;
    }
    case WM_COMMAND:
        switch (wParam) {
        case IDC_OK:
            return 0;
        case IDC_CANCEL:
            EndDialog(hDlg, 0);
            break;
        case IDC_TESTBUTTON: {
                                 HWND hWnd = GetDlgItem(hDlg, IDC_TESTTEXT);
                                 SetWindowText(hWnd, _T("test clicked"));
                                 break;
        }
        case EN_CHANGE:
            HWND hWnd = (HWND)lParam;
            if (hWnd == GetDlgItem(hDlg, IDC_MINBRIGHTNESSTEXT)) {
                string newBrightnessMinimumString;
                newBrightnessMinimumString.resize(GetWindowTextLength(hWnd) + 1, '\0');
                GetWindowText(hWnd, LPWSTR(newBrightnessMinimumString.c_str()), 8);
                int newBrightnessMinimum = atoi(newBrightnessMinimumString.c_str());
                HWND brightnessSliderhWnd = GetDlgItem(hDlg, IDC_MINBRIGHTNESSSLIDER);
                SendMessage(brightnessSliderhWnd, TBM_SETPOS, (WPARAM)newBrightnessMinimum, (LPARAM)MAKELONG(newBrightnessMinimum, 0));
            }
            break;
        }
    case WM_HSCROLL:
        switch (LOWORD(wParam)) {
        case TB_THUMBTRACK:
        case TB_ENDTRACK:
            HWND sliderhWnd = (HWND)lParam;
            if (sliderhWnd == GetDlgItem(hDlg, IDC_MINBRIGHTNESSSLIDER)) {
                int newBrightnessMinimum = SendMessage(sliderhWnd, TBM_GETPOS, 0, 0);
                string newBrightnessMinimumString = to_string(newBrightnessMinimum);
                wstring wNewBrightnessMinimumString = wstring(newBrightnessMinimumString.begin(), newBrightnessMinimumString.end());
                HWND texthWnd = GetDlgItem(hDlg, IDC_MINBRIGHTNESSTEXT);
                SetWindowText(texthWnd, wNewBrightnessMinimumString.c_str());
            }
            else if (sliderhWnd == GetDlgItem(hDlg, IDC_CAPTURESLIDER)) {
                int newCaptureInterval = SendMessage(sliderhWnd, TBM_GETPOS, 0, 0);
                string newCaptureIntervalString = to_string(newCaptureInterval);
                wstring wNewCaptureIntervalString = wstring(newCaptureIntervalString.begin(), newCaptureIntervalString.end());
                HWND texthWnd = GetDlgItem(hDlg, IDC_CAPTURETEXT);
                SetWindowText(texthWnd, wNewCaptureIntervalString.c_str());
            }
            break;
        }
    }
    return 0;
}