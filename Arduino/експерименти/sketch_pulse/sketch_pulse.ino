#include <Adafruit_NeoPixel.h>

#define PIN        6
#define NUMPIXELS 30 // Променете на броя вашите диоди

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

  // Движение НАЗАД (от края до 0)
  for(int i = NUMPIXELS - 1; i >= 0; i--) {
    fadeToBlack(20); // Затъмняваме опашката
    pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // Светваме "главата"
    pixels.show();
    delay(30);
  }
}

// Помощна функция за създаване на ефекта "опашка"
// Тя не гаси диодите веднага, а намалява яркостта им малко по малко
void fadeToBlack(int fadeValue) {
  for(int i = 0; i < NUMPIXELS; i++) {
    uint32_t oldColor = pixels.getPixelColor(i);
    uint8_t r = (oldColor >> 16) & 0xFF;
    uint8_t g = (oldColor >> 8) & 0xFF;
    uint8_t b = oldColor & 0xFF;

    r = (r <= 10) ? 0 : (int) r - (r * fadeValue / 256);
    g = (g <= 10) ? 0 : (int) g - (g * fadeValue / 256);
    b = (b <= 10) ? 0 : (int) b - (b * fadeValue / 256);

    pixels.setPixelColor(i, r, g, b);
  }
}