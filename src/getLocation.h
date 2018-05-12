#ifndef _getLocation_H_
#define _getLocation_H_

#include <string.h>

boolean getLocation();

//location
char* lat = "37.8267";
char* lon = "-122.4233";

boolean getLocation(IPAddress IP){
  //location by ip freegeoip.net/json/173.53.85.225
  Serial.println("---------");
  Serial.println("Getting Location from IP...");
  const char locationHost[] = "ipinfo.io";
  String ipStr = String(IP[0])+"."+String(IP[1])+"."+String(IP[2])+"."+String(IP[3]);
  String locationURL = String("/json");

  WiFiClient httpClient;
  // TextFinder httpFinder(httpClient, 20);//finder and wait time in seconds
  Serial.print(F("connecting to "));
  Serial.println(locationHost);
  const int httpPort = 80;

  if (!httpClient.connect(locationHost, httpPort)) {
    Serial.println("connection failed");
    httpClient.flush();
    httpClient.stop();
    return false;
  }

  Serial.print(F("requesting URL: "));
  Serial.println(locationURL);

  // send the HTTP GET request:
  httpClient.println(String("GET ") + locationURL + " HTTP/1.1");
  httpClient.println(String("Host: ") + locationHost);
  httpClient.println("Connection: close");
  httpClient.println();

  Serial.println(F("request sent"));

  delay(250);
  // Serial.println(httpFinder.findUntil((char *)"{", (char *)"\n\r"));
  // Serial.println(httpFinder.findUntil((char *)" ", (char *)","));
  // char ipBuf[20];
  // httpFinder.getString((char *)"\"", (char *)"\"", ipBuf, 20);
  // Serial.println(ipBuf);

  // if (!httpFinder.findUntil("loc:", "}")){
  //   Serial.println("couldnt find your precious 'loc:'");
  // }
  // char latBuf[15];
  // httpFinder.getString((char *)"loc\": \"", (char *)",", latBuf, 15);
  // strcpy(lat, latBuf);
  // Serial.println(String(lat));
  // //
  // char lonBuf[15];
  // httpFinder.getString((char *)",", (char *)"\"", lonBuf, 15);
  // strcpy(lon, lonBuf);
  // Serial.println(String(lat)+","+lon);

  // httpFinder.findUntil((char *)"latitude", (char *) "\n\r");
  // // lat = (char)httpFinder.getValue();
  // lat=itoa(httpFinder.getValue(), lat, 10);
  //
  // httpFinder.findUntil((char *)"longitude", (char *) "\n\r");
  // // lon = (char)httpFinder.getValue();
  // lon=itoa(httpFinder.getValue(), lon, 10);
  // String json = "";

  String buffbuff = "";
  String preVal = "";
  String value = "";
  bool gettingVal = false;
  bool foundData = false;
  while(httpClient.available()){
    char cChar = httpClient.read();
    // Serial.println(cChar);

    if(cChar == '{'){
      foundData = true;
      Serial.println("Found the data!");
      // json = "{"
    } else if (cChar == '}') {
      foundData = false;
      Serial.println("Done with the data!");
    }

    // if (foundData) {
    //   json = json + cChar;
    // }

    if (foundData) {
      if (!gettingVal && cChar == '\"') {
        gettingVal = true;
      } else if (cChar == '\"') {
        gettingVal = false;
        // Serial.print("Got something: ");
        // Serial.println(value);
        // Serial.print("previousValue: ");
        // Serial.println(preVal);

        if (preVal == "ip") {
          Serial.print(preVal);
          Serial.print(": ");
          Serial.println(value);
        } else if (preVal == "loc") {
          Serial.print(preVal);
          Serial.print(": ");
          Serial.println(value);

          // Token will point to "SEVERAL".
          int comIndex = value.indexOf(",");
          Serial.println(comIndex);
          char * tempValue = new char[value.length() + 1];
          value.toCharArray(tempValue, value.length()+1);
          tempValue[value.length()] = '\0';
          Serial.println(tempValue);
          lat = strtok(tempValue, ",\n\r");
          lon = strtok(0, "\n\r");
          Serial.println(lat);
          Serial.println(lon);

          // lat = strtok(tempValue, ",");
          // Serial.println(lat);
          // lon = strtok(NULL, "\n");
          // Serial.println(lon);
          // Serial.println(tempValue);

          // lat = strtok(line, search);
          // Serial.println(lat);

          // Token will point to "WORDS".
          // lon = strtok(NULL, search);
          // Serial.println(lon);
        }

        preVal = value;
        value = "";
      } else if (gettingVal) {
        value = value + cChar;
      }
    }
  }

  httpClient.flush();
  httpClient.stop();

  return true;
}

#endif
