void handleEncoder() {
  if (enc1.isHolded()) {
    if (currentMode == WATCH) {
      enterSettingMode();
    } else {
      exitSettingMode();
    }
    return;
  }
  if (currentMode == WATCH) return;
  if (enc1.isClick()) {
    selectMode = !selectMode;
    isAdjusting = false; // Сбрасываем флаг настройки при переключении режима
    updateSettingDisplay();
    return;
  }

  if (enc1.isTurn()) {
    if (selectMode) {
      uint8_t prev = currentSetting;
      if (enc1.isRight()) {
        currentSetting = (currentSetting + 1) % 6;
      } else if (enc1.isLeft()) {
        currentSetting = (currentSetting + 5) % 6;
      }
      if (prev != currentSetting) {
        clearSettingSelection(prev);
        drawSettingSelection(currentSetting);
      }
    } else {
      isAdjusting = true; // Устанавливаем флаг настройки
      int8_t delta = enc1.isRight() ? 1 : -1;
      adjustSettingValue(delta);
      isAdjusting = false; // Сбрасываем флаг после изменения
    }
  }
}