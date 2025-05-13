
float getTemperature() {
  return dht.readTemperature();
}

float getHumidity() {
  return dht.readHumidity();
}