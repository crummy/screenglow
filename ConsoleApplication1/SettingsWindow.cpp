#include "stdafx.h"
#include "SettingsWindow.h"
#include "Hue.h"
#include "resource.h"
#include <CommCtrl.h>
#include <assert.h>

SettingsWindow::SettingsWindow(Settings *settings, SettingsWindowCallback reconnectHub)
{
    this->settings = settings;
    this->reconnectHub = reconnectHub;
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
    int wmEvent, wmId;
    switch (uMsg) {
    case WM_INITDIALOG: {
        populateUIFromSettings(hDlg);
        break;
    }
    case WM_COMMAND:
        wmEvent = HIWORD(wParam);
        wmId = LOWORD(wParam);
        switch (wmEvent) {
        case EN_KILLFOCUS:
            HWND hWnd = (HWND)lParam;
            if (hWnd == GetDlgItem(hDlg, IDC_MINBRIGHTNESSTEXT)) {
                setSliderValueFromText(hDlg, IDC_MINBRIGHTNESSSLIDER, IDC_MINBRIGHTNESSTEXT);
            }
            else if (hWnd == GetDlgItem(hDlg, IDC_CAPTURETEXT)) {
                setSliderValueFromText(hDlg, IDC_CAPTURESLIDER, IDC_CAPTURETEXT);
            }
            else if (hWnd == GetDlgItem(hDlg, IDC_MAXBRIGHTNESSSLIDER)) {
                setSliderValueFromText(hDlg, IDC_MAXBRIGHTNESSSLIDER, IDC_MAXBRIGHTNESSTEXT);
            }
            else if (hWnd == GetDlgItem(hDlg, IDC_BUCKETSLIDER)) {
                setSliderValueFromText(hDlg, IDC_BUCKETSLIDER, IDC_BUCKETTEXT);
            }
            break;
        }
        switch (wmId) {
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
        }
    case WM_HSCROLL:
        switch (LOWORD(wParam)) {
        case TB_THUMBTRACK:
        case TB_ENDTRACK:
            HWND sliderhWnd = (HWND)lParam;
            if (sliderhWnd == GetDlgItem(hDlg, IDC_MINBRIGHTNESSSLIDER)) {
                setTextFromSliderValue(hDlg, IDC_MINBRIGHTNESSTEXT, IDC_MINBRIGHTNESSSLIDER);
                int newMinimum = GetWindowInt(hDlg, IDC_MINBRIGHTNESSTEXT);
                int maximum = GetWindowInt(hDlg, IDC_MAXBRIGHTNESSTEXT);
                if (maximum < newMinimum) {
                    HWND maxSliderhWnd = GetDlgItem(hDlg, IDC_MAXBRIGHTNESSSLIDER);
                    SendMessage(maxSliderhWnd, TBM_SETPOS, (WPARAM)newMinimum, (LPARAM)MAKELONG(newMinimum, 0));
                    SetWindowString(hDlg, IDC_MAXBRIGHTNESSTEXT, newMinimum);
                }
            }
            else if (sliderhWnd == GetDlgItem(hDlg, IDC_CAPTURESLIDER)) {
                setTextFromSliderValue(hDlg, IDC_CAPTURETEXT, IDC_CAPTURESLIDER);
            }
            else if (sliderhWnd == GetDlgItem(hDlg, IDC_MAXBRIGHTNESSSLIDER)) {
                setTextFromSliderValue(hDlg, IDC_MAXBRIGHTNESSTEXT, IDC_MAXBRIGHTNESSSLIDER);
                int newMaximum = GetWindowInt(hDlg, IDC_MAXBRIGHTNESSTEXT);
                int minimum = GetWindowInt(hDlg, IDC_MINBRIGHTNESSTEXT);
                if (minimum > newMaximum) {
                    HWND minSliderhWnd = GetDlgItem(hDlg, IDC_MINBRIGHTNESSSLIDER);
                    SendMessage(minSliderhWnd, TBM_SETPOS, (WPARAM)newMaximum, (LPARAM)MAKELONG(newMaximum, 0));
                    SetWindowString(hDlg, IDC_MINBRIGHTNESSTEXT, newMaximum);
                }
            }
            else if (sliderhWnd == GetDlgItem(hDlg, IDC_BUCKETSLIDER)) {
                setTextFromSliderValue(hDlg, IDC_BUCKETTEXT, IDC_BUCKETSLIDER);
            }
            break;
        }
    }
    return DefWindowProc(hDlg, uMsg, wParam, lParam);
}

