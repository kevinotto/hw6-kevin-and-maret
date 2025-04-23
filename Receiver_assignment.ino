#include <ArduinoBLE.h>

//Bluetooth
BLEService ledService("b1dcb9d8-f0ae-4e88-a3cd-3e1b5e5ff23b");

BLEByteCharacteristic switchCharacteristic("a3f3ce58-97a2-41d3-89f0-fd7ea6489426", BLERead | BLEWrite);
BLEByteCharacteristic distanceCharacteristic("3b26ffb0-8744-4209-a471-755ffcd6f3a2", BLERead | BLEWrite);
BLEByteCharacteristic lightCharacteristic("8236e18f-332b-48c5-a963-df034ced1529", BLERead | BLEWrite);

// Pins
const int buzzerPin = 2;
const int ledLow = 5;
const int ledMid = 6;
const int ledHigh = 9;
const int ledButton = 10;

byte distance, light;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(buzzerPin, OUTPUT);
  pinMode(ledLow, OUTPUT);
  pinMode(ledMid, OUTPUT);
  pinMode(ledHigh, OUTPUT);
  pinMode(ledButton, OUTPUT);

  digitalWrite(ledLow, LOW);
  digitalWrite(ledMid, LOW);
  digitalWrite(ledHigh, LOW);
  digitalWrite(ledButton, LOW);

  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);

  ledService.addCharacteristic(switchCharacteristic);
  ledService.addCharacteristic(distanceCharacteristic);
  ledService.addCharacteristic(lightCharacteristic);

  BLE.addService(ledService);

  switchCharacteristic.writeValue(0);
  distanceCharacteristic.writeValue(0);
  lightCharacteristic.writeValue(0);

  BLE.advertise();

  Serial.println("BLE is ready");
}

void loop() {
  BLEDevice central = BLE.central();

//When connection is established
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
// Detects the distance
      if (distanceCharacteristic.written()) {
        distance = distanceCharacteristic.value();
        Serial.print("Distance: ");
        Serial.println(distance);

//Distance treshold and buzzer function condition
        if (distance > 15) {
          tone(buzzerPin, 261);
        } else {
//If the distance is less than 15, then there's no buzzer tone
          noTone(buzzerPin);
        }
      }

// Detects if button is being pressed or released
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {
          Serial.println("Button is being pressed");
          digitalWrite(ledButton, HIGH);
        } else {
          Serial.println("Button is now released");
          digitalWrite(ledButton, LOW);
        }
      }        

// Reads the light level value from transmitter, activates LEDs based on light level
      if (lightCharacteristic.written()) {
        light = lightCharacteristic.value();
        Serial.print("Light level: ");
        Serial.println(light);

        digitalWrite(ledLow, light > 15 ? HIGH : LOW);
        digitalWrite(ledMid, light > 55 ? HIGH : LOW);
        digitalWrite(ledHigh, light > 75 ? HIGH : LOW);
      }
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}
