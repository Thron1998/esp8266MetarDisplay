// #include <SSD1306Ascii.h>

#include <Wire.h>
// #include "font.h"
#include "images.h"
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <WiFiClientSecure.h>
#include "wifiCreds.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

// Old font library
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// New font library
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// Function declaration
void scanForClientPhone(IPAddress addr);
int getMetarInfo(const char* airportCode, char* metarResult, char* conditionResult);
void displayMetarInfo(const char* airportCode, char* metarResult, char* conditionResult, int metarSize, int displayTextDelay);
void displayConditionCode(char* conditionResult, int line);
void printMetarInfoDebug(const char* airportCode, char* metarResult, char* conditionResult);
void setupOled();
void setOledSettings();
uint8_t setupWifi();
void displayStartupScreen();
void displayIpAddress();
bool getNextLine(char* metarResult, uint16_t* pointerToText, int metarSize);
void adjustContrastForTime();
int getCurrentHour();
void testScreen();
// void Wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info);
// void Get_IPAddress(WiFiEvent_t event, WiFiEventInfo_t info);

// Serial definitions
const int BAUD_RATE = 115200;

// Data definitions
const char DATA_END_SYMBOL = '%';

// NTP variables
const uint8_t HOUR_ERROR_CODE = 0;

// Phone definitions
IPAddress ipAddressPhone(192, 168, 2, 168);

// Time definitions
const long utcOffsetInSeconds = 0; // UTC time
const int HIGH_CONTRAST_HOUR_LOW = 6; // These times are in UTC
const int HIGH_CONTRAST_HOUR_HIGH = 18;

// Server definitions
#define SERVER "www.aviationweather.gov"
#define BASE_URI "/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString="
#define HTTPSPORT 443
#define DISPLAY_IP_ADDR_DELAY 2000
#define DATA_REFRESH_DELAY 10000

// Oled definitions
const uint8_t SCREEN_WIDTH = 128;
const uint8_t SCREEN_HEIGHT = 64;
const uint8_t OLED_RESET = 1;
const uint8_t OLED_ADDR = 0x3C;
const uint8_t SCL_PIN = 0;
const uint8_t SDA_PIN = 2;
const uint8_t MAX_OLED_LINES = 4;
const uint8_t LINE_RESET_VALUE = 1;
const uint8_t HIGH_CONTRAST = 255;
const uint8_t LOW_CONTRAST = 0;

// const uint8_t MAX_CHARACTER_COUNT = 20; // Max character count using font lcd5x7
// const int MAX_CHARACTER_COUNT = 9; // Max character count using font Arial14
const int MAX_CHARACTER_COUNT = 16; // Test value

const int DISPLAY_DATA_TIME = 5000;
const int DISPLAY_METAR_ROTATIONS = 5;
const uint8_t* DISPLAY_FONT = Arial14;
/* Fonts to use:
* Arial14
 * fixed_bold10x15
 * font8x8
 * TimesNewRoman16
 * Verdana12
 * X11fixed7x14
 * ZevvPeep8x16
*/

// Wifi event handler object
WiFiEventHandler disconnectedEventHandler;

// NTP object
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

// Wifi object
WiFiClientSecure client;

// Oled object
// Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SSD1306AsciiWire oledDisplay;

// const char allMyText[] PROGMEM = {"Het was al duidelijk dat mensen afgelopen vrijdag in groten getale zijn gaan tanken, " 
//                                     "omdat toen de brandstofprijzen fors omlaag gingen. De dag ervoor, met de oude hoge prijzen, " 
//                                     "was dat juist 22 procent minder dan normaal. Ook zaterdag 2 april was het aantal tankbeurten " 
//                                     "nog fors hoger, bijna 20 procent."};
const char allMyText[] PROGMEM ={"METAR EHAM 071655Z 28024G34KT 250V320 9999 -SHRA FEW020CB SCT045 "
                                 "06/04 Q0985 RESHRA TEMPO 26035G45KT 4000 SHRAGS="};