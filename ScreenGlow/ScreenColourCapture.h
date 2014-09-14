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

// When asked, takes a screenshot and returns the average colour of the screen.
// TODO: Add alternative methods of capturing the screen (to get fullscreen games)
// TODO: Add alternative methods of calculating the average colour of the screen

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
