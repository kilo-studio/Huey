#ifndef _getTimeZone_H_
#define _getTimeZone_H_

boolean getTimeZone();

int timeZone = 0;

boolean getTimeZone(String latitude, String longitude){
  Serial.println("---------");
  Serial.println("Getting timezone...");

  const char timezoneHost[] = "api.timezonedb.com";
  String fields = "&fields=gmtOffset";//countryCode,cityName,gmtOffset,dst
  String timezoneURL = "/v2/get-time-zone?key=713CACOR5I7K&format=json&by=position&lat=" + latitude + "&lng="+ longitude + fields;//remember to add the ip address later

  WiFiClient httpClient;
  TextFinder httpFinder(httpClient, 20);//finder and wait time in seconds
  Serial.print(F("connecting to "));
  Serial.println(timezoneHost);
  const int httpPort = 80;

  if (!httpClient.connect(timezoneHost, httpPort)) {
    Serial.println("connection failed");
    return false;
  }

  Serial.print(F("requesting URL: "));
  Serial.println(timezoneURL);

  // send the HTTP GET request:
  httpClient.println(String("GET ") + timezoneURL + " HTTP/1.1");
  httpClient.println(String("Host: ") + timezoneHost);
  httpClient.println("Connection: close");
  httpClient.println();

  Serial.println(F("request sent"));

  // delay(250);

  httpFinder.findUntil((char *)"{", (char *)"\n\r");
  httpFinder.findUntil((char *)"gmtOffset\":", (char *)"\n\r");
  timeZone = httpFinder.getValue();
  Serial.println(timeZone);

  // while(httpClient.available()){
  //    Serial.print(char(httpClient.read()));
  // }

  httpClient.stop();
  httpClient.flush();

  return true;
}

#endif
