#include "stdafx.h"
#include "ScreenColourCapture.h"


ScreenColourCapture::ScreenColourCapture()
{
    selectedMethod = MEANCOLOUR;
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
    COLORREF averageColour = getMeanColourFromPixels((BYTE *)lpvBits, lpbi);

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

void ScreenColourCapture::setAverageColourMethod(ColourAverageMethod method) {
    this->selectedMethod = method;
}

COLORREF ScreenColourCapture::getMeanColourFromPixels(const BYTE *pixels, const LPBITMAPINFO info) {
    int width = info->bmiHeader.biWidth;
    int height = info->bmiHeader.biHeight;
    float totalRed = 0;
    float totalBlue = 0;
    float totalGreen = 0;
    for (int pixel = 0; pixel < height * width; pixel += 1) {
        int pixelIndex = pixel * (info->bmiHeader.biBitCount / 8);
        totalRed += pixels[pixelIndex + 2];
        totalGreen += pixels[pixelIndex + 1];
        totalBlue += pixels[pixelIndex + 0];
    }
    float averageRed = totalRed / (height * width);
    float averageGreen = totalGreen / (height * width);
    float averageBlue = totalBlue / (height * width);
    return RGB((int)averageRed, (int)averageGreen, (int)averageBlue);
}