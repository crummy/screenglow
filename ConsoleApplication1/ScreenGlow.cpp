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

void quitApp() {
    // if setting is on, turn off light
    if (hue) {
        hue->turnOff();
    }
    settings->saveSettings();
}

void sleepApp() {
    if (hue) {
        hue->turnOff();
    }
}

void wakeApp() {
    if (hue) {
        hue->turnOn();
    }
}

// Kills old hub, connects to new hub.
// Presumably called after something's changed, like the hub IP or light ID.
// Only connect if IP, ID, and username exist.
void reconnectHub() {
    delete hue;
    string ip = settings->getIPAddress();
    string id = settings->getLightId();
    string username = settings->getUsername();
    if (!ip.empty() && !id.empty() && !username.empty()) {
        hue = new Hue(ip, id, username);
    }
}

void openSettingsWindow() {
    SettingsWindow *settingsWindow = new SettingsWindow(settings, reconnectHub);
    settingsWindow->show(hInst);
}

void openAboutWindow() {
    AboutWindow *aboutWindow = new AboutWindow();
    aboutWindow->show(hInst);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // set up globals. sorry for globals.
    settings = new Settings();
    screenCapture = new ScreenColourCapture();
    hInst = hInstance;

    reconnectHub();

    HANDLE timerQueue = setUpWait(1000);

    TaskbarIcon *taskbarIcon = new TaskbarIcon(openSettingsWindow, openAboutWindow, quitApp, sleepApp, wakeApp);
    taskbarIcon->show(hInstance);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hue) {
        delete hue;
    }
    DeleteTimerQueue(timerQueue);
    return (int)msg.wParam;
}