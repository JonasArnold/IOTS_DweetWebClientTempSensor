/*
  Peripheral Explorer

  This example scans for BLE peripherals until one with a particular name ("LED")
  is found. Then connects, and discovers + prints all the peripheral's attributes.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use it with another board that is compatible with this library and the
  Peripherals -> LED example.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

#define LOCAL_NAME_PERIPHERAL IOTS_BLE_Peripheral_JoSi

int illuminanceState = 0;
int oldIlluminanceState = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  Serial.println("BLE Central - IOTS");

  // start scanning for peripherals
  BLE.scan();
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    // see if peripheral is a LED
    if (peripheral.localName() == LOCAL_NAME_PERIPHERAL) {  
      // stop scanning
      BLE.stopScan();

      explorerPeripheral(peripheral);
    }
  }
}

void explorerPeripheral(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  while (peripheral) {
    // discover peripheral attributes
    Serial.println("Discovering attributes ...");
    if (peripheral.discoverAttributes()) {
      Serial.println("Attributes discovered");
    } else {
      Serial.println("Attribute discovery failed!");
      peripheral.disconnect();
      return;
    }

    BLECharacteristic temperatureCharacteristic = peripheral.characteristic("2A6E");
    BLECharacteristic humidityCharacteristic = peripheral.characteristic("2A6F");
    BLECharacteristic illuminanceCharacteristic = peripheral.characteristic("2AFB");
    BLECharacteristic ledCharacteristic = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");

    if(!illuminanceCharacteristic){
      Serial.println('no illuminanceCharacteristic');
      peripheral.disconnect();
      return;
    }else if(!illuminanceCharacteristic.canSubscribe()){
      Serial.println('cant subscribe illuminanceCharacteristic');
      peripheral.disconnect();
      return;            
    }else if(!illuminanceCharacteristic.subscribe()){
      Serial.println("subscription of illuminanceCharacteristic failed!");
      peripheral.disconnect();
      return;
    }else{
      Serial.println('subscribe illuminanceCharacteristic success');
    }

    if(!humidityCharacteristic){
      Serial.println('no humidityCharacteristic');
      peripheral.disconnect();
      return;
    }else if(!humidityCharacteristic.canSubscribe()){
      Serial.println('cant subscribe humidityCharacteristic');
      peripheral.disconnect();
      return;            
    }else if(!humidityCharacteristic.subscribe()){
      Serial.println("subscription of humidityCharacteristic failed!");
      peripheral.disconnect();
      return;
    }else{
      Serial.println('subscribe humidityCharacteristic success');
    }

    if(!temperatureCharacteristic){
      Serial.println('no temperatureCharacteristic');
      peripheral.disconnect();
      return;
    }else if(!temperatureCharacteristic.canSubscribe()){
      Serial.println('cant subscribe temperatureCharacteristic');
      peripheral.disconnect();
      return;            
    }else if(!temperatureCharacteristic.subscribe()){
      Serial.println("subscription of temperatureCharacteristic failed!");
      peripheral.disconnect();
      return;
    }else{
      Serial.println('subscribe temperatureCharacteristic success');
    }

    if (!ledCharacteristic) {
      Serial.println("Peripheral does not have LED characteristic!");
      peripheral.disconnect();
      return;
    } else if (!ledCharacteristic.canWrite()) {
      Serial.println("Peripheral does not have a writable LED characteristic!");
      peripheral.disconnect();
      return;
    }
    int16_t illVal, humVal, tempVal;
    while(peripheral.connected()){
      if(illuminanceCharacteristic.valueUpdated()){
        illuminanceCharacteristic.readValue(illVal);
        Serial.print("illuminance: ");
        Serial.println(illVal);    
      }
      if(humidityCharacteristic.valueUpdated()){
        humidityCharacteristic.readValue(humVal);
        Serial.print("humidity: ");
        Serial.println(humVal);    
      }
      if(temperatureCharacteristic.valueUpdated()){
        temperatureCharacteristic.readValue(tempVal);
        Serial.print("temp: ");
        Serial.println(tempVal);    
      }    

      if(illVal >= 100){
        illuminanceState = 1;                
      }else{
        illuminanceState = 0;
      }

     if (oldIlluminanceState != illuminanceState) {
      oldIlluminanceState = illuminanceState;

      if (illuminanceState) {
        Serial.println("ill > 100");
        ledCharacteristic.writeValue((byte)0x01);
      } else {
        Serial.println("ill < 100");
        ledCharacteristic.writeValue((byte)0x00);
      }
     }
    
  }
  }
}




