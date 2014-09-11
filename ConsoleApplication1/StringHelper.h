#pragma once

#include <string>
#include <Windows.h>

using namespace std;

class StringHelper
{
public:
    static string TCHARtoString(TCHAR* tchar);
    static LPCWSTR StringToWstr(string str);
};