// Helper function that sets the value of a slider to the integer described in a text box
void SettingsWindow::setSliderValueFromText(HWND hWnd, int SLIDER, int TEXT) {
    int newValue = GetWindowInt(hWnd, TEXT);
    HWND sliderhWnd = GetDlgItem(hWnd, SLIDER);
    SendMessage(sliderhWnd, TBM_SETPOS, (WPARAM)newValue, (LPARAM)MAKELONG(newValue, 0));
}

void SettingsWindow::setTextFromSliderValue(HWND hWnd, int TEXT, int SLIDER) {
    HWND sliderhWnd = GetDlgItem(hWnd, SLIDER);
    int newValue = SendMessage(sliderhWnd, TBM_GETPOS, 0, 0);
    SetWindowString(hWnd, TEXT, newValue);
}

// Populates UI elements (text boxes, sliders etc) with data extracted from the Settings object we have.
// Also sets min/max range for sliders...
void SettingsWindow::populateUIFromSettings(HWND hWnd) {
    string lightId = settings->getLightId();
    SetWindowString(hWnd, IDC_LIGHTID, lightId);

    string ipAddress = settings->getIPAddress();
    SetWindowString(hWnd, IDC_IPADDRESS, ipAddress);

    string username = settings->getUsername();
    SetWindowString(hWnd, IDC_USERNAME, username);

    HWND powerOptionhWnd = GetDlgItem(hWnd, IDC_POWERSLEEP);
    bool powerOption = settings->isPowerOptionEnabled();
    SendMessage(powerOptionhWnd, BM_SETCHECK, (WPARAM)powerOption, (LPARAM)0);

    HWND brightnesshWnd = GetDlgItem(hWnd, IDC_BRIGHTNESSCHECK);
    bool brightnessEnabled = settings->isBrightnessEnabled();
    SendMessage(brightnesshWnd, BM_SETCHECK, (WPARAM)brightnessEnabled, (LPARAM)0);

    HWND maxBrightnessSliderhWnd = GetDlgItem(hWnd, IDC_MAXBRIGHTNESSSLIDER);
    SendMessage(maxBrightnessSliderhWnd, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 100));
    int maxBrightnessMinimum = settings->getBrightnessMaximum();
    SendMessage(maxBrightnessSliderhWnd, TBM_SETPOS, (WPARAM)maxBrightnessMinimum, (LPARAM)MAKELONG(maxBrightnessMinimum, 0));
    SetWindowString(hWnd, IDC_MAXBRIGHTNESSTEXT, maxBrightnessMinimum);

    HWND minBrightnessSliderhWnd = GetDlgItem(hWnd, IDC_MINBRIGHTNESSSLIDER);
    SendMessage(minBrightnessSliderhWnd, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 100));
    int minBrightnessMinimum = settings->getBrightnessMinimum();
    SendMessage(minBrightnessSliderhWnd, TBM_SETPOS, (WPARAM)minBrightnessMinimum, (LPARAM)MAKELONG(minBrightnessMinimum, 0));
    SetWindowString(hWnd, IDC_MINBRIGHTNESSTEXT, minBrightnessMinimum);
    
    HWND captureSliderhWnd = GetDlgItem(hWnd, IDC_CAPTURESLIDER);
    SendMessage(captureSliderhWnd, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(30, 3000));
    int captureInterval = settings->getCaptureIntervalMs();
    SendMessage(captureSliderhWnd, TBM_SETPOS, (WPARAM)captureInterval, (LPARAM)MAKELONG(captureInterval, 0));
    SetWindowString(hWnd, IDC_CAPTURETEXT, captureInterval);

    HWND bucketSliderhWnd = GetDlgItem(hWnd, IDC_BUCKETSLIDER);
    SendMessage(bucketSliderhWnd, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(1, 64));
    int bucketInterval = settings->getColourBucketSize();
    SendMessage(bucketSliderhWnd, TBM_SETPOS, (WPARAM)bucketInterval, (LPARAM)MAKELONG(bucketInterval, 0));
    SetWindowString(hWnd, IDC_BUCKETTEXT, bucketInterval);

    int colourMethod = settings->getAverageColourMethod();
    if (colourMethod == 0) {
        CheckDlgButton(hWnd, IDC_RADIOMEAN, BST_CHECKED);
    }
    else if (colourMethod == 1) {
        CheckDlgButton(hWnd, IDC_RADIOMODE, BST_CHECKED);
    }
    else if (colourMethod == 2) {
        CheckDlgButton(hWnd, IDC_RADIOMEDIAN, BST_CHECKED);
    }

}

