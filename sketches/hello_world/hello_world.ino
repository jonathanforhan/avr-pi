static const int z = 1;
static int c = 0;

int add(int a, int b) {
    return a + b + z;
}

void setup() {
    Serial.begin(9600);
}

void loop() {
    for (int i = 0; i < 20; i++) {
        c += add(i, i);
    }

    Serial.println(c);

    delay(1000);
}

// -g -Os -w -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10607 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR
