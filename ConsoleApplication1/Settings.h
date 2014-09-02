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
    string getAverageColourMethod();
    bool isBrightnessEnabled();
    float getBrightnessMinimum();
    int getCaptureIntervalMs();
    string getIPAddress();
    string getLightId();
    int getColourBucketSize();
    int saveSettings();
private:
    map<string, string> settings;
    const string filename = "ScreenGlow_Settings.txt";
    void loadDefaults();
};

