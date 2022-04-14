// Combined url: www.aviationweather.gov/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString=EHLE
// http://oleddisplay.squix.ch/#/home
// Website: https://javl.github.io/image2cpp/
// https://www.youtube.com/watch?v=QFubFhrEfmQ

// max 44 characters (* 2 so 88 characters max from top to bottom)

#include "main.h"
#include "errorCodes.h"

// Buffer to print line on oled display
char reply[21];

const char* HOME_BASE_AIRPORT = "EHLE";

// Enable serial output showing code sequence
const bool SERIAL_DEBUG_SEQUENCE_OUTPUT = false;

void setup() {
  setupOled();
 
  Serial.begin(BAUD_RATE);

  setupWifi();

  displayIpAddress();
  delay(DISPLAY_IP_ADDR_DELAY);
}

void loop() {
  char metar[500], condition[6];
  int metarSize;
    
  metarSize = getMetarInfo(HOME_BASE_AIRPORT, metar, condition); // Fetch data from server

  for(int i = 0; i < DISPLAY_METAR_ROTATIONS; i++) {
    displayMetarInfo(HOME_BASE_AIRPORT, metar, condition, metarSize, DISPLAY_DATA_TIME); // Show data on display

    printMetarInfoDebug(HOME_BASE_AIRPORT, metar, condition); // Show data on serial monitor
  }
}

void displayMetarInfo(const char* airportCode, char* metarResult, char* conditionResult, int metarSize, int displayTextDelay) {
  /*
   * Code is written for a 128x32 oled display. Rewrite code
   * for 128x64 oled display for it to work
   * 
   * Idea: fetch data from server, split data into packs of 128x64 pixels (4 rows) and display one by one
   * To make more fluent: cut all packets in half, this will refresh the page with 2 rows at a time
   * 
   * The 128×64 OLED screen displays all the contents of RAM whereas 128×32 OLED screen displays only 4 pages (half content) of RAM.
   * https://lastminuteengineers.com/oled-display-arduino-tutorial/
   * Datasheet 1306
   * https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
   * Datasheet 128x64 oled
   * https://www.vishay.com/docs/37902/oled128o064dbpp3n00000.pdf
   */

  uint8_t line = LINE_RESET_VALUE;

  // Pointer for index in text
  uint16_t pointerToText = 0;
  uint16_t* pointerToTextPointer = &pointerToText;

  if(SERIAL_DEBUG_SEQUENCE_OUTPUT)
    Serial.println("1");
  
  bool dataAvailable = true;

  // Reset display
  oledDisplay.clear();
  oledDisplay.home();

  // Display weather condition code
  displayConditionCode(conditionResult, line);
  line++;

  while (dataAvailable) { // Data handler loop
    oledDisplay.clearToEOL();

    if(SERIAL_DEBUG_SEQUENCE_OUTPUT)
      Serial.println("2");

    dataAvailable = getNextLine(metarResult, pointerToTextPointer, metarSize); // Grab new line, adjusted to display width

    if(SERIAL_DEBUG_SEQUENCE_OUTPUT)
      Serial.println("9");
    
    Serial.print("{line ");
    Serial.print(line);
    Serial.print("} -- ");
    Serial.print("display reply: ");
    Serial.println(reply);    

    oledDisplay.println(reply);
    
    if (line >= MAX_OLED_LINES) { // End of display is reached. Load new data into RAM

      if(SERIAL_DEBUG_SEQUENCE_OUTPUT)    
        Serial.println("10");

      line = LINE_RESET_VALUE; // Reset lines

      delay(displayTextDelay); // Delay current display state

      oledDisplay.home();
      oledDisplay.clear(); // Reset display for new data
    } else {
      line++;
    }
  }

  delay(displayTextDelay); // Add delay so last array of data doesn't dissapear instantly
}

