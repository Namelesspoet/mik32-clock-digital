
char DaysLetters[7] = { 'M', 'T', 'W', 'T', 'F', 'S', 'S' };


void saveAlarmSettings() {
  EEPROM.update(ALARM_HOUR_ADDR, alarmTime.hour());
  EEPROM.update(ALARM_MINUTE_ADDR, alarmTime.minute());
  EEPROM.update(ALARM_ENABLED_ADDR, alarmEnabled ? 1 : 0);
}

void loadAlarmSettings() {
  byte hour = EEPROM.read(ALARM_HOUR_ADDR);
  byte minute = EEPROM.read(ALARM_MINUTE_ADDR);
  alarmEnabled = EEPROM.read(ALARM_ENABLED_ADDR) == 1;
  
  // Установка времени будильника (год, месяц, день не важны)
  alarmTime = DateTime(2000, 1, 1, hour, minute, 0);
}


void enterAlarmSetMode(){
  currentMode = ALARM;
  alarmSelectMode = true;  // Режим выбора будильника (true) или настройки параметров (false)
  alarmParamSelectMode = false;
  currentAlarmParam = 0;    // Текущий параметр будильника (0-часы, 1-минуты, 2-дни, 3-режим)
  currentDaySelection = 0;  // Текущий выбранный день (для режима CUSTOM_DAYS)
  currentAlarm = 0;
  drawAlarmScreen();
}

void playMelody() {
  static int divider = 0, noteDuration = 0;
  static unsigned long noteStartTime = 0;
  static bool betweenNotes = false;
  
  if (currentNote >= notes * 2) {
    // Мелодия закончилась, начинаем сначала
    currentNote = 0;
  }
  
  if (currentNote % 2 == 0 && !betweenNotes) {
    // Начало новой ноты
    divider = melody[currentNote + 1];
    if (divider > 0) {
      noteDuration = wholenote / divider;
    } else if (divider < 0) {
      noteDuration = wholenote / abs(divider);
      noteDuration *= 1.5;
    }
    
    tone(BUZZER_PIN, melody[currentNote], noteDuration * 0.9);
    noteStartTime = millis();
    betweenNotes = true;
    currentNote += 2;
  } else if (betweenNotes && millis() - noteStartTime >= noteDuration) {
    // Пауза между нотами
    noTone(BUZZER_PIN);
    betweenNotes = false;
  }
}

void exitAlarmSetMode(){

  currentMode = WATCH;
  Datime now = rtc.getTime();
  for (int i = 0; i<4; i++){
    Alarms[i] = tempAlarms[i];
  }

  drawFull(now);

}

void drawDays(int8_t AlarmNum) {
  uint16_t Color;
  tft.setTextSize(2);
  switch (tempAlarms[AlarmNum].mode) {
    case CUSTOM_DAYS:
      for (int j = 0; j < 7; j++) {
        if (tempAlarms[AlarmNum].activeDays[j]) {
          Color = TFT_GREEN;
        } else {
          Color = TFT_RED;
        }
        tft.fillRect(120 + j * 20, 22 + AlarmNum * 60, 16, 16, Color);
        tft.setTextColor(BACK_COLOR, Color);
        tft.setCursor(120 + 3 + j * 20, 23 + AlarmNum * 60);
        tft.print(DaysLetters[j]);
        tft.fillRect(120 + j * 20, 22 + 16 + AlarmNum * 60, 16, 2, BACK_COLOR);
      }
      break;
    case DAILY:
      Color = TFT_GREEN;
      for (int j = 0; j < 7; j++) {
        tft.fillRect(120 + j * 20, 22 + AlarmNum * 60, 16, 16, Color);
        tft.setTextColor(BACK_COLOR, Color);
        tft.setCursor(120 + 3 + j * 20, 23 + AlarmNum * 60);
        tft.print(DaysLetters[j]);
        tft.fillRect(120 + j * 20, 22 + 16 + AlarmNum * 60, 16, 2, BACK_COLOR);
      }
      break;
    default:
      clearDays(AlarmNum);

  }
  tft.setTextSize(3);
  tft.setTextColor(TFT_GREEN, BACK_COLOR);
}

void clearDays(int8_t AlarmNum) {
  tft.fillRect(119, AlarmNum * 60 + 19, 142, 32, BACK_COLOR);
}

void drawDaySelection(int8_t DayNum, int8_t AlarmNum) {
  if (DayNum == 0) {
    clearDaySelection(1, AlarmNum);
    clearDaySelection(6, AlarmNum);
  } else if (DayNum == 6) {
    clearDaySelection(0, AlarmNum);
    clearDaySelection(5, AlarmNum);
  } else {
    clearDaySelection(DayNum - 1, AlarmNum);
    clearDaySelection(DayNum + 1, AlarmNum);
  }

  tft.drawRect(118 + DayNum * 20, 20 + AlarmNum * 60, 20, 20, TFT_YELLOW);
}

void clearDaySelection(int8_t DayNum, int8_t AlarmNum) {
  tft.drawRect(118 + DayNum * 20, 20 + AlarmNum * 60, 20, 20, BACK_COLOR);
}

