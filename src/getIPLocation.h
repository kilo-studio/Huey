#ifndef _getIPLocation_H_
#define _getIPLocation_H_

boolean getIPLocation(IPAddress IP);

//location
// char lat[] = "37.8267";
// char lon[] = "-122.4233";
// int timeZone = 0;

boolean getIPLocation(IPAddress IP){
  String ipStr = String(IP[0])+"."+String(IP[1])+"."+String(IP[2])+"."+String(IP[3]);
  String locationURL = "/json/";

  WiFiSSLClient client;
  TextFinder finder(client, 20);//finder and wait time in seconds

  //location by ip freegeoip.net/json/173.53.85.225
  //https://ipapi.co/8.8.8.8/json/
  Serial.println("---------");
  Serial.println("Getting Location from IP...");
  const char host[] = "ipapi.co";
  String url = "/json/" + ipStr;
  //const static char url[] = "/forecast/e412ad3481ba8e4fdc137985443d60ca/37.593626,-77.521149/?extend=hourly&exclude=[latitude,longitude,minutely,alerts,flags]";

  Serial.print(F("connecting to "));
  Serial.println(host);

  if (!client.connect(host, 80)) {
    Serial.println(F("connection failed"));
    Serial.println(client.connect(host, 165));
    client.stop();
    return false;
  }

  Serial.print(F("requesting URL: "));
  Serial.println(url);


  Serial.print(F("requesting URL: "));
  Serial.println(locationURL);

  // send the HTTP GET request:
  client.println(String("GET ") + locationURL + " HTTP/1.1");
  client.println(String("Host: ") + host);
  client.println("Connection: close");
  client.println();

  Serial.println(F("request sent"));

  // delay(250);
  finder.findUntil((char *)"{", (char *)"\n\r");
  finder.findUntil((char *)"ip\":", (char *)"\n\r");
  //Serial.println(finder.getValue());
  // finder.findUntil("latitude\":", "\n\r");
  char latBuf[15];
  finder.getString((char *)"latitude\":", (char *)",\"", latBuf, 15);
  strcpy(lat, latBuf);

  char lonBuf[15];
  finder.getString((char *)"longitude\":", (char *)",\"", lonBuf, 15);
  strcpy(lon, lonBuf);
  Serial.println(String(lat)+","+lon);

  finder.findUntil((char *)"utc_offset\":", (char *)"\n\r");
  timeZone = finder.getValue();
  Serial.println(timeZone);

  // while(httpClient.available()){
  //    Serial.print(char(httpClient.read()));
  // }

  client.flush();
  client.stop();

  return true;
}

#endif
