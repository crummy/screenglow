#pragma once

#include <string>
#include <fstream>

using namespace std;

class Log
{
public:
    Log(string filename);
    Log(string filename, bool debug, bool info, bool warn, bool error);
    void debug(string message);
    void info(string message);
    void warn(string message);
    void error(string message);
    ~Log();
private:
    ofstream file;
    bool debugSwitch;
    bool infoSwitch;
    bool warnSwitch;
    bool errorSwitch;
};