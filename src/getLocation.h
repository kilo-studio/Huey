#ifndef _getLocation_H_
#define _getLocation_H_

boolean getLocation();

//location
char lat[] = "37.8267";
char lon[] = "-122.4233";

boolean getLocation(IPAddress IP){
  //location by ip freegeoip.net/json/173.53.85.225
  Serial.println("---------");
  Serial.println("Getting Location from IP...");
  const char locationHost[] = "freegeoip.net";
  String ipStr = String(IP[0])+"."+String(IP[1])+"."+String(IP[2])+"."+String(IP[3]);
  String locationURL = "/json/";

  WiFiClient httpClient;
  TextFinder httpFinder(httpClient, 20);//finder and wait time in seconds
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

  // delay(250);
  httpFinder.findUntil((char *)"{", (char *)"\n\r");
  httpFinder.findUntil((char *)"ip\":", (char *)"\n\r");
  //Serial.println(httpFinder.getValue());
  // httpFinder.findUntil("latitude\":", "\n\r");
  char latBuf[15];
  httpFinder.getString((char *)"latitude\":", (char *)",\"", latBuf, 15);
  strcpy(lat, latBuf);

  char lonBuf[15];
  httpFinder.getString((char *)"longitude\":", (char *)",\"", lonBuf, 15);
  strcpy(lon, lonBuf);
  Serial.println(String(lat)+","+lon);

  // while(httpClient.available()){
  //    Serial.print(char(httpClient.read()));
  // }

  httpClient.flush();
  httpClient.stop();

  return true;
}

#endif