void drawAlarmSelection(int8_t itemNum) {
  tft.drawRect(0, (itemNum - 1) * 60, 320, 60, BACK_COLOR);
  tft.drawRect(0, (itemNum + 1) * 60, 320, 60, BACK_COLOR);
  if ((itemNum == 0) or (itemNum == 3)) {
    tft.drawRect(0, (3) * 60, 320, 60, BACK_COLOR);
    tft.drawRect(0, (0) * 60, 320, 60, BACK_COLOR);
  }

  tft.drawRect(0, itemNum * 60, 320, 60, TFT_YELLOW);
}

void AlarmSetValue(int8_t itemNum, int8_t AlarmNum, int8_t delta) {
  switch (itemNum) {

    case 0:  // часы
      tempAlarms[AlarmNum].hour += delta;
      if (tempAlarms[AlarmNum].hour > 23) tempAlarms[AlarmNum].hour = 0;
      if (tempAlarms[AlarmNum].hour < 0) tempAlarms[AlarmNum].hour = 23;
      break;
    case 1:  //минуты
      tempAlarms[AlarmNum].minute += delta;
      if (tempAlarms[AlarmNum].minute > 59) tempAlarms[AlarmNum].minute = 0;
      if (tempAlarms[AlarmNum].minute < 0) tempAlarms[AlarmNum].minute = 59;
      break;
    case 2:  //дни
      break;
    case 3:  //режим
      if (delta > 0) {
        switch (tempAlarms[AlarmNum].mode) {
          case OFF:
            tempAlarms[AlarmNum].mode = ONE_TIME;
            break;
          case ONE_TIME:
            tempAlarms[AlarmNum].mode = DAILY;
            break;
          case DAILY:
            tempAlarms[AlarmNum].mode = CUSTOM_DAYS;
            break;
          case CUSTOM_DAYS:
            tempAlarms[AlarmNum].mode = OFF;
            break;
        }
      } else {
        switch (tempAlarms[AlarmNum].mode) {
          case OFF:
            tempAlarms[AlarmNum].mode = CUSTOM_DAYS;
            break;
          case ONE_TIME:
            tempAlarms[AlarmNum].mode = OFF;
            break;
          case DAILY:
            tempAlarms[AlarmNum].mode = ONE_TIME;
            break;
          case CUSTOM_DAYS:
            tempAlarms[AlarmNum].mode = DAILY;
            break;
        }
      }

      break;
  }
  updateSettingValue(itemNum, AlarmNum);
}

void changeSetDay(int8_t DayNum, int8_t AlarmNum) {
  tempAlarms[AlarmNum].activeDays[DayNum] = !tempAlarms[AlarmNum].activeDays[DayNum];
  updateSetDay(DayNum, AlarmNum);
}

void updateSetDay(int8_t DayNum, int8_t AlarmNum) {
  tft.setTextSize(2);
  if (tempAlarms[AlarmNum].activeDays[DayNum]) {
    tft.fillRect(120 + DayNum * 20, 22 + AlarmNum * 60, 16, 16, TFT_GREEN);
    tft.setTextColor(BACK_COLOR, TFT_GREEN);
    tft.setCursor(120 + 3 + DayNum * 20, 23 + AlarmNum * 60);
    tft.print(DaysLetters[DayNum]);
    tft.fillRect(120 + DayNum * 20, 22 + 16 + AlarmNum * 60, 16, 2, BACK_COLOR);
  } else {
    tft.fillRect(120 + DayNum * 20, 22 + AlarmNum * 60, 16, 16, TFT_RED);
    tft.setTextColor(BACK_COLOR, TFT_RED);
    tft.setCursor(120 + 3 + DayNum * 20, 23 + AlarmNum * 60);
    tft.print(DaysLetters[DayNum]);
    tft.fillRect(120 + DayNum * 20, 22 + 16 + AlarmNum * 60, 16, 2, BACK_COLOR);
  }

  tft.setTextColor(TFT_GREEN, BACK_COLOR);
  tft.setTextSize(3);
}

void updateSettingValue(int8_t itemNum, int8_t AlarmNum) {
  tft.setTextSize(3);
  switch (itemNum) {
    case 0:
      {
        tft.setCursor(20, 20 + AlarmNum * 60);
        String temphour = String(tempAlarms[AlarmNum].hour);
        if (tempAlarms[AlarmNum].hour < 10) { temphour = "0" + temphour; }
        tft.print(temphour);
        break;
      }
    case 1:
      {
        tft.setCursor(75, 20 + AlarmNum * 60);
        String tempminute = String(tempAlarms[AlarmNum].minute);
        if (tempAlarms[AlarmNum].minute < 10) { tempminute = "0" + tempminute; }
        tft.print(tempminute);
        break;
      }
    case 2:
      break;
    case 3:
      {
        //tft.fillRect(266,AlarmNum*60 + 16,34,27,BACK_COLOR);

        tft.setCursor(270, 23 + AlarmNum * 60);
        tft.setTextSize(2);
        switch (tempAlarms[AlarmNum].mode) {
          case 0:
            tft.setTextColor(TFT_RED, BACK_COLOR);

            tft.print("OFF");
            tft.setTextColor(FONT_COLOR, BACK_COLOR);
            drawDays(AlarmNum);
            break;
          case 1:
            tft.print("ONE");
            drawDays(AlarmNum);
            break;
          case 2:
            tft.print("DAY");
            drawDays(AlarmNum);
            break;
          case 3:
            tft.print("CST");
            drawDays(AlarmNum);
            break;
        }
        tft.setTextSize(3);
        break;
      }
  }
}

