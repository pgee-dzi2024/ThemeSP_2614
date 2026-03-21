#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h> // Добавяме библиотеката за софтуерен сериен порт

#define PIN        6
#define NUMPIXELS  120

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Инициализираме Bluetooth на пинове 10 (RX) и 11 (TX)
SoftwareSerial btSerial(10, 11);

int currentMode = 0;
int currentR = 0;
int currentG = 0;
int currentB = 0;
int currentSpeed = 30;

const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

unsigned long previousMillis = 0;
unsigned long previousSolidMillis = 0; // Таймер за плътния цвят
int animationStep = 0;
int krPos = 0;
int krDir = 1;

void setup() {
  // Хардуерният порт остава за дебъгване (ако кабелът е включен)
  Serial.begin(9600);

  // Стартираме Bluetooth комуникацията
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

  // Четем от Bluetooth модула
  while (btSerial.available() > 0 && newData == false) {
    rc = btSerial.read();
    
    // ДЕБЪГ: Принтираме това, което получаваме от Bluetooth в USB серийния монитор
    Serial.print("Получено от BT: ");
    Serial.println(rc);

    // Игнорираме \r (Carriage Return), ако приложението праща CR+LF
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

      // Принтираме в USB Serial монитора за дебъгване
      Serial.print("BT Команда: Режим="); Serial.print(currentMode);
      Serial.print(" R="); Serial.print(currentR);
      Serial.print(" G="); Serial.print(currentG);
      Serial.println();

      animationStep = 0;
      krPos = 0;
      krDir = 1;
      strip.clear();
      strip.show();
    }
    newData = false;
  }
}

void runEffects() {
  switch (currentMode) {
    case 0:
      strip.clear();
      strip.show();
      break;
    case 1:
      // Използваме таймер, за да не блокираме четенето от Bluetooth
      if (millis() - previousSolidMillis >= 50) {
        previousSolidMillis = millis();
        for(int i = 0; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, strip.Color(currentR, currentG, currentB));
        }
        strip.show();
      }
      break;
    case 2:
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
