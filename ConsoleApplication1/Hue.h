#pragma once

#include "stdafx.h"
#include <curl/curl.h>
#include <string>

using namespace std;

class Hue
{
    struct Point {
        Point(float X, float Y) { x = X; y = Y; };
        Point() { x = 0; y = 0; };
        float x;
        float y;
    };
public:
    Hue(string ip);
    int selectLight(string id);
    int registerApp();
    int changeColourTo(COLORREF colour);
    ~Hue();
private:
    Point Hue::convertColourToXY(COLORREF colour);
    int Hue::convertColourToBrightness(COLORREF colour);
    bool Hue::checkPointInLampsReach(Point p, Point *colorPoints);
    float Hue::getDistanceBetweenTwoPoints(Point one, Point two);
    Point Hue::getClosestPointToPoints(Point A, Point B, Point P);
    float Hue::crossProduct(Point one, Point two);
    int Hue::sendMessage(string URL, string message, string &returned_data);
    string ip;
    string username;
    string light_id;
    bool debug = false;
};

