void setup() {
    pinMode(12, OUTPUT);
}

void loop() {
    digitalWrite(12, (millis() % 1000 > 500));
}
