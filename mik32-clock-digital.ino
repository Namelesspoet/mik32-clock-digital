#include <TFT_eSPI.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <GyverDS3231.h>
#include <GyverEncoder.h>
#include <DHT_U.h>
// Конфигурация энкодера
#define CLK_PIN 7
#define DT_PIN 6
#define SW_PIN 5
Encoder enc1(CLK_PIN, DT_PIN, SW_PIN);

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

#define TFT_GREY 0x5AEB
#define FONT 7
#define FONT_COLOR TFT_GREEN
#define BACK_COLOR TFT_BLACK
#define SELECT_COLOR TFT_YELLOW
TFT_eSPI tft = TFT_eSPI();

uint32_t targetTime = 0;
static uint8_t conv2d(const char* p);

GyverDS3231 rtc;

const char* months[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

const char* daysOfWeek[] = {
  "NONE", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"
};




// Переменные для режима настройки
enum settingMode {
  WATCH,
  SETTINGS,
  ALARM
};
settingMode currentMode = WATCH;
//bool settingMode = false;
bool AlarmsettingMode = false;
bool selectMode = true;  // Начинаем с выбора параметра
uint8_t currentSetting = 0;
const char* settingNames[] = { "Hours", "Minutes", "Seconds", "Day", "Month", "Year" };
int8_t tempHours, tempMinutes, tempSeconds, tempDay, tempMonth;
int16_t tempYear;  // Изменено на int16_t для корректного отображения года
bool needRedraw = false;

bool isAdjusting = false;
uint8_t currentAlarm = 0;
bool alarmSelectMode = true;       // Режим выбора будильника (true) или настройки параметров (false)
bool alarmParamSelectMode = true;  //режим выбора настройки будильника (true) или настройка параметра будильника (false)
int8_t currentAlarmParam = 0;      // Текущий параметр будильника (0-часы, 1-минуты, 2-дни, 3-режим)
int8_t currentDaySelection = 0;    // Текущий выбранный день (для режима CUSTOM_DAYS)


enum AlarmMode {
  OFF,
  ONE_TIME,    // 0 - однократный
  DAILY,       // 1 - ежедневный
  CUSTOM_DAYS  // 2 - выборочные дни

};

struct Alarm {
  int8_t hour;         // Час (0-23)
  int8_t minute;       // Минута (0-59)
  AlarmMode mode;      // Режим (из enum)
  bool activeDays[7];  // Битовая маска дней (если режим CUSTOM_DAYS)
};


Alarm Alarms[4] = {
  { 8, 0, OFF, { false, false, true, false, false, false, false } },          // Будильник 1
  { 9, 30, ONE_TIME, { false, false, false, false, false, false, false } },   // Будильник 2
  { 15, 55, DAILY, { true, true, true, true, true, true, true } },            // Будильник 3
  { 8, 0, CUSTOM_DAYS, { false, false, false, false, false, false, false } }  // Будильник 4
};

Alarm tempAlarms[4] = {
  { 8, 0, OFF, { false, false, true, false, false, false, false } },          // Будильник 1
  { 9, 30, ONE_TIME, { false, false, false, false, false, false, false } },   // Будильник 2
  { 15, 55, DAILY, { true, true, true, true, true, true, true } },            // Будильник 3
  { 8, 0, CUSTOM_DAYS, { false, false, false, false, false, false, false } }  // Будильник 4
};

enum AlarmEditMode { SELECT_ALARM,
                     SELECT_PARAM,
                     EDIT_PARAM,
                     EDIT_DAYS };
AlarmEditMode aMode = SELECT_ALARM;

Datime now;
Datime last;

bool dots = 1;

bool charging = 0;
bool charged= 0;


void setup(void) {

  pinMode(A0, INPUT);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, BACK_COLOR);

  dht.begin();


  Wire.begin();
  rtc.begin();
  //rtc.setBuildTime();

  // Настройка энкодера
  enc1.setType(TYPE2);
  enc1.setFastTimeout(100);
  enc1.setTickMode(MANUAL);

  targetTime = millis() + 1000;
  last = rtc.getTime();
  drawFull(last);

}


