#pragma once
#include <string>
#include <map>

using namespace std;

// Loads and saves settings to/from a file, and generates defaults if needed.
// Plus getters and setters for every method.

class Settings
{
public:
    Settings();
    ~Settings();
    int saveSettings();
    int getAverageColourMethod();
    void setAverageColourMethod(int method);
    bool isBrightnessEnabled();
    void setBrightnessEnabled(bool enabled);
    int getBrightnessMinimum();
    void setBrightnessMinimum(int minimum);
    int getBrightnessMaximum();
    void setBrightnessMaximum(int maximum);
    int getCaptureIntervalMs();
    void setCaptureIntervalMs(int ms);
    string getIPAddress();
    void setIPAddress(string IPAddress);
    string getUsername();
    void setUsername(string username);
    string getLightId();
    void setLightID(string lightID);
    int getColourBucketSize();
    void setColourBucketSize(int size);
    bool isPowerOptionEnabled();
    void setPowerOptionEnabled(bool enabled);
private:
    map<string, string> settings;
    const string filename = "ScreenGlow_Settings.txt";
    void loadDefaults();

    // Keys and default values for settings.
    // Used to lookup into settings map, and to store/read from disk.
    const string averageColourMethod = "averageColourMethod";
    const string averageColourMethodDefault = "0";
    const string username = "username";
    const string usernameDefault = "";
    const string brightnessEnabled = "isBrightnessEnabled";
    const string brightnessEnabledDefault = "false";
    const string brightnessMinimum = "brightnessMinimum";
    const string brightnessMinimumDefault = "25";
    const string brightnessMaximum = "brightnessMaximum";
    const string brightnessMaximumDefault = "100";
    const string captureIntervalMs = "captureIntervalMs";
    const string captureIntervalMsDefault = "1000";
    const string ipAddress = "ipAddress";
    const string ipAddressDefault = "";
    const string lightId = "lightId";
    const string lightIdDefault = "1";
    const string colourBucketSize = "colourBucketSize";
    const string colourBucketSizeDefault = "8";
    const string powerOptionEnabled = "isPowerOptionEnabled";
    const string powerOptionEnabledDefault = "true";
};

