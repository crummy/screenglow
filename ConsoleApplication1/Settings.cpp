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
        logging->error("Error reading settings file. Loading defaults.");
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
        logging->error("Exception writing settings file!");
        return 0;
    }
    return 1;
}


Settings::~Settings()
{
}

int Settings::getAverageColourMethod() {
    if (settings.find(averageColourMethod) == settings.end()) {
        settings[averageColourMethod] = averageColourMethodDefault;
    }
    return stoi(settings[averageColourMethod]);
}

void Settings::setAverageColourMethod(int method) {
    settings[averageColourMethod] = to_string(method);
}

bool Settings::isBrightnessEnabled() {
    if (settings.find(brightnessEnabled) == settings.end()) {
        settings[brightnessEnabled] = brightnessEnabledDefault;
    }
    if (settings[brightnessEnabled].compare("true") == 0) {
        return true;
    }
    else {
        return false;
    }
}

void Settings::setBrightnessEnabled(bool enabled) {
    if (enabled) {
        settings[brightnessEnabled] = "true";
    }
    else {
        settings[brightnessEnabled] = "false";
    }
}

int Settings::getBrightnessMinimum() {
    if (settings.find(brightnessMinimum) == settings.end()) {
        settings[brightnessMinimum] = brightnessMinimumDefault;
    }
    return stoi(settings[brightnessMinimum]);
}

void Settings::setBrightnessMinimum(int minimum) {
    settings[brightnessMinimum] = to_string(minimum);
}

int Settings::getBrightnessMaximum() {
    if (settings.find(brightnessMaximum) == settings.end()) {
        settings[brightnessMaximum] = brightnessMaximumDefault;
    }
    return stoi(settings[brightnessMaximum]);
}

void Settings::setBrightnessMaximum(int maximum) {
    settings[brightnessMaximum] = to_string(maximum);
}

int Settings::getCaptureIntervalMs() {
    if (settings.find(captureIntervalMs) == settings.end()) {
        settings[captureIntervalMs] = captureIntervalMsDefault;
    }
    return stoi(settings[captureIntervalMs]);
}

void Settings::setCaptureIntervalMs(int interval) {
    settings[captureIntervalMs] = to_string(interval);
}

string Settings::getIPAddress() {
    if (settings.find(ipAddress) == settings.end()) {
        settings[ipAddress] = ipAddressDefault;
    }
    return settings[ipAddress];
}

void Settings::setIPAddress(string ip) {
    settings[ipAddress] = ip;
}

string Settings::getUsername() {
    if (settings.find(username) == settings.end()) {
        settings[username] = usernameDefault;
    }
    return settings[username];
}

void Settings::setUsername(string username) {
    settings[username] = username;
}

string Settings::getLightId() {
    if (settings.find(lightId) == settings.end()) {
        settings[lightId] = lightIdDefault;
    }
    return settings[lightId];
}

void Settings::setLightID(string lightID) {
    settings[lightId] = lightID;
}

int Settings::getColourBucketSize() {
    if (settings.find(colourBucketSize) == settings.end()) {
        settings[colourBucketSize] = colourBucketSizeDefault;
    }
    return stoi(settings[colourBucketSize]);
}

void Settings::setColourBucketSize(int size) {
    settings[colourBucketSize] = to_string(size);
}

bool Settings::isPowerOptionEnabled() {
    if (settings.find(powerOptionEnabled) == settings.end()) {
        settings[powerOptionEnabled] = powerOptionEnabledDefault;
    }
    if (settings[powerOptionEnabled].compare("true") == 0) {
        return true;
    }
    else {
        return false;
    }
}

void Settings::setPowerOptionEnabled(bool enabled) {
    if (enabled) {
        settings[powerOptionEnabled] = "true";
    }
    else {
        settings[powerOptionEnabled] = "false";
    }
}

void Settings::loadDefaults() {
    settings[averageColourMethod] = averageColourMethodDefault;
    settings[brightnessEnabled] = brightnessEnabledDefault;
    settings[username] = usernameDefault;
    settings[brightnessMinimum] = brightnessMinimumDefault;
    settings[brightnessMaximum] = brightnessMaximumDefault;
    settings[captureIntervalMs] = captureIntervalMsDefault;
    settings[ipAddress] = ipAddressDefault;
    settings[lightId] = lightIdDefault;
    settings[colourBucketSize] = colourBucketSizeDefault;
    settings[powerOptionEnabled] = powerOptionEnabledDefault;
}