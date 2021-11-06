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

/* =========================================================
   ================= SETUP =============================
   =========================================================
*/
// WIFI setup
WiFiClient client;
int status = WL_IDLE_STATUS;     // the WiFi radio's status
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
}

/* =========================================================
   ================= LOOP =============================
   =========================================================
*/
unsigned long lastConnectionTime = 0; // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 1000L; // delay between requests, in milliseconds

void loop() {
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
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "dweet.io";    // name address for Google (using DNS)

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
  if (client.connect(server, 80)) {
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