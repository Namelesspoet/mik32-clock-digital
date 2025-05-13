void drawSettingScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(FONT_COLOR, BACK_COLOR);
  tft.drawString("Settings Mode", 80, 20);

  // Отображение всех параметров
  tft.setTextSize(2);
  
  for (uint8_t i = 0; i < 6; i++) {
    drawSettingItem(i);
  }

  // Выделение текущего элемента
  drawSettingSelection(currentSetting);
}

void enterSettingMode() {
  currentMode = SETTINGS;
  selectMode = true;
  
  Datime now = rtc.getTime();
  tempHours = now.hour;
  tempMinutes = now.minute;
  tempSeconds = now.second;
  tempDay = now.day;
  tempMonth = now.month;
  tempYear = now.year;
  currentSetting = 0;
  tft.fillScreen(TFT_BLACK);
  drawSettingScreen();
}

Datime exitSettingMode() {

  rtc.setTime(tempYear, tempMonth, tempDay, tempHours, tempMinutes, tempSeconds);
  
  currentMode = WATCH;
  selectMode = false;
  needRedraw = true;
  tft.fillScreen(TFT_BLACK);
  Datime now = rtc.getTime();
  drawFull(now);
  return now;
}
void updateSettingDisplay() {

  // Обновляем выделение текущего элемента
  drawSettingSelection(currentSetting);

}

void drawSettingItem(uint8_t itemNum) {
  int yPos = 50 + itemNum * 30;
  int xLabel = 20;
  int xValue = 180;
  
  // Отрисовка метки
  tft.setTextSize(2);
  tft.setTextColor(FONT_COLOR, BACK_COLOR);
  
  String label;
  switch (itemNum) {
    case 0: label = "Hours:"; break;
    case 1: label = "Minutes:"; break;
    case 2: label = "Seconds:"; break;
    case 3: label = "Day:"; break;
    case 4: label = "Month:"; break;
    case 5: label = "Year:"; break;
  }
  
  tft.drawString(label, xLabel, yPos);
  
  // Отрисовка значения
  String valueStr;
  switch (itemNum) {
    case 0: valueStr = (tempHours < 10 ? "0" : "") + String(tempHours); break;
    case 1: valueStr = (tempMinutes < 10 ? "0" : "") + String(tempMinutes); break;
    case 2: valueStr = (tempSeconds < 10 ? "0" : "") + String(tempSeconds); break;
    case 3: valueStr = (tempDay < 10 ? "0" : "") + String(tempDay); break;
    case 4: valueStr = (tempMonth < 10 ? "0" : "") + String(tempMonth); break;
    case 5: valueStr = String(tempYear); break;
  }
  
  tft.drawString(valueStr, xValue, yPos);
}

void drawSettingSelection(uint8_t itemNum) {
  // Пропускаем перерисовку рамки, если в режиме настройки значения
  if (isAdjusting) return;
  
  int yPos = 50 + itemNum * 30;
  int xLabel = 20;
  int xValue = 180;
  int labelWidth = tft.textWidth(itemNum == 0 ? "Hours:" : 
                               itemNum == 1 ? "Minutes:" : 
                               itemNum == 2 ? "Seconds:" : 
                               itemNum == 3 ? "Day:" : 
                               itemNum == 4 ? "Month:" : "Year:");
  int valueWidth = tft.textWidth(itemNum == 5 ? String(tempYear) : "00");
  
  if (selectMode) {
    // Рамка вокруг метки
    tft.drawRect(xLabel - 5, yPos - 4, labelWidth + 10, 26, SELECT_COLOR);
    // Стираем возможную рамку вокруг значения
    tft.drawRect(xValue - 5, yPos - 4, valueWidth + 10, 26, BACK_COLOR);
  } else {
    // Рамка вокруг значения
    tft.drawRect(xValue - 5, yPos - 4, valueWidth + 10, 26, SELECT_COLOR);
    // Стираем рамку вокруг метк
    tft.drawRect(xLabel - 5, yPos - 4, labelWidth + 10, 26, BACK_COLOR);
  }
}

void clearSettingSelection(uint8_t itemNum) {
  int yPos = 50 + itemNum * 30;
  int xLabel = 20;
  int xValue = 180;
  int labelWidth = tft.textWidth(itemNum == 0 ? "Hours:" : 
                               itemNum == 1 ? "Minutes:" : 
                               itemNum == 2 ? "Seconds:" : 
                               itemNum == 3 ? "Day:" : 
                               itemNum == 4 ? "Month:" : "Year:");
  int valueWidth = tft.textWidth(itemNum == 5 ? String(tempYear) : "00");
  
  // Стираем обе возможные рамки
  tft.drawRect(xLabel - 5, yPos - 4, labelWidth + 10, 26, BACK_COLOR);
  tft.drawRect(xValue - 5, yPos - 4, valueWidth + 10, 26, BACK_COLOR);
}

