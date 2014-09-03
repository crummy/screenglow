// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Hue.h"
#include "resource.h"
#include "Settings.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <CommCtrl.h>

HANDLE gDoneEvent;
Hue *hue;
Settings *settings;

using namespace std;

// Iterates over every pixel in *pixels, calculating the average red, green, and blue value
COLORREF getAverageColour(const BYTE *pixels, const LPBITMAPINFO info) {
    int width = info->bmiHeader.biWidth;
    int height = info->bmiHeader.biHeight;
    float totalRed = 0;
    float totalBlue = 0;
    float totalGreen = 0;
    for (int pixel = 0; pixel < height * width; pixel += 1) {
        int pixelIndex = pixel * (info->bmiHeader.biBitCount / 8);
        totalRed += pixels[pixelIndex + 2];
        totalGreen += pixels[pixelIndex + 1];
        totalBlue += pixels[pixelIndex + 0];
    }
    float averageRed = totalRed / (height * width);
    float averageGreen = totalGreen / (height * width);
    float averageBlue = totalBlue / (height * width);
    //printf("Average colour of %d pixels: (%f, %f, %f)\n", height * width, averageRed, averageGreen, averageBlue);
    return RGB((int)averageRed, (int)averageGreen, (int)averageBlue);
}

COLORREF CaptureScreen()
{
    // Most of this is adapted from http://www.cplusplus.com/forum/beginner/25138/
    LPBITMAPINFO lpbi = NULL;
    HBITMAP OffscrBmp = NULL;
    HDC OffscrDC = NULL;
    int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    HDC bitmapDC = CreateCompatibleDC(0);
    HBITMAP hBmp = CreateCompatibleBitmap(GetDC(0), nScreenWidth, nScreenHeight);
    SelectObject(bitmapDC, hBmp);
    BitBlt(bitmapDC, 0, 0, nScreenWidth, nScreenHeight, GetDC(0), 0, 0, SRCCOPY);
    if ((OffscrBmp = CreateCompatibleBitmap(bitmapDC, nScreenWidth, nScreenHeight)) == NULL)
        return 0;
    if ((OffscrDC = CreateCompatibleDC(bitmapDC)) == NULL)
        return 0;
    HBITMAP OldBmp = (HBITMAP)SelectObject(OffscrDC, OffscrBmp);
    BitBlt(OffscrDC, 0, 0, nScreenWidth, nScreenHeight, bitmapDC, 0, 0, SRCCOPY);
    if ((lpbi = (LPBITMAPINFO)(new char[sizeof(BITMAPINFOHEADER)+256 * sizeof(RGBQUAD)])) == NULL)
        return 0;
    ZeroMemory(&lpbi->bmiHeader, sizeof(BITMAPINFOHEADER));
    lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    SelectObject(OffscrDC, OldBmp);

    // First, call GetDIBits with no pixel array. This way it will populate bitmapinfo for us. Then
    // call it with the pixel array and the now populated lpbi.
    GetDIBits(OffscrDC, OffscrBmp, 0, nScreenHeight, NULL, lpbi, DIB_RGB_COLORS);
    LPVOID lpvBits = new char[lpbi->bmiHeader.biSizeImage];
    GetDIBits(OffscrDC, OffscrBmp, 0, nScreenHeight, lpvBits, lpbi, DIB_RGB_COLORS);  

    // Pass BMP data off for computation
    COLORREF averageColour = getAverageColour((BYTE *)lpvBits, lpbi);

    // Wrap things up
    delete[] lpvBits;
    DeleteObject(hBmp);
    ReleaseDC(GetDesktopWindow(), bitmapDC);
    DeleteDC(OffscrDC);
    DeleteObject(OffscrBmp);

    return averageColour;
}

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    if (lpParam == NULL) {
        printf("TimerRoutine lpParam is NULL\n");
    } else {
        COLORREF averageColour = CaptureScreen();
        hue->changeColourTo(averageColour);
    }
    SetEvent(gDoneEvent);
}

HANDLE setUpWait(int milliseconds) {
    // set up wait
    // largely from http://msdn.microsoft.com/en-us/library/ms687003.aspx
    // returns hTimerQueue for later cleanup
    HANDLE hTimer = NULL;
    HANDLE hTimerQueue = NULL;
    int arg = 123;
    gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hTimerQueue = CreateTimerQueue();
    CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK)TimerRoutine, &arg, 0, milliseconds, 0);
    WaitForSingleObject(gDoneEvent, INFINITE);
    CloseHandle(gDoneEvent);
    return hTimerQueue;
}

