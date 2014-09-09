#include "stdafx.h"
#include "Settings.h"
#include <atlstr.h>

#include <fstream>
#include <iostream>
#include <sstream>

Settings::Settings() {
    ifstream file(filename);
    if (file) {
        string settingKey;
        string settingValue;
        TCHAR *settingValueTCHAR;
        string line;
        string delimiter = "=";
        while (getline(file, line)) {
            int delimiterPosition = line.find(delimiter);
            settingKey = line.substr(0, delimiterPosition);
            settingValue = line.substr(delimiterPosition + 1);
            settingValueTCHAR = new TCHAR[settingValue.size() + 1];
            _tcscpy_s(settingValueTCHAR, settingValue.size() * sizeof(TCHAR), CA2T(settingValue.c_str()));
            settings[settingKey] = settingValueTCHAR;
        }
    } else {
        cout << "Exception reading settings file. Loading defaults." << endl;
        loadDefaults();
    }
    file.close();
}

int Settings::saveSettings() {
    try {
        ofstream file(filename);
        for (map<string,TCHAR*>::iterator iterator = settings.begin(); iterator != settings.end(); iterator++) {
            file << iterator->first << "=" << iterator->second << endl;
        }
        file.close();
    }
    catch (ofstream::failure e) {
        cout << "Exception writing settings file!" << endl;
        return 0;
    }
    return 1;
}


Settings::~Settings()
{
}

int Settings::getAverageColourMethod() {
    return _tstoi(settings["averageColourMethod"]);
}

bool Settings::isBrightnessEnabled() { // TODO: real string comparison
    if (settings["isBrightnessEnabled"] == _T("true")) return true;
    else if (settings["isBrightnessEnabled"] == _T("false")) return false;
    else cout << "Error reading brightness setting!" << endl;
    return true;
}

int Settings::getBrightnessMinimum() {
    return _tstoi(settings["brightnessMinimum"]);
}

int Settings::getCaptureIntervalMs() {
    return _tstoi(settings["captureIntervalMs"]);
}

TCHAR* Settings::getIPAddress() {
    return settings["ipAddress"];
}

TCHAR* Settings::getLightId() {
    return settings["lightId"];
}

int Settings::getColourBucketSize() {
    return _tstoi(settings["colourBucketSize"]);
}

void Settings::loadDefaults() {
    settings["averageColourMethod"] = _T("0");
    settings["isBrightnessEnabled"] = _T("true");
    settings["brightnessMinimum"] = _T("25");
    settings["captureIntervalMs"] = _T("1000");
    settings["ipAddress"] = _T("192.168.1.42");
    settings["lightId"] = _T("3");
    settings["colourBucketSize"] = _T("8");
}