// Combined url: www.aviationweather.gov/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString=EHLE

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// Function declaration
void getMetarInfo(const char* airportCode, char* metarResult, char* conditionResult);

#define SERVER "www.aviationweather.gov"
#define BASE_URI "/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString="
#define HTTPSPORT 443
#define OLED_RESET 1

const char* ssid = "mesh_21";
const char* pass = "ap69ju71ju98de00ap05";

WiFiClientSecure client;
Adafruit_SSD1306 display(OLED_RESET);

#if(SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP addres: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, LOW);

  Wire.begin(2, 0); // Set I2C pins (SDA = GPIO2, SCL = GPIO0), default clock 100kHz
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display.display();
}

void loop() {
  char metar[500], condition[6];
  int i = 0;
  
  getMetarInfo("EHLE", metar, condition);

  i = 0;
  Serial.print('\n');
  while(i < sizeof(metar) || metar[i] != '\0') {
    Serial.print(metar[i]);

    if(metar[i] == '\0') {
      break;
    }

    i++;
  }

  i = 0;
  Serial.print('\n');
  while(i < sizeof(condition) || condition[i] != '\0') {
    Serial.print(condition[i]);
    
    if(condition[i] == '\0') {
      break;
    }

    i++;
  }

  delay(2000);
}

void getMetarInfo(const char* airportCode, char* metarResult, char* conditionResult) {
  const char* airportString = airportCode;
  char c;

  String currentCondition = "";
  String currentMetarRaw = "";
  String currentLine = "";
      
  boolean readingCondition = false;
  boolean readingMetarRaw = false;  

  client.setInsecure();
  Serial.println("Starting connection to server...");
  if(!client.connect(SERVER, HTTPSPORT)) {
    Serial.println("Connection failed");
    // Handle failed connection
  } else {
    // Make a HTTP request, and print it to console:
    Serial.println("Connected ...");
    Serial.print("GET ");
    Serial.print(BASE_URI);
    Serial.print(airportString);
    Serial.println(" HTTP/1.1");
    Serial.print("Host: ");
    Serial.println(SERVER);
    Serial.println("User-Agent: LED Map Client");
    Serial.println("Connection: close");
    Serial.println();
  
    client.print("GET ");
    client.print(BASE_URI);
    client.print(airportString);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(SERVER);
    client.println("User-Agent: LED Sectional Client");
    client.println("Connection: close");
    client.println();
    client.flush();

    while(client.connected()) {
      c = client.read();
      if (c >= 0) {
        yield();
        currentLine += c;
        if (c == '\n') currentLine = "";

        if (currentLine.endsWith("<raw_text>")) {
          readingMetarRaw = true;
        } else if (readingMetarRaw) {
          if (!currentLine.endsWith("<")) {
            currentMetarRaw += c; 
          } else {
            currentMetarRaw += '\0';
            readingMetarRaw = false;
          }
        } else if (currentLine.endsWith("<flight_category>")) {
          readingCondition = true;
        } else if (readingCondition) {
          if (!currentLine.endsWith("<")) {
            currentCondition += c;
          } else {
            currentCondition += '\0';
            readingCondition = false;
          }
        }
      }
    }

    // Debug values
    Serial.println("Received values");
    Serial.println("Raw metar: " + currentMetarRaw);
    Serial.println("Flight conditions: " + currentCondition);

    // Save results
    strcpy(metarResult, currentMetarRaw.c_str());
    strcpy(conditionResult, currentCondition.c_str());    
  }  
}

/*
void loop() {
  WiFiClientSecure client;
  String airportString = "EHLE";
  String currentLine = "";
  bool readingMetar = false;
  String metarMessage = "";
  
  client.setInsecure();

  Serial.println("Starting connection to server...");
  if(!client.connect(SERVER, HTTPSPORT)) {
    Serial.println("Connection failed");
  } else {
    Serial.println("Connected ...");
    Serial.print("GET ");
    Serial.print(BASE_URI);
    Serial.print(airportString);
    Serial.println(" HTTP/1.1");
    Serial.print("Host: ");
    Serial.println(SERVER);
    Serial.println("User-Agent: LED Map Client");
    Serial.println("Connection: close");
    Serial.println();
    // Make a HTTP request, and print it to console:
    client.print("GET ");
    client.print(BASE_URI);
    client.print(airportString);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(SERVER);
    client.println("User-Agent: LED Sectional Client");
    client.println("Connection: close");
    client.println();
    client.flush();

    // ---------------
    // -- Grab data --
    // ---------------
    
    char c;
    while(client.connected()) {
      c = client.read();
      
      if (c >= 0) {
        yield();
        currentLine += c;

        if(c == '\n') {
          currentLine = "";
        }

        if(currentLine.endsWith("<raw_text>")) { // Start indiciation for raw metar information
          readingMetar = true;
        } else if (readingMetar) {
          if(!currentLine.endsWith("<")) {
            metarMessage += c;
          } else {
            readingMetar = false;
          }
        }
        
      } // End of message

      
      Serial.println("Received metar message: ");
      Serial.println(metarMessage);
      client.stop(); // Kill connection
    }
  }
  
  delay(5000);
}
*/
