// Combined url: www.aviationweather.gov/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString=EHLE
// http://oleddisplay.squix.ch/#/home
// Website: https://javl.github.io/image2cpp/
// https://www.youtube.com/watch?v=QFubFhrEfmQ

// max 44 characters (* 2 so 88 characters max from top to bottom)

#include "main.h"
#include "errorCodes.h"

#define HOME_BASE_AIRPORT "EHLE"

void setup() {
  setupOled();
  delay(100);
 
  Serial.begin(115200);

  setupWifi();

  displayIpAddress();
  delay(DISPLAY_IP_ADDR_DELAY);
}

void loop() {
  char metar[500], condition[6];
    
  getMetarInfo(HOME_BASE_AIRPORT, metar, condition); // Fetch data from server

  displayMetarInfo(HOME_BASE_AIRPORT, metar, condition); // Show data on display

  printMetarInfoDebug(HOME_BASE_AIRPORT, metar, condition); // Show data on serial monitor

  delay(DATA_REFRESH_DELAY);
}

void displayMetarInfo(const char* airportCode, char* metarResult, char* conditionResult) {
  oledDisplay.clearDisplay();

  int i = 0;
  bool dataAvailable = true;

  oledDisplay.setCursor(0,0);
  oledDisplay.print(String(metarResult));

  // TODO: handle result data for printing correctly

  /*
  oledDisplay.setCursor(0,0);
  oledDisplay.print(airportCode);
  oledDisplay.print(" ");
  oledDisplay.println(String(conditionResult));

  // Print metar results
  oledDisplay.print(String(metarResult));
  */

  oledDisplay.display();
}


void scrollText() {

}

bool getNextLine() {

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

    // --------------
    // TODO: add end character to end of string received

    // --------------

    // Debug values
    Serial.println("Received values");
    Serial.println("Raw metar: " + currentMetarRaw);
    Serial.println("Flight conditions: " + currentCondition);

    // Save results
    strcpy(metarResult, currentMetarRaw.c_str());
    strcpy(conditionResult, currentCondition.c_str());    
  }  
}

void displayStartupScreen() {
  oledDisplay.println("Metar info");
  oledDisplay.println(HOME_BASE_AIRPORT);

  // Display airplane icon (generated size: 30 x 30 px)
  // oledDisplay.drawBitmap(0, 0, epd_bitmap_airplane, 30, 30, WHITE);
  
  oledDisplay.display();    
}

void displayIpAddress() {
  oledDisplay.clearDisplay();
  
  oledDisplay.setCursor(0, 0);
  oledDisplay.println(WiFi.localIP());
  oledDisplay.display();
  delay(DISPLAY_IP_ADDR_DELAY); // Check for delay, IP address doesn't show up

  oledDisplay.clearDisplay();
  oledDisplay.display();
  
  // Debug
  Serial.print("IP addres: ");
  Serial.println(WiFi.localIP());
}

void setOledSettings() {
  // Set display standards
  // oledDisplay.setFont(&Dialog_plain_9);
  oledDisplay.setFont(&Dialog_plain_12);

  oledDisplay.setTextColor(WHITE);
  oledDisplay.setRotation(2); // Rotate screen 90 degrees
}

void setupOled() {
  Wire.begin(SDA_PIN, SCL_PIN);
  oledDisplay.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

  setOledSettings();
  displayStartupScreen(); 
}

uint8_t setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  return WiFi.status(); // Return status after setup
}

void testScreen() {
  oledDisplay.clearDisplay();

  for(int16_t i=0; i<oledDisplay.height()/2; i+=2) {
    oledDisplay.drawRect(i, i, oledDisplay.width()-2*i, oledDisplay.height()-2*i, SSD1306_WHITE);
    oledDisplay.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }

  delay(2000);
}

void printMetarInfoDebug(const char* airportCode, char* metarResult, char* conditionResult) {
  int i;
  
  i = 0;
  Serial.print('\n');
  while(i < sizeof(metarResult) || metarResult[i] != '\0') {
    Serial.print(metarResult[i]);

    if(metarResult[i] == '\0') {
      break;
    }

    i++;
  }

  i = 0;
  Serial.print('\n');
  while(i < sizeof(conditionResult) || conditionResult[i] != '\0') {
    Serial.print(conditionResult[i]);
    
    if(conditionResult[i] == '\0') {
      break;
    }

    i++;
  }  
}
