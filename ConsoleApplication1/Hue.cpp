#include "stdafx.h"
#include "Hue.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <assert.h>


Hue::Hue(string ip, string lightID, string username) {
    this->ip = ip;
    this->lightID = lightID;
    this->username = username;
}

// The CURL library will call this with data returned from HTTP requests. We put it at a memory address
// that was specified earlier (e.g. in registerApp).
// From http://curl.haxx.se/libcurl/c/htmltitle.html
int WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data) {
    if (data) {
        ostream& os = *static_cast<ostream*>(data);
        streamsize len = size*nmemb;
        if (os.write(static_cast<char*>(ptr), len))
            return (int)len;
    }
    return 0;
}

// Performs the leg work of communicating with the Hue hub.
// Accepts the URL to the requested API, the actual data to be sent, and the address of a string
// which will be populated with JSON response on return.
// Returns 0 on success.
int Hue::sendMessage(string URL, string message, string method, string &returned_data) {
    int success = 0;
    CURL *curl = curl_easy_init();
    if (curl) {
        ostringstream data;
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "charsets: utf-8");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, message.length());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            logging->error("curl_easy_perform() failed: " + res);
            success = (int)res;
        } else {
            returned_data = data.str();
        }
        curl_easy_cleanup(curl);
    }
    else {
        logging->error("curl failed initialization!!");
        success = -1;
    }
    return success;
}

int Hue::sendMessage(string URL, string message, string &returned_data) {
    string defaultMethod = "PUT";
    return sendMessage(URL, message, defaultMethod, returned_data);
}

// Tests the IP address, username, and light ID.
// If the username is wrong, or if there is no username, connectionNeedsLink
connectionStatus Hue::testConnection() {
    string testResponse;
    if (username.compare("") != 0) {
        string testURL = "http://" + ip + "/api/" + username + "/lights";
        int testResult = sendMessage(testURL, "", "GET", testResponse);
        if (testResult == CURLE_HTTP_RETURNED_ERROR) {
            logging->warn("Tested connection, but received HTTP error >400");
            return connectionBadIP;
        }
        else if (testResult != 0) {
            logging->warn("Tested connection, but received error " + testResult);
            return connectionUnknownError;
        }
    }
    if ((username.compare("") == 0) || (testResponse.find("unauthorized user") != string::npos)) {
        // We need to register a username.
        string registerURL = "http://" + ip + "/api";
        string registerBody = "{\"devicetype\": \"ScreenGlow\"}";
        string registerResponse;
        int registerResult = sendMessage(registerURL, registerBody, "POST", registerResponse);
        if (registerResponse.find("link button not pressed") != string::npos) {
            // If the link button was not pressed but needs to be, just quit out. Outside you will handle
            // prompting them to press the button, and presumably call testConnection() again.
            logging->info("Attempting to acquire username, but link button not pressed");
            return connectionNeedsLink;
        }
        else {
            // Got a response from username registration. Extract and store username.
            regex username_regex;
            try {
                username_regex = regex("\"success\":\\{\"username\":\"(.*)\"\\}");
            }
            catch (const regex_error &e) {
                logging->error("Tried to register app but couldn't regex username from response");
                return connectionUnknownError;
            }
            smatch match;
            const string s = registerResponse;
            if (regex_search(s.begin(), s.end(), match, username_regex)) {
                username = match[1];
            }
            else {
                logging->error("Tried to register app but couldn't match username from response");
                return connectionUnknownError;
            }
        }
    }
    assert(username.compare("") != 0);
    // At this point we should have a good connection. Check the light ID is correct.
    string lightCheckURL = "http://" + ip + "/api/" + username + "/lights/" + lightID + "/state";
    string lightCheckBody = "{\"on\": true, \"sat\" : 255, \"bri\" : 255, \"hue\" : 10000}";
    string lightCheckResponse;
    int lightCheckResult = sendMessage(lightCheckURL, lightCheckBody, "PUT", lightCheckResponse);
    if (lightCheckResponse.find("not available") != string::npos) {
        logging->warn("Communicating with hub, but lightID seems unavailable");
        return connectionBadID;
    }
    logging->info("Successfully connected to hub!");
    return connectionOK;
}

string Hue::getUsername() {
    return username;
}

void Hue::turnOn() {
    string url = "http://" + ip + "/api/" + username + "/lights/" + lightID + "/state";
    string body = "{\"on\": true}";
    string response;
    int result = sendMessage(url, body, "PUT", response);
    if (result != 0) {
        logging->error("Failed to turn on light!");
    }
}

void Hue::turnOff() {
    string url = "http://" + ip + "/api/" + username + "/lights/" + lightID + "/state";
    string body = "{\"on\": false}";
    string response;
    int result = sendMessage(url, body, "PUT", response);
    if (result != 0) {
        logging->error("Failed to turn off light!");
    }
}

// Given a COLORREF, will change light_id to this colour. Returns 0 on success.
int Hue::changeColourTo(COLORREF colour) {
    int success = 0;
    Point xy = convertColourToXY(colour);
    int brightness = convertColourToBrightness(colour);
    stringstream x, y;
    x << xy.x;
    y << xy.y;
    string returned_data;
    const string URL = "http://" + ip + "/api/" + username + "/lights/" + lightID + "/state";
    const string body = "{ \"xy\": [" + x.str() + "," + y.str() + "] }";
    success = sendMessage(URL, body, returned_data);
    if (returned_data.find("success") == string::npos) {
        success = -1;
        logging->error("changeColourTo failed. Response:" + returned_data);
    }
    return success;
}

