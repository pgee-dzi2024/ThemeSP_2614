#include <Adafruit_NeoPixel.h>

#define PIN        6 // Пинът на Ардуино
#define NUMPIXELS 30 // Брой диоди в лентата

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin(); // Старт на библиотеката
}

void loop() {
  pixels.clear(); // Изчистване на старите цветове

  // Светване на първия диод в червено
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));

  // Светване на втория диод в зелено
  pixels.setPixelColor(1, pixels.Color(0, 255, 0));

  pixels.show();   // Изпращане на командата към лентата
  delay(1000);
}