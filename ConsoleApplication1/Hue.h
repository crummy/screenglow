#pragma once

#include "stdafx.h"
#include <curl/curl.h>
#include <string>

using namespace std;

enum connectionStatus {
    connectionOK,
    connectionBadIP,
    connectionBadID,
    connectionNeedsLink,
    connectionUnknownError
};

class Hue
{
    struct Point {
        Point(float X, float Y) { x = X; y = Y; };
        Point() { x = 0; y = 0; };
        float x;
        float y;
    };
public:
    Hue(string ip, string lightID, string username);
    connectionStatus testConnection();
    int changeColourTo(COLORREF colour, bool brightness, int minBrightness, int maxBrightness);
    string getUsername();
    void turnOn();
    void turnOff();
    ~Hue();
private:
    Point convertColourToXY(COLORREF colour);
    int convertColourToBrightness(COLORREF colour);
    bool checkPointInLampsReach(Point p, Point *colorPoints);
    float getDistanceBetweenTwoPoints(Point one, Point two);
    Point getClosestPointToPoints(Point A, Point B, Point P);
    float crossProduct(Point one, Point two);
    int sendMessage(string URL, string message, string &returned_data);
    int sendMessage(string URL, string message, string method, string &returned_data);
    string ip;
    string username;
    string lightID;
};

