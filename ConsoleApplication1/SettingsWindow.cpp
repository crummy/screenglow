#include "stdafx.h"
#include "SettingsWindow.h"
#include "Hue.h"
#include "resource.h"
#include <CommCtrl.h>
#include <assert.h>

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
            testConnection(hDlg);
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
    string lightId = settings->getLightId();
    SetWindowText(lighthWnd, (LPCTSTR)lightId.c_str());

    HWND iphWnd = GetDlgItem(hWnd, IDC_IPADDRESS);
    string ipAddress = settings->getIPAddress();
    SetWindowText(iphWnd, (LPCTSTR)ipAddress.c_str());

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
    string oldIPAddress = settings->getIPAddress();
    string newIPAddress = GetWindowString(hDlg, IDC_IPADDRESS);
    settings->setIPAddress(newIPAddress);

    string oldLightID = settings->getLightId();
    string newLightID = GetWindowString(hDlg, IDC_LIGHTID);
    settings->setLightID(newLightID);
    if ((newIPAddress.compare(oldIPAddress) != 0) || (newLightID.compare(oldLightID) != 0)) {
        // If the IP address or light ID or username has changed, then we need to get a new connection to the Hub.
    }
    string brightnessMinimum = GetWindowString(hDlg, IDC_MINBRIGHTNESSTEXT);
    settings->setBrightnessMinimum(atoi(brightnessMinimum.c_str()));
    string captureInterval = GetWindowString(hDlg, IDC_CAPTURETEXT);
    settings->setCaptureIntervalMs(atoi(captureInterval.c_str()));
}

void SettingsWindow::testConnection(HWND hDlg) {
    const string IPAddress = GetWindowString(hDlg, IDC_IPADDRESS);
    const string lightID = GetWindowString(hDlg, IDC_LIGHTID);
    const string username = GetWindowString(hDlg, IDC_USERNAME);
    Hue *testHue = new Hue(IPAddress, lightID, username);
    connectionStatus result = testHue->testConnection();

    // set response text 
    LPCSTR resultText;
    switch (result) {
    case connectionOK: {
        // if connection was successful, set username in case a new one was created
        const string newUsername = testHue->getUsername();
        wstring newWUsername(newUsername.begin(), newUsername.end());
        LPCWSTR newLPUsername = newWUsername.c_str();
        SetWindowText(GetDlgItem(hDlg, IDC_USERNAME), newLPUsername);
        resultText = "Connection successful";
        break;
    }
    case connectionBadID:
        resultText = "FAILED: Bad light ID";
        break;
    case connectionBadIP:
        resultText = "FAILED: Bad hub IP";
        break;
    case connectionNeedsLink:
        resultText = "FAILED: Press link button and try again";
        break;
    case connectionUnknownError:
        resultText = "FAILED: Unknown error!";
        break;
    default:
        resultText = "Unrecognized error?!";
    }
    HWND testTextControl = GetDlgItem(hDlg, IDC_TESTTEXT);
    SetWindowTextA(testTextControl, resultText);

    delete testHue;
}

string SettingsWindow::GetWindowString(HWND hDlg, int item) {
    char buffer[256];
    HWND hWnd = GetDlgItem(hDlg, item);
    int textLen = GetWindowTextLength(hWnd);
    assert(256 >= textLen);
    GetWindowTextA(hWnd, buffer, textLen);
    return buffer;
}