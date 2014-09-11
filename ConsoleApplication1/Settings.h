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
    void setBrightnessMinimum(int minimum);
    int getCaptureIntervalMs();
    void setCaptureIntervalMs(int ms);
    TCHAR* getIPAddress();
    void setIPAddress(TCHAR* IPAddress);
    TCHAR* getLightId();
    void setLightID(TCHAR* lightID);
    int getColourBucketSize();
    int saveSettings();
private:
    map<string, TCHAR*> settings;
    const string filename = "ScreenGlow_Settings.txt";
    void loadDefaults();
};