void loop() {
  enc1.tick();
  const bool isHeld = enc1.isHolded();
  const bool isDouble = enc1.isDouble();
  const bool isClick = enc1.isClick();
  const bool isTurn = enc1.isTurn();
  const bool isRight = isTurn && enc1.isRight();
  const bool isLeft = isTurn && enc1.isLeft();
  const bool isTick  = rtc.tick();
  //Serial.println(isTick);

  // Обработка режимов по удержанию кнопки
  if (isHeld) {
    if (currentMode == SETTINGS) {
      last = exitSettingMode();
    } else if (currentMode == WATCH) {
      enterSettingMode();
    }
  }

  // Обработка двойного клика
  if (isDouble && currentMode == WATCH) {
    enterAlarmSetMode();
  }

  // Обработка режима будильника
  if (currentMode == ALARM) {

    switch (aMode) {
      case SELECT_ALARM:
        if (isTurn) {
          currentAlarm = (currentAlarm + (isRight ? 1 : 3)) % 4;
          drawAlarmSelection(currentAlarm);
        }
        break;
      case SELECT_PARAM:
        if (isTurn) {

          if ((tempAlarms[currentAlarm].mode != CUSTOM_DAYS) and (currentAlarmParam == 1) and isRight) {
            currentAlarmParam = (currentAlarmParam + 2) % 4;
          } else if ((tempAlarms[currentAlarm].mode != CUSTOM_DAYS) and (currentAlarmParam == 3) and isLeft) {
            currentAlarmParam = (currentAlarmParam + 2) % 4;
          } else {
            currentAlarmParam = (currentAlarmParam + (isRight ? 1 : 3)) % 4;
          }

          drawAlarmSetSelection(currentAlarmParam, currentAlarm);
        }
        break;
      case EDIT_PARAM:
        if (isTurn) {
          switch (currentAlarmParam) {
            case 2:

              break;
            default:
              {
                int8_t delta = (isRight ? 1 : -1);
                AlarmSetValue(currentAlarmParam, currentAlarm, delta);
                break;
              }
          }
        }
        break;
      case EDIT_DAYS:
        if (isTurn) {
          currentDaySelection = (currentDaySelection + (isRight ? 1 : 6)) % 7;
          drawDaySelection(currentDaySelection,currentAlarm);
        }
      
        break;


    }

    Serial.println(aMode);
    if (isClick) {
      switch (aMode) {
        case SELECT_ALARM:
          aMode = SELECT_PARAM;
          drawAlarmSetSelection(0,currentAlarm);
          break;
        case SELECT_PARAM:
          switch (currentAlarmParam){
            case 2:
              aMode = EDIT_DAYS;
              currentDaySelection = 0;
              drawDaySelection(currentDaySelection,currentAlarm);
              break;
            case 3:
              AlarmSetValue(currentAlarmParam, currentAlarm, 1);
              break;
            default:
              aMode = EDIT_PARAM;
              break;
          }          
          break;
        case EDIT_PARAM:
          aMode = SELECT_PARAM;
          break;
        case EDIT_DAYS:   
          changeSetDay(currentDaySelection,currentAlarm);
          drawDaySelection(currentDaySelection,currentAlarm);
          break;
      }
    }

    if (isHeld){
      switch (aMode){
        case EDIT_DAYS:
          aMode = SELECT_PARAM;
          clearDaySelection(currentDaySelection,currentAlarm);
          break;

        case SELECT_ALARM:
          exitAlarmSetMode();
          break;
        case SELECT_PARAM:
          aMode = SELECT_ALARM;
          clearAlarmSetSelection(currentAlarmParam,currentAlarm);
          currentAlarmParam = 0;
          break;
        case EDIT_PARAM:
          aMode = SELECT_PARAM;
          break;
        

      }  
    }
  }



  // Обработка режима настроек
  if (currentMode == SETTINGS) {
    if (isClick) {
      selectMode = !selectMode;
      isAdjusting = false;
      updateSettingDisplay();
    }

    if (isTurn) {
      if (selectMode) {
        uint8_t prev = currentSetting;
        currentSetting = (currentSetting + (isRight ? 1 : 5)) % 6;
        if (prev != currentSetting) {
          clearSettingSelection(prev);
          drawSettingSelection(currentSetting);
        }
      } else {
        isAdjusting = true;
        adjustSettingValue(isRight ? 1 : -1);
        isAdjusting = false;
      }
    }
  }

  // Обработка режима часов
  if (currentMode == WATCH) {

    now = rtc.getTime();
    if (rtc.tick()){
      dots = drowDots(dots);
      
    }
    if (now.second == 0) {
      testupdate(now, last);
      last = now;

    }
    if (!(digitalRead(A1)==charging) or  !(digitalRead(A2)==charged)){
      drawBattery(tft, analogRead(A0)*1.5*100/1024, 280, 14,digitalRead(A1),digitalRead(A2));
      charging = digitalRead(A1);
      charged = digitalRead(A2);

    }
  }

    if (alarmEnabled && !alarmTriggered && 
      now.hour() == alarmTime.hour() && 
      now.minute() == alarmTime.minute() && 
      now.second() == 0) {
    alarmTriggered = true;
    alarmStartTime = millis();
    currentNote = 0;
    isPlaying = true;
  }
  
  // Автоматическое отключение будильника через ALARM_DURATION
  if (alarmTriggered && millis() - alarmStartTime > ALARM_DURATION) {
    alarmTriggered = false;
    isPlaying = false;
    noTone(BUZZER_PIN);
  }
  
  // Воспроизведение мелодии будильника
  if (isPlaying) {
    playMelody();
  }
  
  // Обновление дисплея
  if (millis() - lastUpdate > 500) {
    updateDisplay(now);
    lastUpdate = millis();
  }
}

}

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}