// Takes RGB from a COLORREF, and returns XY values in the hue space.
// Now adapted from Philips' official example!
// https://github.com/PhilipsHue/PhilipsHueSDK-iOS-OSX/blob/master/ApplicationDesignNotes/RGB%20to%20xy%20Color%20conversion.md
Hue::Point Hue::convertColourToXY(COLORREF colour) {
    float red = (float)GetRValue(colour) / 255;
    float green = (float)GetGValue(colour) / 255;
    float blue = (float)GetBValue(colour) / 255;
    
    // Apply gamma correction
    red   = (red   > 0.04045f) ? pow((red   + 0.055f) / (1.0f + 0.055f), 2.4f) : (red   / 12.92f);
    green = (green > 0.04045f) ? pow((green + 0.055f) / (1.0f + 0.055f), 2.4f) : (green / 12.92f);
    blue  = (blue  > 0.04045f) ? pow((blue  + 0.055f) / (1.0f + 0.055f), 2.4f) : (blue  / 12.92f);

    // "Wide gamut conversion D65"
    float X = red * 0.649926f + green * 0.103455f + blue * 0.197109f;
    float Y = red * 0.234327f + green * 0.743075f + blue * 0.022598f;
    float Z = red * 0.000000f + green * 0.053077f + blue * 1.035763f;
    float cx = X / (X + Y + Z);
    float cy = Y / (X + Y + Z);
    if (isnan(cx)) {
        cx = 0.0f;
    }

    if (isnan(cy)) {
        cy = 0.0f;
    }

    //Check if the given XY value is within the colourreach of our lamps.
    Point xyPoint = Point(cx, cy);
    Point *lampColours = new Point[3]; // Holds the lamp colour triangle for standard Hue lamps
    lampColours[0].x = 0.674f; // red
    lampColours[0].y = 0.322f;
    lampColours[1].x = 0.408f; // green
    lampColours[1].y = 0.517f;
    lampColours[2].x = 0.168f; // blue
    lampColours[2].y = 0.041f;
    bool inReachOfLamp = checkPointInLampsReach(xyPoint, lampColours);
    if (!inReachOfLamp) {
        // If desired colour is out of reach, find closest colour.
        // First, find closest point on each line in the triangle.
        Point pAB = getClosestPointToPoints(lampColours[0], lampColours[1], xyPoint);
        Point pAC = getClosestPointToPoints(lampColours[2], lampColours[0], xyPoint);
        Point pBC = getClosestPointToPoints(lampColours[1], lampColours[2], xyPoint);

        float dAB = getDistanceBetweenTwoPoints(xyPoint, pAB);
        float dAC = getDistanceBetweenTwoPoints(xyPoint, pAC);
        float dBC = getDistanceBetweenTwoPoints(xyPoint, pBC);

        float lowest = dAB;
        Point closestPoint = pAB;
        if (dAC < lowest) {
            lowest = dAC;
            closestPoint = pAC;
        }
        if (dBC < lowest) {
            lowest = dBC;
            closestPoint = pBC;
        }
        xyPoint.x = closestPoint.x;
        xyPoint.y = closestPoint.y;
    }
    return Point(xyPoint.x, xyPoint.y);
}

// Checks to see if the given XY value is within the reach of the lamp
bool Hue::checkPointInLampsReach(Point p, Point *colourPoints) {
    Hue::Point red = colourPoints[0];
    Hue::Point green = colourPoints[1];
    Hue::Point blue = colourPoints[2];

    Point v1 = Point(green.x - red.x, green.y - red.y);
    Point v2 = Point(blue.x - red.x, blue.y - red.y);
    Point q = Point(p.x - red.x, p.y - red.y);

    float s = crossProduct(q, v2) / crossProduct(v1, v2);
    float t = crossProduct(v1, q) / crossProduct(v1, v2);

    if ((s >= 0.0f) && (t >= 0.0f) && (s + t <= 1.0f)) {
        return true;
    } else {
        return false;
    }
}

float Hue::getDistanceBetweenTwoPoints(Point one, Point two) {
    float dx = one.x - two.x;
    float dy = one.y - two.y;
    float dist = sqrt(dx * dx + dy * dy);
    return dist;
}

// Finds the closest point on a line, which we'll use to ensure the colour we choose is in range of our lamp.
Hue::Point Hue::getClosestPointToPoints(Point A, Point B, Point P) {
    Point AP = Point(P.x - A.x, P.y - A.y);
    Point AB = Point(B.x - A.x, B.y - A.y);

    float ab2 = AB.x * AB.x + AB.y * AB.y;
    float ap_ab = AP.x * AB.x + AP.y * AB.y;
    float t = ap_ab / ab2;

    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }

    return Point(A.x + AB.x * t, A.y + AB.y * t);
}

float Hue::crossProduct(Point one, Point two) {
    return one.x * two.y - one.y * two.x;
}

int Hue::convertColourToBrightness(COLORREF colour) {
    return 0;
}

Hue::~Hue()
{
}
