void setup() {
    Serial.begin(9600);
    Serial.setTimeout(10);
    Serial.println("serial echo...");
}

void loop() {
#if 0
    if (Serial.available() > 0) {
        Serial.print("> ");
        char buf[64] = {0};
        auto size = Serial.readBytesUntil('\n', buf, 64);
        for (int i = 0; i < size; i++) {
            Serial.print(buf[i]);
        }
        Serial.print('\n');
    }
#else
    if (Serial.available() > 0) {
        Serial.print("> ");
        String s = Serial.readString();
        Serial.print(s);
    }
#endif
}
