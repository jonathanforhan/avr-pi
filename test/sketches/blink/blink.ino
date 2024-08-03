void setup() {
    pinMode(12, OUTPUT);
}

void loop() {
    if (millis() % 2000 > 1000) {
        digitalWrite(12, HIGH);
    } else {
        digitalWrite(12, LOW);
    }
}
