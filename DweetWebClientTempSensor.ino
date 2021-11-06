#include <WiFiNINA.h>
#include <Arduino_MKRENV.h>
#include "arduino_secrets.h"
#include <stdlib.h>

/*
  This code is based on the Arduino exampe:
  File->Examples->WiFiNINA->WifiWebClientRepeating
  see: https://www.arduino.cc/en/Tutorial/LibraryExamples/WiFiWebClientRepeating

  WiFiNINA reference
  https://www.arduino.cc/en/Reference/WiFiNINA
  
  Reads the light sensor from the MKR ENV shield and sends it to
  http://dweet.io/dweet/for/IOTS2020

  to do the same in cURL:
  curl -vX POST "https://dweet.io/dweet/for/IOTS2020?lux=81"
  to post 2 values in cURL:
  curl -vX POST "https://dweet.io/dweet/for/IOTS2020?tmp=20&lux=81"
*/

/* =========================================================
   ================= SETTINGS ==============================
   =========================================================
*/
// Please choose wether the WiFi credentials from school or home shall be taken 
// by uncommenting the inapplicable option
//#define WIFI_SCHOOL
#define WIFI_HOME

/* =========================================================
   ================= FUNCTION DEFINITION ===================
   =========================================================
*/
void enable_WiFi();
void connect_WiFi();
void printWifiStatus();
void httpRequest();
void printServerPage();

/* =========================================================
   ================= SETUP =============================
   =========================================================
*/
// WIFI setup
WiFiClient client;          // client
WiFiServer server(80);      // server socket
WiFiClient clientWebServer = server.available(); // client for webserver
int status = WL_IDLE_STATUS;  // the WiFi radio's status
#ifdef WIFI_SCHOOL
char ssid[] = SECRET_SSID_SCHOOL; 
char user[] = SECRET_USER_SCHOOL;  
char pass[] = SECRET_PASS_SCHOOL; 
#endif
#ifdef WIFI_HOME
char ssid[] = SECRET_SSID_HOME; 
char pass[] = SECRET_PASS_HOME; 
#endif

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Serial port opened successfully.");

  // initialize ENV shield
  if (!ENV.begin()) {
    Serial.println("ERROR: Failed to initialize MKR ENV shield!");
    while (1);
  }
  Serial.println("MKR ENV shield inizialized successfully");
 
  // start WiFi
  enable_WiFi();
  connect_WiFi();
  Serial.println("Connected to WiFi");
  printWifiStatus();

  // start webserver
  server.begin();
  Serial.println("Started Webserver");

}

/* =========================================================
   ================= LOOP =============================
   =========================================================
*/
unsigned long lastConnectionTime = 0; // last time you connected to the server, in milliseconds
unsigned long postingInterval = 10L * 1000L; // delay between requests, in milliseconds

void loop() {
  // Web server
  clientWebServer = server.available();  //check if the server is available
  if (clientWebServer) {  
    Serial.println("server available");
    printServerPage(clientWebServer);
  }  
  
  // if there's incoming data from a webserver connection,
  // send it out the serial port for debugging purposes:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // every postingInterval miliseconds (default 10s), connect to the dweet.io server and send data
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
}

/* =========================================================
   ================= FUNCTIONS =============================
   =========================================================
*/

/* =========================================================
  void httpRequest() {
  ========================================================= */
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress host(74,125,232,128);  // numeric IP for Google (no DNS)
char host[] = "dweet.io";    // name address for Google (using DNS)

char *req[] = {
  "POST /dweet/for/IOTS2021?tmp=23 HTTP / 1.1",
  "Host : dweet.io",
  "User - Agent : IOTS_ArduinoWiFi / 1.1",
  "Connection : close"
};

void httpRequest() {

  // close any connection before send a new request.
  // This will free the socket on the Nina module
  client.stop();

  // if there's a successful connection:
  if (client.connect(host, 80)) {
    Serial.println("connecting to Dweet.io");

    Serial.println("sensors read.");

    // create string from int
    int lux = (int)(ENV.readIlluminance() + 0.5);
    char buffLight[20];
    itoa(lux, buffLight, 10);
    int tmp = (int)(ENV.readTemperature() + 0.5);
    char buffTemp[20];
    itoa(tmp, buffTemp, 10);

    // EXAMPLE "POST /dweet/for/IOTS2020?lux=99&tmp=23 HTTP/1.1"
    client.print("POST /dweet/for/IOTS2021?lux=");
    client.print(buffLight);
    client.print("&tmp=");
    client.print(buffTemp);
    client.println(" HTTP/1.1");
    client.println(req[1]);
    client.println(req[2]);
    client.println(req[3]);
    client.println();

    // remember the current time, i.e., when the last connection was made
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }

} // end httpRequest()

/* =========================================================
   printServerPage()

   ========================================================= */

void printServerPage(WiFiClient client) {

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {

            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            //create the links
            client.print("Click <a href=\"/H\">here</a> turn the LED on<br>");
            client.print("Click <a href=\"/L\">here</a> turn the LED off<br>");
            client.print("<br><br><form action=\"/get\">Sensor update cycle periodicity (in ms): <input type=\"number\" name=\"periodMs\" min=\"2000\" max=\"60000\" value=\"");
            client.print(postingInterval);
            client.print("\"><input type=\"submit\" value=\"Set\"></form><br>");

            int randomReading = analogRead(A1);
            client.print("Random reading from analog pin: ");
            client.print(randomReading);

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } // end if (c == '\n')
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        bool getRequestRecognized = false;
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(LED_BUILTIN, HIGH);
          getRequestRecognized = true;
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(LED_BUILTIN, LOW);
          getRequestRecognized = true;
        }
        String getRequestKey = "/get?periodMs=";
        int indexOfGetRequest = currentLine.indexOf(getRequestKey);
        if(indexOfGetRequest > 0) {   // get request has been found in the uri
          long requestedPeriodicity = currentLine.substring(indexOfGetRequest + getRequestKey.length()).toInt();  // + for amount of letters for getRequestKey
          Serial.print("New periodicity requested: ");
          Serial.println(requestedPeriodicity);
          if(requestedPeriodicity >= 2000 && requestedPeriodicity < 60000)  // range of allowed values
          {
            postingInterval = requestedPeriodicity;
            Serial.print("New periodicity set.");
          }
          getRequestRecognized = true;
        }

        // reload client page when a get request was recognized => clear URI
        if(getRequestRecognized)
        {
            client.println("HTTP/1.1 301 Moved Permanently");
            client.println("Location: /");
            client.println("Content-type:text/html");
            client.println();
        }
      } // end if (client.available())
    } // end while (client.connected())

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  } // end if (client)
} // end printWEB()

/* =========================================================
  enable_WiFi()
  checks if the prerequisites are met by WiFi-module on the board
  (if any) and it the correct firmware version is installed.

  this is NOT enabling but checking the prereqisites...
  ========================================================= */
void enable_WiFi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  // check for firmware
  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
} // end enable_WiFi()


/* =========================================================
  connect_WiFi()
  ========================================================= */
void connect_WiFi() {
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network (enterprise at school)
    #ifdef WIFI_HOME
    status = WiFi.begin(ssid, pass);
    #endif
    #ifdef WIFI_SCHOOL
    status = WiFi.beginEnterprise(ssid, user, pass);
    #endif

    // wait 10 seconds for connection:
    delay(10000);
  }
} // end connect_WiFi()

/* =========================================================
  printWifiStatus()

  ========================================================= */

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");

} // end printWifiStatus()