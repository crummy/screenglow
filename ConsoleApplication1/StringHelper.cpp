#include "stdafx.h"
#include "StringHelper.h"

// from http://vajris.wordpress.com/2012/10/15/conversion-tchar-wstring-string-string/
string StringHelper::TCHARtoString(TCHAR* tchar) {
    wstring w(&tchar[0]);
    string s(w.begin(), w.end());
    return s;
}

LPCWSTR StringHelper::StringToWstr(string str) {
    wstring wstr(str.begin(), str.end());
    return wstr.c_str();
}

TCHAR* StringHelper::IntToTCHAR(int i) {
    wstring s = to_wstring(i);
    //TCHAR* t = s.c_str();
    TCHAR* tchar;
    return tchar;
}