
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

// #define WIFI_SCHOOL
#define WIFI_HOME

#ifdef WIFI_SCHOOL
char ssid[] = SECRET_SSID_SCHOOL; 
char user[] = SECRET_USER_SCHOOL;  
char pass[] = SECRET_PASS_SCHOOL; 
#endif
#ifdef WIFI_HOME
char ssid[] = SECRET_SSID_HOME; 
char pass[] = SECRET_PASS_HOME; 
#endif


void enable_WiFi();
void connect_WiFi();
void printWifiStatus();
void connectBroker();
void subscribeAll();
void publishLightValue();

/*-------------------
Setup
*/
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

int statusWifi = WL_IDLE_STATUS;  // the WiFi radio's status
int statusBroker = MQTT_CONNECTION_TIMEOUT;

const char broker[] = "test.mosquitto.org";
int        port     = 1883;
// const char topicSubscribe1[]  = "sensor1/ledState";
// const char topicSubscribe2[]  = "sensor1/cylceTime";
String topicSubscribe1 = "sensor1/ledState";
String topicSubscribe2 = "sensor1/cycleTime";
String topicPublish1 = "sensor1/light";

unsigned long lastConnectionTime = 0;
unsigned long cycleTime = 10L * 1000L;

const int LightSensorPin = A0;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  enable_WiFi();
  connect_WiFi();
  Serial.println("Connected to WiFi");
  printWifiStatus();

  connectBroker();
  Serial.println("You're connected to the MQTT broker!");
  subscribeAll();

}

void loop() {
  size_t messageSize = mqttClient.parseMessage();
  if(messageSize){ //recieved
    String topicRecieved = mqttClient.messageTopic();
    // debug
    Serial.print("Received a message with topic '");
    Serial.print(topicRecieved);
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");
    //----------
    uint8_t message[messageSize];
    int idx = 0;
    mqttClient.read(message, messageSize); 
    String messageStr = (char*)message;  
    if(topicRecieved.equals(topicSubscribe1)){
      if(messageStr.startsWith("ON")){
        digitalWrite(LED_BUILTIN, HIGH);
      }else{
        digitalWrite(LED_BUILTIN, LOW);        
      }
    }else if(topicRecieved.equals(topicSubscribe2)){  
      cycleTime = (unsigned long)messageStr.toInt(); 
    }else{
      Serial.print("Unkown Topic recieved: ");
      Serial.println(topicRecieved); 
    }
  }

    
  if(millis() - lastConnectionTime > cycleTime){
    publishLightValue();
  } 

}

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
  while (statusWifi != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network (enterprise at school)
    #ifdef WIFI_HOME
    statusWifi = WiFi.begin(ssid, pass);
    #endif
    #ifdef WIFI_SCHOOL
    statusWifi = WiFi.beginEnterprise(ssid, user, pass);
    #endif

    // wait 10 seconds for connection:
    delay(5000);
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

/* ==========================================================
    connect to Mqtt Broker
   ========================================================= */
void connectBroker() {
  
  while(statusBroker != MQTT_SUCCESS){
    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(broker);

    mqttClient.connect(broker,port);
    statusBroker = mqttClient.connectError();
    Serial.println(statusBroker);
    delay(1000);
  }

}

/* ==========================================================
    subscribe all 'Subscribe' topcis
   ========================================================= */
void subscribeAll() {
  mqttClient.subscribe(topicSubscribe2, 0);
  mqttClient.subscribe(topicSubscribe1, 0);
}

/* ==========================================================
   publish the light value to Broker
   ========================================================= */
void publishLightValue(){
  float illuminance = analogRead(LightSensorPin) * 3300 / 1023.0 / 5.0;
  mqttClient.beginMessage(topicPublish1);
  mqttClient.print(illuminance);
  mqttClient.endMessage();
  lastConnectionTime = millis();    
}

