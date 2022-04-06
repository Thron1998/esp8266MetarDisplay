// Combined url: www.aviationweather.gov/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString=EHLE
// http://oleddisplay.squix.ch/#/home
// Website: https://javl.github.io/image2cpp/
// https://www.youtube.com/watch?v=QFubFhrEfmQ

// max 44 characters (* 2 so 88 characters max from top to bottom)

#include "main.h"
#include "errorCodes.h"

char reply[21];

// #define HOME_BASE_AIRPORT "EHLE"
const char* HOME_BASE_AIRPORT = "EHLE";

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
  int* metarSize;
    
  getMetarInfo(HOME_BASE_AIRPORT, metar, condition, metarSize); // Fetch data from server

  displayMetarInfo(HOME_BASE_AIRPORT, metar, condition, metarSize); // Show data on display

  printMetarInfoDebug(HOME_BASE_AIRPORT, metar, condition); // Show data on serial monitor

  delay(DATA_REFRESH_DELAY);
}

void displayMetarInfo(const char* airportCode, char* metarResult, char* conditionResult, int* metarSize) {
  int scroll = 0;
  int line = 0;
  static unsigned long* txtPointer = 0;
  
  bool dataAvailable = true;

  while (dataAvailable) {
    oledDisplay.clearToEOL();

    dataAvailable = getNextLine(metarResult, txtPointer);
    oledDisplay.println(reply);
    Serial.println(reply);
    
    if (line >= 4) {
      for (int i = 0; i < 8; ++i) {
        delay(50);
        oledDisplay.ssd1306WriteCmd(SSD1306_SETSTARTLINE | scroll % 64);
        ++scroll;
      }
    } else {
      delay(0);
    }

    if (oledDisplay.row() >= 7 && scroll >= 32) {
      oledDisplay.home();
    }

    if (scroll >= 64) {
      scroll = 0;
    }
    line++;
  }

  // No more data, clear screen start all over again
  delay(3000);
  oledDisplay.clear();
  oledDisplay.home();

  /*
  // oledDisplay.clearDisplay();
  oledDisplay.clear();

  // oledDisplay.setCursor(0,0);
  oledDisplay.print("---");
  oledDisplay.print(String(conditionResult));
  oledDisplay.println("---");

  // Print metar results
  oledDisplay.print(String(metarResult));

  // oledDisplay.display();
  */
}

// Get the next 20 characters (or up to space to prevent trunction)
bool getNextLine(char* metarResult, unsigned long* txtPointer) {

  // This
  // static unsigned long txtPointer = 0;
  static bool lineBreakInProgress = false;

  // Fill reply array with spaces
  for (int cnt = 0; cnt < 20; cnt++) {
    reply[cnt] = ' ';
  }

  if (lineBreakInProgress) {
    lineBreakInProgress = false;
    return true;
  }

  for (uint8_t cnt = 0; cnt < 20; cnt++) {

    if(*txtPointer < strlen(allMyText)) {
      char myChar = pgm_read_byte_near(allMyText + *txtPointer);

      // increment pointer before we return
      *txtPointer++;

      // Deal with special characters (here, just new lines)
      if (myChar == '%') {
        // Set the flag that we have a line-break situation
        lineBreakInProgress = true;
        return true;
      }

      reply[cnt] = myChar;
    } else {
      *txtPointer = 0;
      Serial.println("\nEnd of data!");
      return false;
    }
  }

  if (reply[19] == ' ') {
    Serial.println("Final char is a space");
    return true;
  }

  if (pgm_read_byte_near(allMyText + *txtPointer) == ' ') {
    Serial.println("Next char is a space");
    *txtPointer++;
    return true;
  }

  // Track back to last space
  for (uint8_t cnt = 18; cnt > 0; cnt--) {
    if (reply[cnt] == ' ') {

      Serial.print("Space found at char: ");
      Serial.println(cnt);
      // Space fill rest of line and decrement pointer for next line
      for (uint8_t cnt2 = cnt; cnt2 < 20; cnt2++) {
        reply[cnt2] = ' ';
        *txtPointer--;
      }

      // If the next character in the string (yet to be printed) is a space
      // increment the pointer so we don't start a line with a space
      if (pgm_read_byte_near(allMyText + *txtPointer) == ' ') {
        Serial.println("Next char is a space");
        *txtPointer++;
      }

      break;
    }
  }
  return true;
}

void getMetarInfo(const char* airportCode, char* metarResult, char* conditionResult, int* metarSize) {
  const char* airportString = airportCode;
  char c;

  String currentCondition = "";
  String currentMetarRaw = "";
  String currentLine = "";
  int currentMetarCount = 0;
      
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

    // Get metar length
    currentMetarCount = currentMetarRaw.length();

    // Debug values
    Serial.println("Received values");
    Serial.println("Raw metar: " + currentMetarRaw);
    Serial.println("Flight conditions: " + currentCondition);
    Serial.println("Metar count: " + currentMetarCount);

    // Save results
    strcpy(metarResult, currentMetarRaw.c_str());
    strcpy(conditionResult, currentCondition.c_str());    
  }  
}

void displayStartupScreen() {
  // oledDisplay.clearDisplay();
  oledDisplay.clear();

  oledDisplay.println("Metar info");
  oledDisplay.println(HOME_BASE_AIRPORT);
  
  // oledDisplay.display();    
}

void displayIpAddress() {
  // oledDisplay.clearDisplay();
  oledDisplay.clear();
  
  oledDisplay.setCursor(0,0);
  oledDisplay.print(WiFi.localIP());

  // oledDisplay.display();

  // Debug
  Serial.print("IP addres: ");
  Serial.println(WiFi.localIP());
}

void setOledSettings() {
  // Set display standards
  oledDisplay.setFont(Arial14);
  
  // oledDisplay.setRotation(0); // For debugging
  // oledDisplay.setRotation(2); // Rotate screen 90 degrees (for final product)
}

void setupOled() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000L);

  // oledDisplay.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  oledDisplay.begin(&Adafruit128x64, OLED_ADDR, OLED_RESET);

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
  Serial.print('\n');

  return WiFi.status(); // Return status after setup
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

void testScreen() {
  // Test display!
  /*
  Wire.begin(SDA_PIN, SCL_PIN);
  oledDisplay.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

  oledDisplay.setFont(&Dialog_plain_9);
  oledDisplay.setFont(&Dialog_plain_12);

  oledDisplay.setTextColor(WHITE);
  oledDisplay.setRotation(0); // For debugging
  */
}