#include <SoftwareSerial.h>

// RX пин е D10, TX пин е D11
SoftwareSerial BTSerial(10, 11); 

void setup() {
  Serial.begin(9600);
  Serial.println("Въведете AT команди:");
  
  // HC-05 в AT режим по подразбиране комуникира на 38400 бода
  BTSerial.begin(38400);  
}

void loop() {
  // Четене от Bluetooth и писане в Serial Monitor
  if (BTSerial.available()) {
    Serial.write(BTSerial.read());
  }
  // Четене от Serial Monitor и писане към Bluetooth
  if (Serial.available()) {
    BTSerial.write(Serial.read());
  }
}