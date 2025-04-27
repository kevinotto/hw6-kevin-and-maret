#include <ArduinoBLE.h>

const int buttonPin = 8;
const int triggerPin = 3;
const int echoPin = 2;
const int lightPin = A0;

int oldButtonState = LOW;

byte distance;
byte light;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // configure pin modes
  pinMode(buttonPin, INPUT);
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // initialize BLE
  BLE.begin();
  Serial.println("BLE Address: " + BLE.address());

  // scan for peripheral with specific service UUID
  BLE.scanForUuid("b1dcb9d8-f0ae-4e88-a3cd-3e1b5e5ff23b");
}

void loop() {
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    Serial.println("Found " + peripheral.address() + " '" + peripheral.localName() + "' " + peripheral.advertisedServiceUuid());

    if (peripheral.localName() != "LED") {
      return;
    }

    BLE.stopScan();
    button_and_transmit_data(peripheral);
    BLE.scanForUuid("b1dcb9d8-f0ae-4e88-a3cd-3e1b5e5ff23b");
  }
}

void button_and_transmit_data(BLEDevice peripheral) {
  Serial.println("Connecting to bluetooth peripheral");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  Serial.println("Discovering attributes");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic ledCharacteristic = peripheral.characteristic("a3f3ce58-97a2-41d3-89f0-fd7ea6489426");
  BLECharacteristic distanceCharacteristic = peripheral.characteristic("3b26ffb0-8744-4209-a471-755ffcd6f3a2");
  BLECharacteristic lightCharacteristic = peripheral.characteristic("8236e18f-332b-48c5-a963-df034ced1529");

  if (!ledCharacteristic || !ledCharacteristic.canWrite()) {
    Serial.println("LED characteristic not writable or missing!");
    peripheral.disconnect();
    return;
  }

  if (!distanceCharacteristic || !distanceCharacteristic.canWrite()) {
    Serial.println("Distance characteristic not writable or missing!");
    peripheral.disconnect();
    return;
  }

  if (!lightCharacteristic || !lightCharacteristic.canWrite()) {
    Serial.println("Light characteristic not writable or missing!");
    peripheral.disconnect();
    return;
  }

  while (peripheral.connected()) {
    measureDistance();
    measureLight();

    int buttonState = digitalRead(buttonPin);

    if (oldButtonState != buttonState) {
      oldButtonState = buttonState;

      if (buttonState) {
        Serial.println("Button pressed");
        ledCharacteristic.writeValue((byte)0b00000001);
      } else {
        Serial.println("Button released");
        ledCharacteristic.writeValue((byte)0b00000000);
      }
    }

    distanceCharacteristic.writeValue(distance);
    lightCharacteristic.writeValue(light);
    delay(500);
  }

  Serial.println("Peripheral disconnected");
}

void measureDistance() {
  long duration;
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  const float tempDistance = duration * 0.034 / 2;
  distance = (byte)tempDistance;

  Serial.print("Distance: ");
  Serial.println(distance);
}

void measureLight() {
  int sum = 0;
  const int samples = 10;

  for (int i = 0; i < samples; i++) {
    sum += analogRead(lightPin);
    delay(5);  // short delay between samples
  }

  int average = sum / samples;
  light = (byte)(average / 10);

  Serial.print("Light (scaled): ");
  Serial.println(light);
}