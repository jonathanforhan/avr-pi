void setup() {
    pinMode(12, OUTPUT);
}

void loop() {
    digitalWrite(12, (millis() % 2000 > 1000));
}
