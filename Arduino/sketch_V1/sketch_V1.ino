#include <Adafruit_NeoPixel.h>

#include <Adafruit_NeoPixel.h>

#define PIN        6      // Пинът, към който е свързана лентата
#define NUMPIXELS  180    // Брой диоди

// Инициализация на лентата
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Променливи за текущото състояние
int currentMode = 1;      // 0 = Изключено, 1 = Плътен цвят, 2 = Бягаща светлина
int currentR = 255;
int currentG = 0;
int currentB = 0;

// Променливи за анимациите (неблокиращи)
unsigned long previousMillis = 0;
int animationStep = 0;

void setup() {
  Serial.begin(9600);     // Стартираме серийната комуникация (важно: 9600 бода)
  strip.begin();          // Инициализираме лентата
  strip.show();           // Изключваме всички диоди първоначално
  strip.setBrightness(100); // Яркост от 0 до 255 (започваме с по-ниска за безопасност)
  
  Serial.println("Системата е готова. Въведете команда във формат: РЕЖИМ,R,G,B");
}

void loop() {
  // 1. Проверка за нови команди по серийния порт
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n'); // Четем до натискане на Enter
    
    // Търсим запетаите, за да разделим стринга (парсинг)
    int comma1 = data.indexOf(',');
    int comma2 = data.indexOf(',', comma1 + 1);
    int comma3 = data.indexOf(',', comma2 + 1);
    
    if (comma1 > 0 && comma2 > 0 && comma3 > 0) {
      currentMode = data.substring(0, comma1).toInt();
      currentR = data.substring(comma1 + 1, comma2).toInt();
      currentG = data.substring(comma2 + 1, comma3).toInt();
      currentB = data.substring(comma3 + 1).toInt();
      
      Serial.print("Нова команда приета -> Режим: "); Serial.print(currentMode);
      Serial.print(" | RGB: "); Serial.print(currentR); Serial.print(","); 
      Serial.print(currentG); Serial.print(","); Serial.println(currentB);
      
      // Рестартираме стъпките за анимациите при смяна на режима
      animationStep = 0; 
      strip.clear();
      strip.show();
    }
  }

  // 2. Изпълнение на ефектите спрямо текущия режим
  switch (currentMode) {
    case 0: // Изключено
      strip.clear();
      strip.show();
      break;
      
    case 1: // Плътен цвят (Solid)
      for(int i=0; i<NUMPIXELS; i++) {
        strip.setPixelColor(i, strip.Color(currentR, currentG, currentB));
      }
      strip.show();
      break;
      
    case 2: // Бягаща светлина (Running Light)
      runEffectRunningLight();
      break;
      
    // Тук по-късно ще добавим Knight Rider и други ефекти
  }
}

// Функция за бягаща светлина (без delay!)
void runEffectRunningLight() {
  unsigned long currentMillis = millis();
  int speedDelay = 30; // Скорост в милисекунди
  
  if (currentMillis - previousMillis >= speedDelay) {
    previousMillis = currentMillis;
    
    strip.clear(); // Изчистваме старите пиксели
    strip.setPixelColor(animationStep, strip.Color(currentR, currentG, currentB));
    strip.show();
    
    animationStep++;
    if (animationStep >= NUMPIXELS) {
      animationStep = 0; // Връщаме в началото
    }
  }
}