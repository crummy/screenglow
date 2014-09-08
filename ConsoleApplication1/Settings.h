#pragma once
#include <string>
#include <map>

using namespace std;

class Settings
{
public:
    Settings();
    Settings(string filename);
    ~Settings();
    int getAverageColourMethod();
    bool isBrightnessEnabled();
    int getBrightnessMinimum();
    int getCaptureIntervalMs();
    TCHAR* getIPAddress();
    TCHAR* getLightId();
    int getColourBucketSize();
    int saveSettings();
private:
    map<string, TCHAR*> settings;
    const string filename = "ScreenGlow_Settings.txt";
    void loadDefaults();
};

