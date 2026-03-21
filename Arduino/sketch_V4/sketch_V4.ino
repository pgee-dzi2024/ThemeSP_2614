#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

#define PIN        6
#define NUMPIXELS  120

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
SoftwareSerial btSerial(10, 11);

int currentMode = 0;
int currentR = 0;
int currentG = 0;
int currentB = 0;
int currentSpeed = 30;

const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

// Флаг, който показва, че имаме нова команда и трябва да нарисуваме ефекта веднъж
boolean modeChanged = true; 

unsigned long previousMillis = 0;
int animationStep = 0;
int krPos = 0;
int krDir = 1;

void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);

  strip.begin();
  strip.show();
  strip.setBrightness(100);

  Serial.println("Системата е готова. Очаквам команди през Bluetooth...");
}

void loop() {
  recvWithEndMarker();
  parseData();
  runEffects();
}

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (btSerial.available() > 0 && newData == false) {
    rc = btSerial.read();
    
    if (rc != endMarker && rc != '\r') {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else if (rc == endMarker) {
      receivedChars[ndx] = '\0';
      ndx = 0;
      newData = true;
    }
  }
}

void parseData() {
  if (newData == true) {
    int parsed = sscanf(receivedChars, "%d,%d,%d,%d,%d", &currentMode, &currentR, &currentG, &currentB, &currentSpeed);

    if (parsed == 5) {
      if (currentSpeed < 5) currentSpeed = 5;

      Serial.print("BT Команда: Режим="); Serial.print(currentMode);
      Serial.print(" R="); Serial.print(currentR);
      Serial.print(" G="); Serial.print(currentG);
      Serial.print(" Скорост="); Serial.println(currentSpeed);

      // Рестартираме променливите за анимациите
      animationStep = 0;
      krPos = 0;
      krDir = 1;
      
      // Вдигаме флага, за да кажем на runEffects() да приложи промяната
      modeChanged = true; 
      
      btSerial.println("OK: Mode " + String(currentMode));
    }
    newData = false;
  }
}

void runEffects() {
  switch (currentMode) {
    case 0:
      // Изчистваме лентата САМО ВЕДНЪЖ след получаване на командата
      if (modeChanged) {
        strip.clear();
        strip.show();
        modeChanged = false; // Сваляме флага, за да не блокираме Bluetooth-а
      }
      break;
      
    case 1:
      // Светваме плътен цвят САМО ВЕДНЪЖ
      if (modeChanged) {
        for(int i = 0; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, strip.Color(currentR, currentG, currentB));
        }
        strip.show();
        modeChanged = false;
      }
      break;
      
    case 2:
      // При анимациите timer-ът си върши работата
      runEffectRunningLight();
      break;
      
    case 3:
      runEffectKnightRider();
      break;
  }
}

void runEffectRunningLight() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= currentSpeed) {
    previousMillis = currentMillis;

    strip.clear();
    strip.setPixelColor(animationStep, strip.Color(currentR, currentG, currentB));
    strip.show();

    animationStep++;
    if (animationStep >= NUMPIXELS) animationStep = 0;
  }
}

void runEffectKnightRider() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= currentSpeed) {
    previousMillis = currentMillis;

    for(int i = 0; i < NUMPIXELS; i++) {
      uint32_t c = strip.getPixelColor(i);
      uint8_t r = (uint8_t)(c >> 16);
      uint8_t g = (uint8_t)(c >>  8);
      uint8_t b = (uint8_t)c;

      r = (r * 200) >> 8;
      g = (g * 200) >> 8;
      b = (b * 200) >> 8;
      strip.setPixelColor(i, strip.Color(r, g, b));
    }

    strip.setPixelColor(krPos, strip.Color(currentR, currentG, currentB));
    strip.show();

    krPos += krDir;
    if (krPos >= NUMPIXELS - 1) {
      krPos = NUMPIXELS - 1;
      krDir = -1;
    } else if (krPos <= 0) {
      krPos = 0;
      krDir = 1;
    }
  }
}