// Takes values from the settings window and inserts them into the settings object.
// Prefers values from text boxes over sliders, that way users can set values outside of the slider
// ranges if they wish.
void SettingsWindow::populateSettingsFromUI(HWND hDlg) {
    string oldIPAddress = settings->getIPAddress();
    string newIPAddress = GetWindowString(hDlg, IDC_IPADDRESS);
    settings->setIPAddress(newIPAddress);
    bool IPChanged = (newIPAddress.compare(oldIPAddress) != 0);

    string oldLightID = settings->getLightId();
    string newLightID = GetWindowString(hDlg, IDC_LIGHTID);
    settings->setLightID(newLightID);
    bool IDChanged = (newLightID.compare(oldLightID) != 0);

    string oldUsername = settings->getUsername();
    string newUsername = GetWindowString(hDlg, IDC_USERNAME);
    settings->setUsername(newUsername);
    bool usernameChanged = (newUsername.compare(oldUsername) != 0);

    // if IP, ID or username have changed, it's necessary to reconnect to the hub again
    // to have the users new changes immediately take place
    if (IPChanged || IDChanged || usernameChanged) {
        reconnectHub();
    }

    bool brightnessEnabled = IsDlgButtonChecked(hDlg, IDC_BRIGHTNESSCHECK);
    settings->setBrightnessEnabled(brightnessEnabled);
    int brightnessMaximum = GetWindowInt(hDlg, IDC_MAXBRIGHTNESSTEXT);
    settings->setBrightnessMaximum(brightnessMaximum);
    int brightnessMinimum = GetWindowInt(hDlg, IDC_MINBRIGHTNESSTEXT);
    settings->setBrightnessMinimum(brightnessMinimum);

    int bucketSize = GetWindowInt(hDlg, IDC_BUCKETTEXT);
    settings->setColourBucketSize(bucketSize);
    
    int captureInterval = GetWindowInt(hDlg, IDC_CAPTURETEXT);
    settings->setCaptureIntervalMs(captureInterval);

    bool powerOption = IsDlgButtonChecked(hDlg, IDC_POWERSLEEP);
    settings->setPowerOptionEnabled(powerOption);

    if (IsDlgButtonChecked(hDlg, IDC_RADIOMEAN)) {
        settings->setAverageColourMethod(0);
    }
    else if (IsDlgButtonChecked(hDlg, IDC_RADIOMODE)) {
        settings->setAverageColourMethod(1);
    }
    else if (IsDlgButtonChecked(hDlg, IDC_RADIOMEDIAN)) {
        settings->setAverageColourMethod(2);
    }
    else {
        logging->warn("Couldn't figure out which radio button was checked");
    }
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
    int textLen = GetWindowTextLengthA(hWnd) + 1; // +1 seems necessary for null character? odd... not in documentation
    assert(256 >= textLen);
    GetWindowTextA(hWnd, buffer, textLen);
    return buffer;
}

int SettingsWindow::GetWindowInt(HWND hDlg, int item) {
    string s = GetWindowString(hDlg, item);
    int i = -1;
    try {
        i = stoi(s);
    }
    catch (invalid_argument) {
        logging->warn("Failed to convert string to int!");
    }
    return i;
}

void SettingsWindow::SetWindowString(HWND hDlg, int item, string str) {
    HWND hWnd = GetDlgItem(hDlg, item);
    SetWindowTextA(hWnd, str.c_str());
}

void SettingsWindow::SetWindowString(HWND hDlg, int item, int i) {
    HWND hWnd = GetDlgItem(hDlg, item);
    string str = to_string(i);
    SetWindowTextA(hWnd, str.c_str());
}