void drawAlarmSetSelection(int8_t itemNum, int8_t AlarmNum) {

  switch (itemNum) {

    case 0:  // часы
      clearAlarmSetSelection(3, AlarmNum);
      clearAlarmSetSelection(1, AlarmNum);
      tft.drawRect(15, AlarmNum * 60 + 15, 45, 30, TFT_YELLOW);
      break;
    case 1:  //минуты
      clearAlarmSetSelection(0, AlarmNum);
      clearAlarmSetSelection(2, AlarmNum);
      clearAlarmSetSelection(3, AlarmNum);
      tft.drawRect(70, AlarmNum * 60 + 15, 45, 30, TFT_YELLOW);
      break;
    case 2:  //дни
      clearAlarmSetSelection(1, AlarmNum);
      clearAlarmSetSelection(3, AlarmNum);
      tft.drawRect(113, AlarmNum * 60 + 15, 150, 30, TFT_YELLOW);
      break;
    case 3:  //режим
      clearAlarmSetSelection(2, AlarmNum);
      clearAlarmSetSelection(1, AlarmNum);
      clearAlarmSetSelection(0, AlarmNum);
      tft.drawRect(266, AlarmNum * 60 + 15, 42, 30, TFT_YELLOW);
      break;
  }
}

void clearAlarmSetSelection(int8_t itemNum, int8_t AlarmNum) {

  switch (itemNum) {

    case 0:  // часы
      tft.drawRect(15, AlarmNum * 60 + 15, 45, 30, BACK_COLOR);
      break;
    case 1:  //минуты
      tft.drawRect(70, AlarmNum * 60 + 15, 45, 30, BACK_COLOR);
      break;
    case 2:  //дни
      tft.drawRect(113, AlarmNum * 60 + 15, 150, 30, BACK_COLOR);
      break;
    case 3:  //режим
      tft.drawRect(266, AlarmNum * 60 + 15, 42, 30, BACK_COLOR);
      break;
  }
}

void drawAlarmScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_GREEN, BACK_COLOR);
  for (int i = 0; i < 4; i++) {
    tempAlarms[i] = Alarms[i];
    tft.setCursor(20, 20 + i * 60);
    tft.setTextSize(3);

    String hour = String(Alarms[i].hour);
    if (Alarms[i].hour < 10) { hour = "0" + hour; }
    tft.print(hour);
    tft.setCursor(55, 20 + i * 60);
    tft.print(":");
    tft.setCursor(75, 20 + i * 60);
    String minute = String(Alarms[i].minute);
    if (Alarms[i].minute < 10) { minute = "0" + minute; }
    tft.print(minute);

    tft.setCursor(210, 20 + i * 60);
    tft.setTextSize(2);

    if (Alarms[i].mode == CUSTOM_DAYS) {
      for (int j = 0; j < 7; j++) {
        if (Alarms[i].activeDays[j]) {
          tft.fillRect(120 + j * 20, 22 + i * 60, 16, 16, TFT_GREEN);
          tft.setTextColor(BACK_COLOR, TFT_GREEN);
          tft.setCursor(120 + 3 + j * 20, 23 + i * 60);
          tft.print(DaysLetters[j]);
          tft.fillRect(120 + j * 20, 22 + 16 + i * 60, 16, 2, BACK_COLOR);
        } else {
          tft.fillRect(120 + j * 20, 22 + i * 60, 16, 16, TFT_RED);
          tft.setTextColor(BACK_COLOR, TFT_RED);
          tft.setCursor(120 + 3 + j * 20, 23 + i * 60);
          tft.print(DaysLetters[j]);
          tft.fillRect(120 + j * 20, 22 + 16 + i * 60, 16, 2, BACK_COLOR);
        }
      }
      tft.setTextColor(TFT_GREEN, BACK_COLOR);
      tft.setTextSize(3);
    }
    tft.setCursor(270, 23 + i * 60);
    tft.setTextSize(2);
    switch (Alarms[i].mode) {
      case 0:
        tft.setTextColor(TFT_RED, BACK_COLOR);

        tft.print("OFF");
        tft.setTextColor(FONT_COLOR, BACK_COLOR);
        drawDays(i);
        break;
      case 1:
        tft.print("ONE");
        drawDays(i);
        break;
      case 2:
        tft.print("DAY");
        drawDays(i);
        break;
      case 3:
        tft.print("CST");
        drawDays(i);
        break;
    }
    tft.setTextSize(3);
  }
  drawAlarmSelection(0);
}