BOOL CALLBACK AppDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG: {
        HWND lighthWnd = GetDlgItem(hDlg, ID_LIGHTID);
        string lightId = settings->getLightId();
        wstring wLightId = wstring(lightId.begin(), lightId.end());
        SetWindowText(lighthWnd, wLightId.c_str());

        HWND iphWnd = GetDlgItem(hDlg, ID_IPADDRESS);
        string ipAddress = settings->getIPAddress();
        wstring wIpAddress = wstring(ipAddress.begin(), ipAddress.end());
        SetWindowText(iphWnd, wIpAddress.c_str());

        HWND brightnessSliderhWnd = GetDlgItem(hDlg, ID_BRIGHTNESSSLIDER);
        SendMessage(brightnessSliderhWnd, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0,255));
        int brightnessMinimum = (float)settings->getBrightnessMinimum();
        SendMessage(brightnessSliderhWnd, TBM_SETPOS, (WPARAM)brightnessMinimum, (LPARAM)MAKELONG(brightnessMinimum, 0));

        HWND brightnessTexthWnd = GetDlgItem(hDlg, ID_BRIGHTNESSTEXT);
        string brightnessMinimumString = to_string(brightnessMinimum);
        wstring wBrightnessMinimumString = wstring(brightnessMinimumString.begin(), brightnessMinimumString.end());
        SetWindowText(brightnessTexthWnd, wBrightnessMinimumString.c_str());

        HWND captureSliderhWnd = GetDlgItem(hDlg, ID_CAPTURESLIDER);
        SendMessage(captureSliderhWnd, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(30, 3000));
        int captureInterval = (float)settings->getCaptureIntervalMs();
        SendMessage(captureSliderhWnd, TBM_SETPOS, (WPARAM)captureInterval, (LPARAM)MAKELONG(captureInterval, 0));

        HWND captureTexthWnd = GetDlgItem(hDlg, ID_CAPTURETEXT);
        string captureIntervalString = to_string(captureInterval);
        wstring wCaptureIntervalString = wstring(captureIntervalString.begin(), captureIntervalString.end());
        SetWindowText(captureTexthWnd, wCaptureIntervalString.c_str());

        return 1;
        break;
    }
    case WM_COMMAND:
        switch (wParam) {
        case ID_OK:
            return 0;
        case ID_CANCEL:
            EndDialog(hDlg, 0);
            break;
        case ID_TESTBUTTON: {
            HWND hWnd = GetDlgItem(hDlg, ID_TESTRESULT);
            SetWindowText(hWnd, _T("test clicked"));
            break;
        }
        case EN_CHANGE:
            HWND hWnd = (HWND)lParam;
            if (hWnd == GetDlgItem(hDlg, ID_BRIGHTNESSTEXT)) {
                string newBrightnessMinimumString;
                newBrightnessMinimumString.resize(GetWindowTextLength(hWnd) + 1, '\0');
                GetWindowText(hWnd, LPWSTR(newBrightnessMinimumString.c_str()), 8);
                int newBrightnessMinimum = atoi(newBrightnessMinimumString.c_str());
                HWND brightnessSliderhWnd = GetDlgItem(hDlg, ID_BRIGHTNESSSLIDER);
                SendMessage(brightnessSliderhWnd, TBM_SETPOS, (WPARAM)newBrightnessMinimum, (LPARAM)MAKELONG(newBrightnessMinimum, 0));
            }
            break;
        }
    case WM_HSCROLL:
        switch (LOWORD(wParam)) {
        case TB_THUMBTRACK:
        case TB_ENDTRACK:
            HWND sliderhWnd = (HWND)lParam;
            if (sliderhWnd == GetDlgItem(hDlg, ID_BRIGHTNESSSLIDER)) {
                int newBrightnessMinimum = SendMessage(sliderhWnd, TBM_GETPOS, 0, 0);
                string newBrightnessMinimumString = to_string(newBrightnessMinimum);
                wstring wNewBrightnessMinimumString = wstring(newBrightnessMinimumString.begin(), newBrightnessMinimumString.end());
                HWND texthWnd = GetDlgItem(hDlg, ID_BRIGHTNESSTEXT);
                SetWindowText(texthWnd, wNewBrightnessMinimumString.c_str());
            }
            else if (sliderhWnd == GetDlgItem(hDlg, ID_CAPTURESLIDER)) {
                int newCaptureInterval = SendMessage(sliderhWnd, TBM_GETPOS, 0, 0);
                string newCaptureIntervalString = to_string(newCaptureInterval);
                wstring wNewCaptureIntervalString = wstring(newCaptureIntervalString.begin(), newCaptureIntervalString.end());
                HWND texthWnd = GetDlgItem(hDlg, ID_CAPTURETEXT);
                SetWindowText(texthWnd, wNewCaptureIntervalString.c_str());
            }
            break;
        }
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    settings = new Settings();
    DialogBoxParam(hInstance, (LPCTSTR)IDD_DIALOG1, 0, AppDlgProc, 0);
    return 0;
}

int main() {
    cout << "Establishing connection with Hue hub." << endl;
    hue = new Hue("192.168.1.42");
    cout << "Selecting light." << endl;
    if (hue->selectLight("3") == 0) {
        cout << "Successfully selected light." << endl;
    }
    HANDLE timerQueue = setUpWait(1000);
    while (true) {
        Sleep(0);
    }
    DeleteTimerQueue(timerQueue);
    return 0;
}