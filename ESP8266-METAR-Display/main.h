// #include <SSD1306Ascii.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "font.h"
#include "images.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "wifiCreds.h"

// Function declaration
void getMetarInfo(const char* airportCode, char* metarResult, char* conditionResult);
void displayMetarInfo(const char* airportCode, char* metarResult, char* conditionResult);
void printMetarInfoDebug(const char* airportCode, char* metarResult, char* conditionResult);
void setupOled();
void setOledSettings();
uint8_t setupWifi();
void displayStartupScreen();
void displayIpAddress();
void testScreen();
void scrollText();
bool getNextLine();

// Data definitions
#define DATA_END_SYMBOL '%'

// Server definitions
#define SERVER "www.aviationweather.gov"
#define BASE_URI "/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString="
#define HTTPSPORT 443
#define DISPLAY_IP_ADDR_DELAY 2000
#define DATA_REFRESH_DELAY 10000

// Oled definitions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 1
#define OLED_ADDR 0x3C
#define SCL_PIN 0
#define SDA_PIN 2

// Wifi object
WiFiClientSecure client;

// Oled object
Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