void adjustSettingValue(int8_t delta) {
  switch (currentSetting) {
    case 0: // Часы
      tempHours += delta;
      if (tempHours > 23) tempHours = 0;
      if (tempHours < 0) tempHours = 23;
      break;
    case 1: // Минуты
      tempMinutes += delta;
      if (tempMinutes > 59) tempMinutes = 0;
      if (tempMinutes < 0) tempMinutes = 59;
      break;
    case 2: // Секунды
      tempSeconds += delta;
      if (tempSeconds > 59) tempSeconds = 0;
      if (tempSeconds < 0) tempSeconds = 59;
      break;
    case 3: // День
      tempDay += delta;
      if (tempDay > 31) tempDay = 1;
      if (tempDay < 1) tempDay = 31;
      break;
    case 4: // Месяц
      tempMonth += delta;
      if (tempMonth > 12) tempMonth = 1;
      if (tempMonth < 1) tempMonth = 12;
      break;
    case 5: // Год
      tempYear += delta;
      if (tempYear > 2099) tempYear = 2000;
      if (tempYear < 2000) tempYear = 2099;
      break;
  }
  
  updateSettingValue(currentSetting);
}

void updateSettingValue(uint8_t itemNum) {
  const int yPos = 50 + itemNum * 30;
  const int xValue = 180;
  const int charHeight = 20;
  const int charWidth = tft.textWidth("0");
  
  // Определяем параметры для каждого типа значения с правильными типами указателей
  struct ValueParams {
    void* valuePtr;  // Универсальный указатель
    int minVal;
    int maxVal;
    bool leadingZero;
    uint8_t type; // 0 - int8_t, 1 - int16_t
  };

  ValueParams params[] = {
    {(void*)&tempHours,   0, 23,  true, 0},  // Часы (int8_t)
    {(void*)&tempMinutes, 0, 59,  true, 0},  // Минуты (int8_t)
    {(void*)&tempSeconds, 0, 59,  true, 0},  // Секунды (int8_t)
    {(void*)&tempDay,     1, 31,  true, 0},  // День (int8_t)
    {(void*)&tempMonth,   1, 12,  true, 0},  // Месяц (int8_t)
    {(void*)&tempYear, 2000, 2099, false, 1} // Год (int16_t)
  };

  ValueParams p = params[itemNum];
  int currentValue;
  
  // Получаем значение с учетом типа
  if (p.type == 0) {
    currentValue = *(int8_t*)p.valuePtr;
  } else {
    currentValue = *(int16_t*)p.valuePtr;
  }

  int digits = (itemNum == 5) ? 4 : 2; // Для года 4 цифры

  // Форматируем текущее значение
  String currentStr;
  if (p.leadingZero) {
    currentStr = currentValue < 10 ? "0" + String(currentValue) : String(currentValue);
  } else {
    currentStr = String(currentValue);
    while (currentStr.length() < digits) currentStr = " " + currentStr;
  }

  // Определяем, нужно ли перерисовывать обе цифры
  bool redrawBothDigits = false;
  
  // Проверяем граничные условия для полей с двумя цифрами
  if (digits == 2) {
    if (currentValue == p.minVal || currentValue == p.maxVal || 
        currentValue == p.minVal + 1 || currentValue == p.maxVal - 1) {
      redrawBothDigits = true;
    }
  }
  // Для года всегда перерисовываем полностью при переходе декад
  else if (itemNum == 5 && (currentValue % 100 == 99 || currentValue % 100 == 0)) {
    redrawBothDigits = true;
  }

  if (redrawBothDigits) {
    // Полная перерисовка всего значения
    tft.fillRect(xValue, yPos, digits * charWidth, charHeight, BACK_COLOR);
    tft.drawString(currentStr, xValue, yPos);
  } else {
    // Точечное обновление измененных цифр
    static int lastValues[6] = {0};
    
    if (lastValues[itemNum] != currentValue) {
      String lastStr;
      if (p.leadingZero) {
        lastStr = lastValues[itemNum] < 10 ? "0" + String(lastValues[itemNum]) : String(lastValues[itemNum]);
      } else {
        lastStr = String(lastValues[itemNum]);
        while (lastStr.length() < digits) lastStr = " " + lastStr;
      }

      for (int i = 0; i < digits; i++) {
        if (i >= lastStr.length() || i >= currentStr.length() || lastStr[i] != currentStr[i]) {
          tft.fillRect(xValue + i * charWidth, yPos, charWidth, charHeight, BACK_COLOR);
          if (i < currentStr.length() && currentStr[i] != ' ') {
            tft.drawChar(currentStr[i], xValue + i * charWidth, yPos);
          }
        }
      }
    }
    lastValues[itemNum] = currentValue;
  }
}