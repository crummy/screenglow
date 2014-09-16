#include "stdafx.h"
#include "ScreenColourCapture.h"
#include <algorithm>
#include <map>

ScreenColourCapture::ScreenColourCapture()
{
    selectedMethod = MEDIANCOLOUR;
    bucketSize = 8;
}


ScreenColourCapture::~ScreenColourCapture()
{
}

COLORREF ScreenColourCapture::getScreenColour() {
    // Most of this is adapted from http://www.cplusplus.com/forum/beginner/25138/
    LPBITMAPINFO lpbi = NULL;
    HBITMAP OffscrBmp = NULL;
    HDC OffscrDC = NULL;
    int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    HDC bitmapDC = CreateCompatibleDC(0);
    HDC hDC = GetDC(0);
    HBITMAP hBmp = CreateCompatibleBitmap(hDC, nScreenWidth, nScreenHeight);
    SelectObject(bitmapDC, hBmp);
    BitBlt(bitmapDC, 0, 0, nScreenWidth, nScreenHeight, hDC, 0, 0, SRCCOPY);
    if ((OffscrBmp = CreateCompatibleBitmap(bitmapDC, nScreenWidth, nScreenHeight)) == NULL) {
        int error = GetLastError();
        logging->error("CreateCompatibleBitmap failed! Error: " + to_string(error));
        return 0;
    }
    if ((OffscrDC = CreateCompatibleDC(bitmapDC)) == NULL) {
        logging->error("CreateCompatibleDC failed!");
        return 0;
    }
    DeleteObject(hBmp); // must be deleted before we select another object or we leak..?
    HBITMAP OldBmp = (HBITMAP)SelectObject(OffscrDC, OffscrBmp);
    BitBlt(OffscrDC, 0, 0, nScreenWidth, nScreenHeight, bitmapDC, 0, 0, SRCCOPY);
    if ((lpbi = (LPBITMAPINFO)(new char[sizeof(BITMAPINFOHEADER)+256 * sizeof(RGBQUAD)])) == NULL)
        return 0;
    ZeroMemory(&lpbi->bmiHeader, sizeof(BITMAPINFOHEADER));
    lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    SelectObject(OffscrDC, OldBmp);

    // First, call GetDIBits with no pixel array. This way it will populate bitmapinfo for us. Then
    // call it with the pixel array and the now populated lpbi.
    GetDIBits(OffscrDC, OffscrBmp, 0, nScreenHeight, NULL, lpbi, DIB_RGB_COLORS);
    LPVOID lpvBits = new char[lpbi->bmiHeader.biSizeImage];
    GetDIBits(OffscrDC, OffscrBmp, 0, nScreenHeight, lpvBits, lpbi, DIB_RGB_COLORS);

    // Pass BMP data off for computation
    COLORREF averageColour;
    if (selectedMethod == MEANCOLOUR) {
        averageColour = getMeanColourFromPixels((BYTE *)lpvBits, lpbi);
    }
    else if (selectedMethod == MEDIANCOLOUR) {
        averageColour = getMedianColourFromPixels((BYTE *)lpvBits, lpbi);
    }
    else if (selectedMethod == MODECOLOUR) {
        averageColour = getModeColourFromPixels((BYTE *)lpvBits, lpbi); // TODO
    }
    else {
        logging->warn("Could not figure out which colour method to use!");
        averageColour = 0;
    }

    // Wrap things up
    delete[] lpvBits;
    delete[] lpbi;
    DeleteObject(OldBmp);
    DeleteObject(OffscrBmp);
    DeleteObject(bitmapDC);
    ReleaseDC(0, hDC);
    DeleteDC(OffscrDC);

    return averageColour;
}

void ScreenColourCapture::setAverageColourMethod(int method) {
    this->selectedMethod = (ColourAverageMethod)method;
}

void ScreenColourCapture::setModeBucketSize(int bucketSize) {
    this->bucketSize = bucketSize;
}

COLORREF ScreenColourCapture::getMeanColourFromPixels(const BYTE *pixels, const LPBITMAPINFO info) {
#pragma warning(suppress: 28159)
    DWORD startTime = GetTickCount();
    int totalPixels = info->bmiHeader.biWidth * info->bmiHeader.biHeight;
    float totalRed = 0;
    float totalBlue = 0;
    float totalGreen = 0;
    for (int pixel = 0; pixel < totalPixels; pixel += 1) {
        int pixelIndex = pixel * (info->bmiHeader.biBitCount / 8);
        totalRed += pixels[pixelIndex + 2];
        totalGreen += pixels[pixelIndex + 1];
        totalBlue += pixels[pixelIndex + 0];
    }
    float averageRed = totalRed / totalPixels;
    float averageGreen = totalGreen / totalPixels;
    float averageBlue = totalBlue / totalPixels;
#pragma warning(suppress: 28159)
    logging->info("found mean colour in " + to_string(GetTickCount() - startTime) + "ms");
    return RGB((int)averageRed, (int)averageGreen, (int)averageBlue);
}

// returns true if a < b, where "<" is "less bright".
// useful for sort() or nth_element()
struct pixelCompare {
    inline bool operator() (const BYTE *a, const BYTE *b) {
        return (a[0] + a[1] + a[2]) < (b[0] + b[1] + b[2]);
    }
};

COLORREF ScreenColourCapture::getMedianColourFromPixels(const BYTE *pixels, const LPBITMAPINFO info) {
#pragma warning(suppress: 28159)
    DWORD startTime = GetTickCount();
    int totalPixels = info->bmiHeader.biWidth * info->bmiHeader.biHeight;
    int pixelSize = info->bmiHeader.biBitCount / 8;
    vector<const BYTE*> pixelPointers;
    for (int pixel = 0; pixel < totalPixels; pixel++) {
        pixelPointers.push_back(&pixels[pixel * pixelSize]);
    }
    nth_element(pixelPointers.begin(), pixelPointers.begin() + totalPixels / 2, pixelPointers.end(), pixelCompare());
    const BYTE* median = pixelPointers[totalPixels / 2];
    int red = (int)(median[2]);
    int green = (int)(median[1]);
    int blue = (int)(median[0]);
#pragma warning(suppress: 28159)
    logging->info("found median colour in " + to_string(GetTickCount() - startTime) + "ms");
    return RGB(red, green, blue);
}

COLORREF ScreenColourCapture::getModeColourFromPixels(const BYTE *pixels, const LPBITMAPINFO info) {
#pragma warning(suppress: 28159)
    DWORD startTime = GetTickCount();
    int totalPixels = info->bmiHeader.biWidth * info->bmiHeader.biHeight;
    int pixelSize = info->bmiHeader.biBitCount / 8;
    map<COLORREF, int> colourBuckets;
    for (int pixel = 0; pixel < totalPixels; pixel++) {
        int pixelIndex = pixel * pixelSize;
        COLORREF colour = RGB(pixels[pixelIndex + 2], pixels[pixelIndex + 1], pixels[pixelIndex + 0]);
        colourBuckets[colour]++;
    }
    COLORREF mostCommonColour;
    int mostCommonColourCount = 0;
    for (map<COLORREF, int>::iterator it = colourBuckets.begin(); it != colourBuckets.end(); it++) {
        if (it->second > mostCommonColourCount) {
            mostCommonColourCount = it->second;
            mostCommonColour = it->first;
        }
    }
#pragma warning(suppress: 28159)
    logging->info("found mode colour in " + to_string(GetTickCount() - startTime) + "ms");
    return mostCommonColour;
}