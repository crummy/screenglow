// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Hue.h"
#include "resource.h"
#include "Settings.h"
#include "SettingsWindow.h"
#include "TaskbarIcon.h"
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    settings = new Settings();
    screenCapture = new ScreenColourCapture();

    string hubIPAddress = settings->getIPAddress();
    hue = new Hue(hubIPAddress);
    string lightID = settings->getLightId();
    hue->selectLight(lightID);
    HANDLE timerQueue = setUpWait(1000);
    TaskbarIcon *taskbarIcon = new TaskbarIcon();
    taskbarIcon->show(hInstance);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //SettingsWindow *settingsWindow = new SettingsWindow(settings);
    //settingsWindow->show(hInstance);
    DeleteTimerQueue(timerQueue);
    return (int)msg.wParam;
}