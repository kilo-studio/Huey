#ifndef _webService_h_
#define _webService_h_

#include "main.cpp"

void checkForClients();
boolean setUpWebService();

char mdnsName[] = "huey"; // the MDNS name that the board will respond to
                             // after WiFi settings have been provisioned
// Note that the actual MDNS name will have '.local' after
// the name above, so "wifi101" will be accessible on
// the MDNS name "wifi101.local".

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
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
//          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
//            int sensorReading = analogRead(analogChannel);
//            client.print("analog input ");
//            client.print(analogChannel);
//            client.print(" is ");
//            client.print(sensorReading);
//            client.println("<br />");
//          }
          client.println("<h1>Hello!</h1>");
          client.print("<input type=""text"" name=""fname""><br>");
          client.print("<a href=\"/H\">red LED on</a><br>");
          client.print("<a href=\"/L\">red LED off</a><br>");
          // The HTTP response ends with another blank line:
          client.println();
          client.println("</html>");


          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          currentLine = "";
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
          currentLine += c;
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(13, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(13, LOW);                // GET /L turns the LED off
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}




#endif