// Get the next 20 characters (or up to space to prevent trunction)
bool getNextLine(char* metarResult, uint16_t* pointerToText, int metarSize) {
  // Serial.print("Pointer value: ");
  // Serial.println((int)*pointerToText);

  if(SERIAL_DEBUG_SEQUENCE_OUTPUT)
    Serial.println("3");

  static bool lineBreakInProgress = false;

  // Fill reply array with spaces
  for (uint8_t cnt = 0; cnt < MAX_CHARACTER_COUNT; cnt++) {
    reply[cnt] = ' ';
  }

  if (lineBreakInProgress) {
    lineBreakInProgress = false;
    return true;
  }

  for (uint8_t cnt = 0; cnt < MAX_CHARACTER_COUNT; cnt++) {
    
    if(*pointerToText < metarSize) {
      char myChar = metarResult[*pointerToText];

      // increment pointer before we return
      (*pointerToText)++;
      
      if(SERIAL_DEBUG_SEQUENCE_OUTPUT)
        Serial.println("4");
      
      // Deal with special characters (here, just new lines)
      if (myChar == DATA_END_SYMBOL) {
        // Set the flag that we have a line-break situation
        lineBreakInProgress = true;
        return true;
      }

      reply[cnt] = myChar;
    } else {

      if(SERIAL_DEBUG_SEQUENCE_OUTPUT)  
        Serial.println("5");

      *pointerToText = 0;
      return false; // End of data, return dataAvailable = false
    }
  }

  if (reply[(MAX_CHARACTER_COUNT-1)] == ' ') {
    return true; // Final char is a space
  }

  // if (pgm_read_byte_near(allMyText + *pointerToText) == ' ') {
  if (metarResult[*pointerToText] == ' ') {
    
    if(SERIAL_DEBUG_SEQUENCE_OUTPUT)
      Serial.println("6");

    (*pointerToText)++;
    return true; // Next char is a space
  }

  // Track back to last space
  for (uint8_t cnt = (MAX_CHARACTER_COUNT-2); cnt > 0; cnt--) {
    if (reply[cnt] == ' ') {

      // Space fill rest of line and decrement pointer for next line
      for (uint8_t cnt2 = cnt; cnt2 < MAX_CHARACTER_COUNT; cnt2++) {
        reply[cnt2] = ' ';
        
        if(SERIAL_DEBUG_SEQUENCE_OUTPUT)
          Serial.println("7");
        
        (*pointerToText)--;
      }

      // If the next character in the string (yet to be printed) is a space
      // increment the pointer so we don't start a line with a space
      if (metarResult[*pointerToText] == ' ') {
        
        if(SERIAL_DEBUG_SEQUENCE_OUTPUT)
          Serial.println("8");

        (*pointerToText)++; // Next char is a space
      }

      break; // Last space was found, break from for loop
    }
  }
  return true;
}

void displayConditionCode(char* conditionResult, int line) {
  Serial.print("{line ");
  Serial.print(line);
  Serial.print("} -- ");
  Serial.print("display reply: ");
  Serial.println(conditionResult);

  oledDisplay.print("--- ");
  oledDisplay.print(conditionResult);
  oledDisplay.println(" ---");
}

int getMetarInfo(const char* airportCode, char* metarResult, char* conditionResult) {
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

    // Debug values
    Serial.println("Received values");
    Serial.println("Raw metar: " + currentMetarRaw);
    Serial.println("Flight conditions: " + currentCondition);
    Serial.println("Metar count: " + currentMetarCount);

    // Save results
    // Serial.println("Metarsize pointer");
    strcpy(metarResult, currentMetarRaw.c_str());
    strcpy(conditionResult, currentCondition.c_str());
    // Serial.println("End of strcpy");

    // *metarSize = currentMetarCount; // TODO, fix this action, crashes esp8266

    // Get metar string length
    currentMetarCount = currentMetarRaw.length();

    return currentMetarCount;
  }  
}

void displayStartupScreen() {
  // oledDisplay.clearDisplay();
  oledDisplay.clear();

  oledDisplay.println("Metar info:");
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
  oledDisplay.setFont(DISPLAY_FONT);
  
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

  delay(100);
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

// --------------------
// TEST SECTION
// --------------------

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

// void testPointerValues(int* pointer);

// int value = 0;
// int* p = &value;

// void testPointerValues(int* pointer) {
//   (*pointer)++;
// }

  // Serial.print("*p: ");
  // Serial.println((int)*p);
  // Serial.print("&p: ");
  // Serial.println((int)&p);
  // Serial.print("p: ");
  // Serial.println((int)p);

  // testPointerValues(p);

  // delay(1000);

// ------------------
// Old main
// ------------------
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