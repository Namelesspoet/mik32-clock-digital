void drawHumidityIcon(TFT_eSPI &tft, int size, int x, int y) {
    const uint16_t color = TFT_WHITE;
    const uint16_t bg_color = TFT_BLACK;
    const float scale = size / 32.0f;

    const int centerX = x + size / 2;
    const int centerY = y + size / 2;

    const int dropWidth = 16 * scale;
    const int dropHeight = 26 * scale;
    const int circleRadius = 9 * scale;

    const int circleCenterY = centerY + dropHeight / 2 - circleRadius;
    const int triangleTop = circleCenterY - dropHeight + circleRadius;

    tft.fillCircle(centerX, circleCenterY, circleRadius, color);

    tft.fillTriangle(
        centerX - dropWidth / 2, triangleTop + dropHeight / 2,
        centerX + dropWidth / 2, triangleTop + dropHeight / 2,
        centerX, triangleTop,
        color
    );
}

void drawThermometerIcon(TFT_eSPI &tft, int size, int x, int y) {
    const uint16_t color = TFT_WHITE;
    const uint16_t bg_color = TFT_BLACK;
    const float scale = size / 32.0f;

    const int centerX = x + size / 2;
    const int centerY = y + size / 2;

    const int bulbDiameter = 12 * scale;
    const int stemWidth = 6 * scale;
    const int stemHeight = 18 * scale;
    const int numMarkings = 3;

    const int bulbCenterY = centerY + size / 4 + 2 * scale;
    const int stemTop = bulbCenterY - stemHeight - bulbDiameter / 2;

    tft.drawCircle(centerX, bulbCenterY - 1 * scale, bulbDiameter / 2, color);
    tft.fillCircle(centerX, bulbCenterY - 1 * scale, bulbDiameter / 2 - 2, color);

    tft.drawRect(centerX - stemWidth / 2, stemTop, stemWidth, stemHeight, color);

    tft.fillRect(centerX - stemWidth / 2 + 1, stemTop + stemHeight - scale, stemWidth - 2, 2 * scale, bg_color);

    const int markingSpacing = stemHeight / (numMarkings + 1);
    for (int i = 1; i <= numMarkings; i++) {
        int markY = stemTop + i * markingSpacing;
        tft.drawLine(centerX - stemWidth / 2, markY, centerX - stemWidth / 2 + 3 * scale, markY, color);
    }
}

void drawBattery(TFT_eSPI &tft, int charge, int x, int y, bool charging, bool charged) {
 
      if (charged){
        charge = 100;
         Serial.println("Charged");
    }
    const int cellWidth = 6;
    const int spacing = 1;
    const int height = 20;
    const int segmentsCount = 4;
    const int innerWidth = segmentsCount * cellWidth + (segmentsCount + 1) * spacing;
    const int bodyWidth = innerWidth + 2;
    const int contactWidth = max(2, height / 6);
    const int contactHeight = max(4, height / 3);
    const int totalWidth = bodyWidth + contactWidth;

    tft.drawRect(x, y, bodyWidth, height, TFT_WHITE);
    tft.fillRect(x + bodyWidth, y + (height - contactHeight) / 2, contactWidth, contactHeight, TFT_WHITE);

    const int thresholdStep = 100 / segmentsCount;
    int segments = round(charge/thresholdStep);

    const int cellY = y + spacing + 1;
    const int cellHeight = height - 2 * (spacing + 1);
    int cellX = x + spacing + 1;

    for (int i = 0; i < segments; i++) {
        tft.fillRect(cellX, cellY, cellWidth, cellHeight, TFT_WHITE);
        cellX += cellWidth + spacing;
    }

    if (charging and !charged){
    Serial.println("Charging");

      tft.fillTriangle(
          x + 6,  48 - (y + 9),   // x+6,  y+15 → (286, 29)
          x + 15, 48 - (y + 11),  // x+15, y+13 → (295, 27)
          x + 17, 48 - (y + 5),   // x+17, y+19 → (297, 33)
          TFT_BLACK
      );

// Второй треугольник (отражённый по Y)
      tft.fillTriangle(
          x + 24, 48 - (y + 13),  // x+24, y+11 → (304, 25)
          x + 15, 48 - (y + 11),  // x+15, y+13 → (295, 27)
          x + 13, 48 - (y + 16),  // x+13, y+8  → (293, 22)
          TFT_BLACK
      );
    }
}
