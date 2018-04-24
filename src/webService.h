#ifndef _webService_h_
#define _webService_h_

#include <SPI.h>
#include <WiFi101.h>
#include <WiFi101OTA.h>
#include <WiFiMDNSResponder.h>

#include "main.cpp"

void checkForClients();
boolean setUpWebService();
void SetSettings(void);
void SetLEDs(void);
char StrContains(char *str, char *sfind);
void StrClear(char *str, char length);
void XML_response(WiFiClient cl);

char mdnsName[] = "huey"; // the MDNS name that the board will respond to
// after WiFi settings have been provisioned
// Note that the actual MDNS name will have '.local' after
// the name above, so "wifi101" will be accessible on
// the MDNS name "wifi101.local".

// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   100
File webFile;               // the web page file on the SD card
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer
boolean LED_state[2] = {0}; // stores the states of the LEDs
int runs = 0;

WiFiServer server(80);

// Create a MDNS responder to listen and respond to MDNS name requests.
WiFiMDNSResponder mdnsResponder;

boolean setUpWebService(){
  server.begin();

  // Setup the MDNS responder to listen to the configured name.
  // NOTE: You _must_ call this _after_ connecting to the WiFi network and
  // being assigned an IP address.
  if (!mdnsResponder.begin(mdnsName)) {
    Serial.println("Failed to start MDNS responder!");
    while(1);
  }

  Serial.print("Server listening at http://");
  Serial.print(mdnsName);
  Serial.println(".local/");
  return true;
}

void checkForClients(){
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {  // got client?
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {   // client data available to read
        char c = client.read(); // read 1 byte (character) from client
        // limit the size of the stored received HTTP request
        // buffer first part of HTTP request in HTTP_req array (string)
        // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
        if (req_index < (REQ_BUF_SZ - 1)) {
          HTTP_req[req_index] = c;          // save HTTP request character
          req_index++;
        }
        // last line of client request is blank and ends with \n
        // respond to client only after last line received
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          // remainder of header follows below, depending on if
          // web page or XML page is requested
          // Ajax request - send XML file
          if (StrContains(HTTP_req, "ajax_inputs")) {
            // send rest of HTTP header
            client.println("Content-Type: text/xml");
            client.println("Connection: keep-alive");
            client.println();
            // SetLEDs();
            SetSettings();
            // send XML file containing input states
            XML_response(client);
            if (runs == 0) {
              Serial.print("runs: ");
              Serial.println(runs);
              runs = runs + 1;
              break;
            }
          } else
          {  // web page request
            // send rest of HTTP header
            client.println("Content-Type: text/html");
            client.println("Connection: keep-alive");
            client.println();
            // send web page
            webFile = SD.open("index.htm");        // open web page file
            if (webFile) {
              while(webFile.available()) {
                client.write(webFile.read()); // send web page to client
              }
              webFile.close();
            }

            runs = 0;
          }
          // display received HTTP request on serial port
          Serial.print(HTTP_req);
          // reset buffer index and all buffer elements to 0
          req_index = 0;
          StrClear(HTTP_req, REQ_BUF_SZ);

          break;
        }
        // every line of text received from the client ends with \r\n
        if (c == '\n') {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // a text character was received from client
          currentLineIsBlank = false;
        }
      } // end if (client.available())
    } // end while (client.connected())
    delay(2);      // give the web browser time to receive the data
    client.stop(); // close the connection
  } // end if (client)

  // read buttons and debounce
  //ButtonDebounce();
}

void SetSettings(void){
  // LED 1 (pin 6)
  if (StrContains(HTTP_req, "LED1=1")) {
    LED_state[0] = 1;  // save LED state
    // digitalWrite(6, HIGH);
    strip.setPixelColor(0, 100, 100, 100);
    strip.show();
  }
  else if (StrContains(HTTP_req, "LED1=0")) {
    LED_state[0] = 0;  // save LED state
    // digitalWrite(6, LOW);
    strip.setPixelColor(0, 0, 0, 0);
    strip.show();
  }
  // LED 2 (pin 7)
  if (StrContains(HTTP_req, "LED2=1")) {
    LED_state[1] = 1;  // save LED state
    // digitalWrite(7, HIGH);
    strip.setPixelColor(1, 100, 100, 100);
    strip.show();
  }
  else if (StrContains(HTTP_req, "LED2=0")) {
    LED_state[1] = 0;  // save LED state
    // digitalWrite(7, LOW);
    strip.setPixelColor(1, 0, 0, 0);
    strip.show();
  }

  // Add the final 0 to end the C string
  HTTP_req[REQ_BUF_SZ] = 0;

  Serial.print(HTTP_req);

  // Read each command pair
  char* command = strtok(HTTP_req, "&");
  while (command != 0)
  {
    // Split the command in two values
    char* separator = strchr(command, '=');

    if (separator != 0)
    {
      // Actually split the string in 2: replace ':' with 0
      *separator = 0;
      // int servoId = atoi(command);
      ++separator;

      // Do something with servoId and position
      if (StrContains(command, "latitude")){
        // lat = separator;
        strcpy(lat, separator);
        Serial.println(String("lat: ") + lat);
      }
      if (StrContains(command, "longitude")) {
        // lon = separator;
        strcpy(lon, separator);
        Serial.println(String("lon: ") + lon);
      }
      if (StrContains(command, "dayBrightness")){
        int value = atoi(separator);
        prevDefaultBrightness = defaultBrightness;
        defaultBrightness = value / 100.0;
        settingBrightness = true;
        Serial.println(String("defaultBrightness: ") + defaultBrightness);
      }
      if (StrContains(command, "nightBrightness")) {
        int value = atoi(separator);
        prevSunsetBrightness = sunsetBrightness;
        sunsetBrightness = value / 100.0;
        settingBrightness = true;
        Serial.println(String("sunsetBrightness: ") + sunsetBrightness);
      }
    }
    // Find the next command in input string
    command = strtok(0, "&");
  }
}

