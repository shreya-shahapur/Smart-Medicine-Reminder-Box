#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
Servo myServo;

// Buttons
int btnSet = 2;
int btnInc = 3;
int btnDec = 4;
int btnNext = 5;

// Outputs
int whiteLed = 10;
int blueLed = 9;
int buzzer = 8;

// IR sensor
int irSensor = 7;

// Servo
int servoPin = 6;

// Alarm storage
int alarmHour[3];
int alarmMin[3];
bool alarmTriggered[3] = {false, false, false};

int currentAlarm = 0;
bool settingMode = false;

void setup() {
  pinMode(btnSet, INPUT_PULLUP);
  pinMode(btnInc, INPUT_PULLUP);
  pinMode(btnDec, INPUT_PULLUP);
  pinMode(btnNext, INPUT_PULLUP);

  pinMode(whiteLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(irSensor, INPUT);

  digitalWrite(buzzer, HIGH);

  myServo.attach(servoPin);
  myServo.write(0);

  lcd.init();
  lcd.backlight();

  rtc.begin();

  // USE ONCE THEN COMMENT
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  digitalWrite(whiteLed, HIGH);
}

void loop() {

  DateTime now = rtc.now();
  int h = now.hour();
  int m = now.minute();
  int d = now.day();
  int mo = now.month();
  int y = now.year();

  // ===== SET BUTTON =====
  if (digitalRead(btnSet) == LOW) {
    delay(300);

    if (!settingMode) {
      alarmHour[currentAlarm] = h;
      alarmMin[currentAlarm] = m;
    }

    settingMode = !settingMode;
    lcd.clear();

    if (!settingMode) {
      lcd.setCursor(0, 0);
      lcd.print("ALARM IS SET");
      delay(1500);
      lcd.clear();
    }
  }

  // ================= NORMAL MODE =================
  if (!settingMode) {

    lcd.setCursor(0, 0);
    lcd.print(d);
    lcd.print("/");
    lcd.print(mo);
    lcd.print(" ");

    lcd.print(h);
    lcd.print(":");
    if (m < 10) lcd.print("0");
    lcd.print(m);
    lcd.print("   ");

    lcd.setCursor(0, 1);
    lcd.print("Waiting...     ");

    digitalWrite(whiteLed, HIGH);
    digitalWrite(blueLed, LOW);

    // ===== CHECK ALARMS =====
    for (int i = 0; i < 3; i++) {

      if (h == alarmHour[i] && m == alarmMin[i] && !alarmTriggered[i]) {

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Take Medicine");

        digitalWrite(whiteLed, LOW);
        digitalWrite(blueLed, HIGH);

        // SERVO ROTATE ONCE
        myServo.write(180);
        delay(2000);
        myServo.write(0);

        // BUZZER 15 TIMES
        for (int j = 0; j < 15; j++) {
          digitalWrite(buzzer, LOW);
          delay(80);
          digitalWrite(buzzer, HIGH);
          delay(250);
        }

        // 🔴 IMPORTANT FIX: WAIT FOR ACTUAL CHANGE
        int initialState = digitalRead(irSensor);

        while (true) {
          int currentState = digitalRead(irSensor);

          if (currentState != initialState) {
            delay(200); // debounce

            // confirm stable change
            if (digitalRead(irSensor) == currentState) {
              break;
            }
          }
        }

        // MEDICINE TAKEN
        digitalWrite(buzzer, HIGH);
        digitalWrite(blueLed, LOW);
        digitalWrite(whiteLed, HIGH);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Medicine Taken");
        delay(2000);

        alarmTriggered[i] = true;
        lcd.clear();
      }
    }
  }

  // ================= SET MODE =================
  else {

    lcd.setCursor(0, 0);
    lcd.print("SET REMINDER   ");

    lcd.setCursor(0, 1);
    lcd.print("T");
    lcd.print(currentAlarm + 1);
    lcd.print(": ");

    lcd.print(alarmHour[currentAlarm]);
    lcd.print(":");
    if (alarmMin[currentAlarm] < 10) lcd.print("0");
    lcd.print(alarmMin[currentAlarm]);
    lcd.print("   ");

    if (digitalRead(btnInc) == LOW) {
      alarmMin[currentAlarm] = (alarmMin[currentAlarm] + 1) % 60;
      delay(200);
    }

    if (digitalRead(btnDec) == LOW) {
      alarmMin[currentAlarm]--;
      if (alarmMin[currentAlarm] < 0) alarmMin[currentAlarm] = 59;
      delay(200);
    }

    if (digitalRead(btnNext) == LOW) {
      currentAlarm = (currentAlarm + 1) % 3;

      alarmHour[currentAlarm] = h;
      alarmMin[currentAlarm] = m;

      delay(300);
    }
  }

  delay(150);
}