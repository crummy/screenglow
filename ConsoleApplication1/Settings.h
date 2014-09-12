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
    string getIPAddress();
    void setIPAddress(string IPAddress);
    string getLightId();
    void setLightID(string lightID);
    int getColourBucketSize();
    int saveSettings();
private:
    map<string, string> settings;
    const string filename = "ScreenGlow_Settings.txt";
    void loadDefaults();
};