// checks if received HTTP request is switching on/off LEDs
// also saves the state of the LEDs
void SetLEDs(void)
{
  // LED 1 (pin 6)
  if (StrContains(HTTP_req, "LED1=1")) {
    LED_state[0] = 1;  // save LED state
    // digitalWrite(6, HIGH);
    strip.setPixelColor(0, 100, 100, 100);
  }
  else if (StrContains(HTTP_req, "LED1=0")) {
    LED_state[0] = 0;  // save LED state
    // digitalWrite(6, LOW);
    strip.setPixelColor(0, 0, 0, 0);
  }
  // LED 2 (pin 7)
  if (StrContains(HTTP_req, "LED2=1")) {
    LED_state[1] = 1;  // save LED state
    // digitalWrite(7, HIGH);
    strip.setPixelColor(1, 100, 100, 100);
  }
  else if (StrContains(HTTP_req, "LED2=0")) {
    LED_state[1] = 0;  // save LED state
    // digitalWrite(7, LOW);
    strip.setPixelColor(1, 0, 0, 0);
  }

  strip.show();
}

// send the XML file with analog values, switch status
//  and LED status
void XML_response(WiFiClient cl)
{
  int analog_val;            // stores value read from analog inputs
  int count;                 // used by 'for' loops
  int sw_arr[] = {2, 3};  // pins interfaced to switches

  cl.print("<?xml version = \"1.0\" ?>");
  cl.print("<inputs>");
  // checkbox LED states
  // LED1
  cl.print("<LED>");
  if (LED_state[0]) {
    cl.print("checked");
  }
  else {
    cl.print("unchecked");
  }
  cl.println("</LED>");
  // button LED states
  // LED3
  cl.print("<LED>");
  if (LED_state[1]) {
    cl.print("on");
  }
  else {
    cl.print("off");
  }
  cl.println("</LED>");
  cl.print("</inputs>");
}

// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
  for (int i = 0; i < length; i++) {
    str[i] = 0;
  }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
  char found = 0;
  char index = 0;
  char len;

  len = strlen(str);

  if (strlen(sfind) > len) {
    return 0;
  }
  while (index < len) {
    if (str[index] == sfind[found]) {
      found++;
      if (strlen(sfind) == found) {
        return 1;
      }
    }
    else {
      found = 0;
    }
    index++;
  }

  return 0;
}

// function reads the push button switch states, debounces and latches the LED states
// toggles the LED states on each push - release cycle
// hard coded to debounce two switches on pins 2 and 3; and two LEDs on pins 6 and 7
// function adapted from Arduino IDE built-in example:
// File --> Examples --> 02.Digital --> Debounce
// void ButtonDebounce(void)
// {
//   static byte buttonState[2]     = {LOW, LOW};   // the current reading from the input pin
//   static byte lastButtonState[2] = {LOW, LOW};   // the previous reading from the input pin
//
//   // the following variables are long's because the time, measured in miliseconds,
//   // will quickly become a bigger number than can be stored in an int.
//   static long lastDebounceTime[2] = {0};  // the last time the output pin was toggled
//   long debounceDelay = 50;         // the debounce time; increase if the output flickers
//
//   byte reading[2];
//
//   reading[0] = digitalRead(2);
//   reading[1] = digitalRead(3);
//
//   for (int i = 0; i < 2; i++) {
//     if (reading[i] != lastButtonState[i]) {
//       // reset the debouncing timer
//       lastDebounceTime[i] = millis();
//     }
//
//     if ((millis() - lastDebounceTime[i]) > debounceDelay) {
//       // whatever the reading is at, it's been there for longer
//       // than the debounce delay, so take it as the actual current state:
//
//       // if the button state has changed:
//       if (reading[i] != buttonState[i]) {
//         buttonState[i] = reading[i];
//
//         // only toggle the LED if the new button state is HIGH
//         if (buttonState[i] == HIGH) {
//           LED_state[i] = !LED_state[i];
//         }
//       }
//     }
//   } // end for() loop
//
//   // set the LEDs
//   digitalWrite(6, LED_state[0]);
//   digitalWrite(7, LED_state[1]);
//
//   // save the reading.  Next time through the loop,
//   // it'll be the lastButtonState:
//   lastButtonState[0] = reading[0];
//   lastButtonState[1] = reading[1];
// }


#endif
