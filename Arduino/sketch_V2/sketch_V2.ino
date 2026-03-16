#include <Adafruit_NeoPixel.h>

#define PIN        6
#define NUMPIXELS  180

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Текущо състояние
int currentMode = 0;
int currentR = 0;
int currentG = 0;
int currentB = 0;
int currentSpeed = 30; // Дефолтна скорост (закъснение в ms)

// Променливи за серийната комуникация
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

// Променливи за анимациите
unsigned long previousMillis = 0;
int animationStep = 0;

// Променливи специфични за Knight Rider
int krPos = 0;
int krDir = 1;

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show();
  strip.setBrightness(100);
  Serial.println("Системата е готова. Формат: РЕЖИМ,R,G,B,СКОРОСТ");
}

void loop() {
  recvWithEndMarker(); 
  parseData();         
  runEffects();        
}

// --- Функция за надеждно четене от серийния порт ---
void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0'; // Терминираме стринга
      ndx = 0;
      newData = true;
    }
  }
}

// --- Функция за извличане на числата ---
void parseData() {
  if (newData == true) {
    // Вече очакваме 5 параметъра
    int parsed = sscanf(receivedChars, "%d,%d,%d,%d,%d", &currentMode, &currentR, &currentG, &currentB, &currentSpeed);
    
    // Проверяваме дали успешно сме прочели и 5-те
    if (parsed == 5) {
      Serial.print("ОК: Режим="); Serial.print(currentMode);
      Serial.print(" R="); Serial.print(currentR);
      Serial.print(" G="); Serial.print(currentG);
      Serial.print(" B="); Serial.print(currentB);
      Serial.print(" Скорост(ms)="); Serial.println(currentSpeed);
      
      // Предпазваме от задаване на скорост 0 или отрицателна (ще забие Arduino-то)
      if (currentSpeed < 5) currentSpeed = 5; 
      
      // Рестартираме променливите за анимация при смяна на команда
      animationStep = 0;
      krPos = 0;
      krDir = 1;
      strip.clear();
      strip.show();
    } else {
      Serial.println("Грешка в командата! Използвайте формат: РЕЖИМ,R,G,B,СКОРОСТ");
    }
    newData = false;
  }
}

// --- Управление на ефектите ---
void runEffects() {
  switch (currentMode) {
    case 0:
      strip.clear();
      strip.show();
      break;
    case 1:
      // ДОБАВЕНО: Опресняваме плътния цвят само на всеки 50ms, 
      // за да не блокираме четенето от серийния порт!
      if (millis() - previousMillis >= 50) {
        previousMillis = millis();
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

// --- Ефект: Бягаща светлина ---
void runEffectRunningLight() {
  unsigned long currentMillis = millis();
  
  // Използваме currentSpeed вместо твърдо зададено число
  if (currentMillis - previousMillis >= currentSpeed) { 
    previousMillis = currentMillis;
    
    strip.clear();
    strip.setPixelColor(animationStep, strip.Color(currentR, currentG, currentB));
    strip.show();
    
    animationStep++;
    if (animationStep >= NUMPIXELS) animationStep = 0;
  }
}

// --- Ефект: Knight Rider (KITT) ---
void runEffectKnightRider() {
  unsigned long currentMillis = millis();
  
  // Използваме currentSpeed вместо твърдо зададено число
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