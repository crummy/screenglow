#pragma once

#include <Windows.h>
#include <string.h>
#include <vector>

enum ColourAverageMethod {
    MEANCOLOUR,
    MODECOLOUR,
    MEDIANCOLOUR
};

using namespace std;

class ScreenColourCapture
{
public:
    ScreenColourCapture();
    ~ScreenColourCapture();
    COLORREF getScreenColour();
    void setAverageColourMethod(ColourAverageMethod method);

private:
    COLORREF getMeanColourFromPixels(const BYTE *pixels, const LPBITMAPINFO infos);
    COLORREF getModeColourFromPixels(const BYTE *pixels, const LPBITMAPINFO info);
    COLORREF getMedianColourFromPixels(const BYTE *pixels, const LPBITMAPINFO info);
    ColourAverageMethod selectedMethod;
};

