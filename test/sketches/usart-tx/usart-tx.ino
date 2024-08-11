void setup() {
    Serial.begin(9600);
}

void loop() {
    String s("Hello, world!");
    Serial.println(s);
    delay(1000);
}
