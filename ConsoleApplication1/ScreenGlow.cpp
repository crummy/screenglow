// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Hue.h"
#include <iostream>
#include <fstream>
#include <windows.h>

HANDLE gDoneEvent;
Hue *hue;

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