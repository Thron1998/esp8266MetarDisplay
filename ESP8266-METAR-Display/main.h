// #include <SSD1306Ascii.h>

#include <Wire.h>
// #include "font.h"
#include "images.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "wifiCreds.h"

// Old font library
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// New font library
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// Function declaration
void getMetarInfo(const char* airportCode, char* metarResult, char* conditionResult, int* metarSize);
void displayMetarInfo(const char* airportCode, char* metarResult, char* conditionResult, int* metarSize);
void printMetarInfoDebug(const char* airportCode, char* metarResult, char* conditionResult);
void setupOled();
void setOledSettings();
uint8_t setupWifi();
void displayStartupScreen();
void displayIpAddress();
bool getNextLine(char* metarResult, uint16_t* pointerToText);
void testScreen();

// Serial definitions
const int BAUD_RATE = 115200;

// Data definitions
const char DATA_END_SYMBOL = '%';

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
const int MAX_OLED_LINES = 4;
const int DISPLAY_DATA_TIME = 5000;
const int MAX_CHARACTER_COUNT = 20;
/* Fonts to use:
* Arial14
 * fixed_bold10x15
 * font8x8
 * TimesNewRoman16
 * Verdana12
 * X11fixed7x14
 * ZevvPeep8x16
*/

// Wifi object
WiFiClientSecure client;

// Oled object
// Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SSD1306AsciiWire oledDisplay;

const char allMyText[] PROGMEM = {"Het was al duidelijk dat mensen afgelopen vrijdag in groten getale zijn gaan tanken, " 
                                    "omdat toen de brandstofprijzen fors omlaag gingen. De dag ervoor, met de oude hoge prijzen, " 
                                    "was dat juist 22 procent minder dan normaal. Ook zaterdag 2 april was het aantal tankbeurten " 
                                    "nog fors hoger, bijna 20 procent."};