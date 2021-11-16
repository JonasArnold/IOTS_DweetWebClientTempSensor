/*
  BLE Peripheral

  This sketch creates a BLE peripheral with the Environment sensing service, 
  that contains the following characteristics:
  - temperature
  - humidity
  - illuminance
  Also the sketch runs a custom service that contains a
  custom characteristic to control an LED.

  The circuit:
  - Arduino MKR WiFi 1010 with MKR ENV Board attached.

  You can use a generic BLE central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <Arduino_MKRENV.h>

#define LOCAL_NAME "IOTS_BLE_Peripheral_JoSi"
#define SENSOR_UPDATE_TIME_MS 1000

// SERVICE
BLEService envService("181A"); // BLE Environmental Sensing Service
BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // custom BLE LED Service

// CHARACTERISTIC
// BLE Environmental Sensing Characteristics - read and notify permissions for central
BLEIntCharacteristic temperatureCharacteristic("2A6E", BLERead | BLENotify);
BLEIntCharacteristic humidityCharacteristic("2A6F", BLERead | BLENotify);
BLEIntCharacteristic illuminanceCharacteristic("2AFB", BLERead | BLENotify);

// BLE LED Switch custom Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int ledPin = LED_BUILTIN; // pin to use for the LED
unsigned long last_update;  // system tick when last sensor update was sent via BLE

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // MKR ENV BOARD INIT
  if (!ENV.begin()) {
      Serial.println("Failed to initialize MKR ENV Shield!");
      while (1);
      }

  // BLE INIT
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  // set advertised local name and services UUID:
  BLE.setLocalName(LOCAL_NAME);
  BLE.setAdvertisedService(envService);
  BLE.setAdvertisedService(ledService);

  // add the characteristics to the service
  envService.addCharacteristic(temperatureCharacteristic);
  envService.addCharacteristic(humidityCharacteristic);
  envService.addCharacteristic(illuminanceCharacteristic);
  ledService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(envService);
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE Peripheral started.");
}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      
      // ENVIRONMENTAL SENSING SERVICE
      // update sensors every SENSOR_UPDATE_TIME_MS to not stress the ENV board
      if((last_update - millis()) > SENSOR_UPDATE_TIME_MS)
      {
        int16_t temperature = ENV.readTemperature();
        int16_t humidity    = ENV.readHumidity();
        int16_t illuminance = ENV.readIlluminance();
        
        temperatureCharacteristic.writeValue(temperature);
        humidityCharacteristic.writeValue(humidity);
        illuminanceCharacteristic.writeValue(illuminance);

        last_update = millis();  // update time
      }
      
      // LED SERVICE
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }
      }
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}
