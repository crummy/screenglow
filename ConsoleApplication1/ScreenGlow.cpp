// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Hue.h"
#include "resource.h"
#include "Settings.h"
#include "SettingsWindow.h"
#include "AboutWindow.h"
#include "TaskbarIcon.h"
#include "ScreenColourCapture.h"
#include <iostream>
#include <fstream>
#include <windows.h>

HANDLE gDoneEvent;
Hue *hue = NULL;
Settings *settings;
ScreenColourCapture *screenCapture;
HINSTANCE hInst;

using namespace std;

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    if (lpParam == NULL) {
        printf("TimerRoutine lpParam is NULL\n");
    } else {
        COLORREF averageColour = screenCapture->getScreenColour();
        if (hue != NULL) {
            hue->changeColourTo(averageColour);
        }
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


void openSettingsWindow() {
    SettingsWindow *settingsWindow = new SettingsWindow(settings);
    settingsWindow->show(hInst);
}

void openAboutWindow() {
    AboutWindow *aboutWindow = new AboutWindow();
    aboutWindow->show(hInst);
}

void quitApp() {
    // if setting is on, turn off light
    // hue->turnofflight()
    settings->saveSettings();
}

void sleepApp() {
    // turn off light
}

void wakeApp() {
    // turn on light
}

void newHubConnection(Hue *newHue) {
    delete hue;
    hue = newHue;
}

// from http://vajris.wordpress.com/2012/10/15/conversion-tchar-wstring-string-string/
string TCHARtoString(TCHAR* tchar) {
    wstring w(&tchar[0]);
    string s(w.begin(), w.end());
    return s;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    settings = new Settings();
    screenCapture = new ScreenColourCapture();
    hInst = hInstance;

    //string hubIPAddress = TCHARtoString(settings->getIPAddress());
    //hue = new Hue(hubIPAddress);
    //string lightID = TCHARtoString(settings->getLightId());
    //hue->selectLight(lightID);
    HANDLE timerQueue = setUpWait(1000);
    TaskbarIcon *taskbarIcon = new TaskbarIcon(openSettingsWindow, openAboutWindow, quitApp, sleepApp, wakeApp);
    taskbarIcon->show(hInstance);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteTimerQueue(timerQueue);
    return (int)msg.wParam;
}