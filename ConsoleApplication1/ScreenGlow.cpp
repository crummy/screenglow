// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Hue.h"
#include "resource.h"
#include "Settings.h"
#include "SettingsWindow.h"
#include "ScreenColourCapture.h"
#include <iostream>
#include <fstream>
#include <windows.h>

HANDLE gDoneEvent;
Hue *hue;
Settings *settings;
ScreenColourCapture *screenCapture;

using namespace std;


VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    if (lpParam == NULL) {
        printf("TimerRoutine lpParam is NULL\n");
    } else {
        COLORREF averageColour = screenCapture->getScreenColour();
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

// thank you http://www.codeproject.com/Articles/4768/Basic-use-of-Shell-NotifyIcon-in-Win
void addTaskbarIcon(HINSTANCE hInstance) {
    NOTIFYICONDATA niData;
    ZeroMemory(&niData, sizeof(NOTIFYICONDATA));
    niData.cbSize = sizeof(NOTIFYICONDATA);
    niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    niData.hIcon = (HICON)LoadImage(hInstance,
        MAKEINTRESOURCE(IDI_ICON),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);
    Shell_NotifyIcon(NIM_ADD, &niData);

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    settings = new Settings();
    screenCapture = new ScreenColourCapture();
    addTaskbarIcon(hInstance);

    string hubIPAddress = settings->getIPAddress();
    hue = new Hue(hubIPAddress);
    string lightID = settings->getLightId();
    hue->selectLight(lightID);
    HANDLE timerQueue = setUpWait(1000);

    SettingsWindow *settingsWindow = new SettingsWindow(settings);
    settingsWindow->show(hInstance);
    //DeleteTimerQueue(timerQueue);
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