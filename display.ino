
void drawFull(Datime now) {
  Serial.println("drawFull");
  tft.fillScreen(TFT_BLACK);
  drawThermometerIcon(tft, 30, 0, 8);

  drawHumidityIcon(tft, 30, 135, 8);
  printHum();
  printTemp();
  drawBattery(tft, analogRead(A0)*2*100/1024, 280, 14,digitalRead(A1),digitalRead(A2));
  redrawDate(now);
  redrawHour(now);
  
  tft.setTextDatum(0);
  tft.setTextSize(2);
}

void testupdate(Datime now, Datime last){

  if ((now.day!=last.day) or (now.month!=last.month) or (now.year!=last.year)){redrawDate(now);}
  if ((now.hour!=last.hour) or (now.minute!=last.minute)) {redrawHour(now);}
  printHum();
  printTemp();
  Serial.println(analogRead(A0));
  drawBattery(tft, analogRead(A0)*1.5*100/1024, 280, 14,digitalRead(A1),digitalRead(A2));
} 

void redrawDate(Datime now){
  
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(4);
  tft.fillRect(0,40,360,60, TFT_BLACK);
  tft.drawString(daysOfWeek[now.weekDay], 160, 70);
  
  tft.drawString(String(now.day) + " " + String(months[now.month - 1]) + " " + String(now.year), 160, 110);

  tft.setTextSize(2);
  tft.setTextDatum(0);
}

void redrawHour(Datime now){
  tft.setTextSize(2);
  int xpos1 = 20;
  int ypos1 = 130;
  if (now.hour < 10) {xpos1 += tft.drawChar('0', xpos1, ypos1, FONT);}
  
  xpos1 += tft.drawNumber(now.hour, xpos1, ypos1, FONT);
  tft.setTextColor(BACK_COLOR, BACK_COLOR);
  xpos1 += 24;
  tft.setTextColor(FONT_COLOR, BACK_COLOR);
  if (now.minute < 10) {xpos1 += tft.drawChar('0', xpos1, ypos1, FONT);}
  xpos1 += tft.drawNumber(now.minute, xpos1, ypos1, FONT);
  printTemp();
  printHum();
}

bool drowDots(bool back){
  tft.setTextDatum(0);
  tft.setTextSize(2);
   if (back) {
    tft.setTextColor(BACK_COLOR, BACK_COLOR);
    tft.drawChar(':', 148, 130 - 8, FONT);
    tft.setTextColor(FONT_COLOR, BACK_COLOR);
  } else {
    tft.drawChar(':', 148, 130 - 8, FONT);
  }
  return (!back);
}

void printHum(){
  tft.setTextSize(3);
  tft.setCursor(170, 14);
  tft.print(getHumidity(), 1);
  tft.print("%");
}

void printTemp(){
  tft.setCursor(30, 14);
  tft.setTextSize(3);
  tft.print(getTemperature(), 1);
  tft.print(" C");
}