#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define SERVER "www.aviationweather.gov"
#define BASE_URI "/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString="
#define HTTPSPORT 443

// Combined url: www.aviationweather.gov/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString=EHLE

//const char* nwsServer = "https://tgftp.nsw.noaa.gov";
//const char* metarHostEHLE = "https://tgftp.nws.noaa.gov/data/observations/metar/stations/EHLE.TXT";
//const char* fingerprint = "4179182b809f3ba376cba6ec069d44bee7b4f0ba";


const char* ssid = "mesh_21";
const char* pass = "ap69ju71ju98de00ap05";

WiFiClientSecure client;

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
}

void loop() {
  WiFiClientSecure client;
  String airportString = "EHLE";

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

    char c;

    String currentAirport = "";
    String currentCondition = "";
    String currentLine = "";
    String currentWind = "";
    String currentGusts = "";
    String currentWxstring = "";
    String currentMetarRaw = "";
    String airportString = "";
      
    boolean readingAirport = false;
    boolean readingCondition = false;
    boolean readingWind = false;
    boolean readingGusts = false;
    boolean readingWxstring = false;
    boolean readingMetarRaw = false;

    while(client.connected()) {
      c = client.read();
      if (c >= 0) {
        yield();
        currentLine += c;
        if (c == '\n') currentLine = "";

        if(currentLine.endsWith("<raw_text>")) {
          readingMetarRaw = true;
        } else if (readingMetarRaw) {
          if(!currentLine.endsWith("<")) {
            currentMetarRaw += c; 
          } else {
            readingMetarRaw = false;
          }
        }
      }
    }

    // Debug values
    Serial.println("Received values");
    Serial.println("Raw metar: " + currentMetarRaw);

    delay(5000);
    
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
