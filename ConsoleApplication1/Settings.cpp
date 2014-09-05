#include "stdafx.h"
#include "Settings.h"

#include <fstream>
#include <iostream>
#include <sstream>

Settings::Settings() {
    ifstream file(filename);
    if (file) {
        string settingKey;
        string settingValue;
        string line;
        string delimiter = "=";
        while (getline(file, line)) {
            int delimiterPosition = line.find(delimiter);
            settingKey = line.substr(0, delimiterPosition);
            settingValue = line.substr(delimiterPosition + 1);
            settings[settingKey] = settingValue;
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
        for (map<string,string>::iterator iterator = settings.begin(); iterator != settings.end(); iterator++) {
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

string Settings::getAverageColourMethod() {
    return settings["averageColourMethod"];
}

bool Settings::isBrightnessEnabled() {
    if (settings["isBrightnessEnabled"] == "true") return true;
    else if (settings["isBrightnessEnabled"] == "false") return false;
    else cout << "Error reading brightness setting!" << endl;
    return true;
}

float Settings::getBrightnessMinimum() {
    return (float)atof(settings["brightnessMinimum"].c_str());
}

int Settings::getCaptureIntervalMs() {
    return atoi(settings["captureIntervalMs"].c_str());
}

string Settings::getIPAddress() {
    return settings["ipAddress"];
}

string Settings::getLightId() {
    return settings["lightId"];
}

int Settings::getColourBucketSize() {
    return atoi(settings["colourBucketSize"].c_str());
}

void Settings::loadDefaults() {
    settings["averageColourMethod"] = "average";
    settings["isBrightnessEnabled"] = "true";
    settings["brightnessMinimum"] = "25";
    settings["captureIntervalMs"] = "1000";
    settings["ipAddress"] = "192.168.1.42";
    settings["lightId"] = "3";
    settings["colourBucketSize"] = "8";
}