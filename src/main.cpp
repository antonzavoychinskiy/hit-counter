/*
  Counter node bring-up test (PlatformIO version)
  ------------------------------------------------
  Проверка периферии узла-счётчика ДО подключения LM393 и радио E220:
  TM1637 (дисплей) + KY-004 (кнопка, имитирует "попадание") + активный зуммер.

  По кнопке: счётчик +1, дисплей обновляется, короткий писк, лог в Serial.
  Позже вход GPIO_BTN просто переключается на выход LM393-модуля —
  остальная логика (счётчик/дисплей/бип) не меняется.
*/

#include <Arduino.h>
#include <TM1637Display.h>

// ---- Пины (СВЕРЬТЕ С ШЁЛКОГРАФИЕЙ СВОЕЙ КОНКРЕТНОЙ ПЛАТЫ ESP32-C3!) ----
#define PIN_CLK   4   // TM1637 CLK тут должно быть 4
#define PIN_DIO   5   // TM1637 DIO тут должно быть 5
#define PIN_BTN   6   // KY-004 сигнал (S)
#define PIN_BUZZ  7   // Активный зуммер, сигнальный вход

// Если ваш KY-004 при нажатии выдаёт HIGH вместо LOW — смените true<->false здесь:
const bool BUTTON_ACTIVE_LOW = true;

TM1637Display display(PIN_CLK, PIN_DIO);

uint16_t hitCount = 0;

// --- антидребезг ---
int lastRawState = -1;
unsigned long lastChangeTime = 0;
const unsigned long DEBOUNCE_MS = 40;
bool latched = false; // не даём одному нажатию засчитаться дважды, пока кнопка не отпущена

void beep(unsigned int ms) {
  digitalWrite(PIN_BUZZ, HIGH);
  delay(ms);
  digitalWrite(PIN_BUZZ, LOW);
}

bool isPressed(int rawState) {
  return BUTTON_ACTIVE_LOW ? (rawState == LOW) : (rawState == HIGH);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_BTN, INPUT_PULLUP); // если у KY-004 нет своего подтягивающего резистора
  pinMode(PIN_BUZZ, OUTPUT);
  digitalWrite(PIN_BUZZ, LOW);

  display.setBrightness(5); // диапазон 0-7
  display.showNumberDec(hitCount, false);

  Serial.println("=== Counter node bring-up test (PlatformIO) ===");
  Serial.println("Нажимайте кнопку KY-004, чтобы имитировать попадание.");
}

void loop() {
  int raw = digitalRead(PIN_BTN);

  if (raw != lastRawState) {
    lastChangeTime = millis();
    lastRawState = raw;
  }

  if ((millis() - lastChangeTime) > DEBOUNCE_MS) {
    bool pressedNow = isPressed(raw);

    if (pressedNow && !latched) {
      latched = true;
      hitCount++;
      display.showNumberDec(hitCount, false);
      beep(80);
      Serial.print("HIT #");
      Serial.println(hitCount);
    }

    if (!pressedNow) {
      latched = false;
    }
  }
}