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
INT_PTR CALLBACK SettingsWindow::StaticAppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_INITDIALOG) {
        SetProp(hDlg, L"Settings Window", (HANDLE)lParam);
    }
    else if (uMsg == WM_NCDESTROY) {
        SetProp(hDlg, L"Settings Window", 0);
    }
    SettingsWindow *pThis = (SettingsWindow*)GetProp(hDlg, L"Settings Window");
    return pThis ? pThis->AppDlgProc(hDlg, uMsg, wParam, lParam) : FALSE;
}

INT_PTR CALLBACK SettingsWindow::AppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG: {
        populateUIFromSettings(hDlg);
        break;
    }
    case WM_COMMAND:
        switch (wParam) {
        case IDC_OK:
            populateSettingsFromUI(hDlg);
            DestroyWindow(hDlg);
            return 0;
        case IDC_CANCEL:
            DestroyWindow(hDlg);
            break;
        case IDC_TESTBUTTON:
            populateSettingsFromUI(hDlg);
            break;
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
    return DefWindowProc(hDlg, uMsg, wParam, lParam);
}

// Populates UI elements (text boxes, sliders etc) with data extracted from the Settings object we have.
void SettingsWindow::populateUIFromSettings(HWND hWnd) {
    HWND lighthWnd = GetDlgItem(hWnd, IDC_LIGHTID);
    TCHAR* lightId = settings->getLightId();
    SetWindowText(lighthWnd, lightId);

    HWND iphWnd = GetDlgItem(hWnd, IDC_IPADDRESS);
    TCHAR* ipAddress = settings->getIPAddress();
    SetWindowText(iphWnd, ipAddress);

    HWND brightnessSliderhWnd = GetDlgItem(hWnd, IDC_MINBRIGHTNESSSLIDER);
    SendMessage(brightnessSliderhWnd, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 255));
    int brightnessMinimum = settings->getBrightnessMinimum();
    SendMessage(brightnessSliderhWnd, TBM_SETPOS, (WPARAM)brightnessMinimum, (LPARAM)MAKELONG(brightnessMinimum, 0));

    HWND brightnessTexthWnd = GetDlgItem(hWnd, IDC_MINBRIGHTNESSTEXT);
    TCHAR brightnessMinimumString[16];
    _stprintf_s(brightnessMinimumString, 16, _T("%d"), brightnessMinimum);
    SetWindowText(brightnessTexthWnd, brightnessMinimumString);
    
    HWND captureSliderhWnd = GetDlgItem(hWnd, IDC_CAPTURESLIDER);
    SendMessage(captureSliderhWnd, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(30, 3000));
    int captureInterval = settings->getCaptureIntervalMs();
    SendMessage(captureSliderhWnd, TBM_SETPOS, (WPARAM)captureInterval, (LPARAM)MAKELONG(captureInterval, 0));

    HWND captureTexthWnd = GetDlgItem(hWnd, IDC_CAPTURETEXT);
    TCHAR captureIntervalString[16];
    _stprintf_s(captureIntervalString, _T("%d"), captureInterval);
    SetWindowText(captureTexthWnd, captureIntervalString);
}

// Takes values from the settings window and inserts them into the settings object.
// Prefers values from text boxes over sliders, that way users can set values outside of the slider
// ranges if they wish.
void SettingsWindow::populateSettingsFromUI(HWND hDlg) {
    TCHAR IPAddress[256];
    GetWindowText(GetDlgItem(hDlg, IDC_IPADDRESS), &IPAddress[0], 256);
    // TODO: Set IP address in settings
    TCHAR* oldIPAddress = settings->getIPAddress();
    TCHAR lightID[256];
    GetWindowText(GetDlgItem(hDlg, IDC_LIGHTID), &lightID[0], 256);
    // TODO: Set lightID in settings
    TCHAR* oldLightID = settings->getLightId();
    if ((_tcscmp(IPAddress, oldIPAddress) != 0) || (_tcscmp(lightID, oldLightID) != 0)) {
        // If the IP address or light ID have changed, then we need to get a new connection to the Hub.
    }
    TCHAR brightnessMinimum[256];
    GetWindowText(GetDlgItem(hDlg, IDC_MINBRIGHTNESSTEXT), &brightnessMinimum[0], 256);
    // TODO: Set brightnessMinimum in settings
    TCHAR captureInterval[256];
    GetWindowText(GetDlgItem(hDlg, IDC_CAPTURETEXT), &captureInterval[0], 256);
    // TODO: Set captureInterval in Settings
}