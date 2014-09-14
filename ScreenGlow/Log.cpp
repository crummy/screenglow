#include "stdafx.h"
#include "Log.h"


Log::Log(string filename)
{
    Log(filename, false, false, true, true);
}

Log::Log(string filename, bool debug, bool info, bool warn, bool error) {
    this->debugSwitch = debug;
    this->infoSwitch = info;
    this->warnSwitch = warn;
    this->errorSwitch = error;
    this->file.open(filename);
}

void Log::debug(string message) {
    if (debugSwitch) {
        file << message << endl;
    }
}

void Log::info(string message) {
    if (infoSwitch) {
        file << message << endl;
    }
}

void Log::warn(string message) {
    if (warnSwitch) {
        file << message << endl;
    }
}

void Log::error(string message) {
    if (errorSwitch) {
        file << message << endl;
    }
}

Log::~Log()
{
    file.close();
}
