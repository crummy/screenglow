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
            file << iterator->first << "=" << iterator->second << endl; // TODO: Convert TCHAR to string before writing
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
    return atoi(settings["averageColourMethod"].c_str());
}

bool Settings::isBrightnessEnabled() { // TODO: real string comparison
    if (settings["isBrightnessEnabled"].compare("true") == 0) return true;
    else if (settings["isBrightnessEnabled"].compare("false") == 0) return false;
    else cout << "Error reading brightness setting!" << endl;
    return true;
}

int Settings::getBrightnessMinimum() {
    return atoi(settings["brightnessMinimum"].c_str());
}

void Settings::setBrightnessMinimum(int minimum) {
    string minString = to_string(minimum);
    settings["brightnessMinimum"] = minString;
}

int Settings::getCaptureIntervalMs() {
    return atoi(settings["captureIntervalMs"].c_str());
}

void Settings::setCaptureIntervalMs(int interval) {
    settings["captureIntervalMs"] = to_string(interval);
}

string Settings::getIPAddress() {
    return settings["ipAddress"];
}

void Settings::setIPAddress(string ip) {
    settings["ipAddress"] = ip;
}

string Settings::getUsername() {
    return settings["username"];
}

void Settings::setUsername(string username) {
    settings["username"] = username;
}

string Settings::getLightId() {
    string id = settings["lightId"];
    return settings["lightId"];
}

void Settings::setLightID(string lightID) {
    settings["lightId"] = lightID;
}

int Settings::getColourBucketSize() {
    return atoi(settings["colourBucketSize"].c_str());
}

void Settings::loadDefaults() {
    settings["averageColourMethod"] = "0";
    settings["isBrightnessEnabled"] = "false";
    settings["username"] = "";
    settings["brightnessMinimum"] = "25";
    settings["captureIntervalMs"] = "1000";
    settings["ipAddress"] = "";
    settings["lightId"] = "";
    settings["colourBucketSize"] = "8";
}