#include <Adafruit_NeoPixel.h>

#define PIN        6
#define NUMPIXELS 180 // Променете на броя вашите диоди

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();
  pixels.setBrightness(100); // Средна яркост
}

void loop() {
  for(int i = 0; i < NUMPIXELS; i++) {
    // 1. Светваме текущия диод в синьо
    pixels.setPixelColor(i, pixels.Color(0, 0, 255));
    pixels.show();
    
    // 2. Чакаме малко (скорост на бягане)
    delay(50);
    
    // 3. Гасим същия диод, за да не остане светнал
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
}