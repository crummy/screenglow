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
#include "Log.h"
#include <fstream>
#include <windows.h>

HANDLE gDoneEvent;
Hue *hue = NULL;
Settings *settings;
ScreenColourCapture *screenCapture;
HINSTANCE hInst;
Log *logging;

using namespace std;

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    if (lpParam == NULL) {
        logging->error("TimerRoutine lpParam is NULL");
    } else {
        COLORREF averageColour = screenCapture->getScreenColour();
        if (hue != NULL) {
            hue->changeColourTo(averageColour, settings->isBrightnessEnabled(), settings->getBrightnessMinimum(), settings->getBrightnessMaximum());
        }
    }
    SetEvent(gDoneEvent);
}


HANDLE setUpWait(int milliseconds) {
    // sets up the screenshot timer
    // largely from http://msdn.microsoft.com/en-us/library/ms687003.aspx
    // returns hTimerQueue for later cleanup
    HANDLE hTimer = NULL;
    HANDLE hTimerQueue = NULL;
    int arg = 123;
    gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (gDoneEvent == NULL) {
        logging->error("CreateEvent failed!");
        return NULL;
    }
    hTimerQueue = CreateTimerQueue();
    if (hTimerQueue == NULL) {
        logging->error("CreateTimerQueue failed!");
        CloseHandle(gDoneEvent);
        return NULL;
    }
    CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK)TimerRoutine, &arg, 0, milliseconds, 0);
    int waitResponse = WaitForSingleObject(gDoneEvent, INFINITE);
    if (waitResponse == WAIT_OBJECT_0) {
        logging->error("WaitForSingleObject failed!");
        CloseHandle(gDoneEvent);
        return NULL;
    }
    CloseHandle(gDoneEvent);
    return hTimerQueue;
}

void quitApp() {
    // if setting is on, turn off light
    logging->info("App quitting");
    if (hue && settings->isPowerOptionEnabled()) {
        hue->turnOff();
    }
    int saveSuccess = settings->saveSettings();
    if (saveSuccess == 0) {
        logging->info("Settings saved.");
    }
    else {
        logging->warn("Settings failed to save!");
    }
}

void sleepApp() {
    logging->info("App going to sleep");
    if (hue && settings->isPowerOptionEnabled()) {
        hue->turnOff();
    }
}

void wakeApp() {
    logging->info("App waking up");
    if (hue && settings->isPowerOptionEnabled()) {
        hue->turnOn();
    }
}

// Kills old hub, connects to new hub.
// Presumably called after something's changed, like the hub IP or light ID.
// Only connect if IP, ID, and username exist.
void reconnectHub() {
    delete hue;
    const string ip = settings->getIPAddress();
    const string id = settings->getLightId();
    const string username = settings->getUsername();
    logging->info("Connecting to hub, IP: " + ip + ", light: " + id + ", username: " + username);
    if (!ip.empty() && !id.empty() && !username.empty()) {
        hue = new Hue(ip, id, username);
        hue->turnOn();
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

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow) {
    // set up globals. sorry for globals.
    settings = new Settings();
    screenCapture = new ScreenColourCapture();
    screenCapture->setAverageColourMethod(settings->getAverageColourMethod());
    hInst = hInstance;
    logging = new Log("ScreenGlow_log.txt", true, true, true, true);

    reconnectHub();

    HANDLE timerQueue = setUpWait(settings->getCaptureIntervalMs());

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
    BOOL res = DeleteTimerQueue(timerQueue);
    return (int)msg.wParam;
}