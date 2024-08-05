# avr-pi

ATmega328P emulator enabling Arduino code to run on a Raspberry Pi

https://github.com/user-attachments/assets/39a314e8-c110-4cb7-8dd1-8153276ac464

Demo using tests/sketches/hello_world/hello_world.ino sketch

## Supported Features

- Entire AVR instruction set supported by the ATmega328P
- Interrupts
- Pin mapping to Raspberry Pi GPIO
- PWM output pins
- SPI and I2C
- Accurate simulated frequency of up to 8MHz, inaccurate simulated frequency of up to 16MHz

## Planned Features

- USART to stdout/stdin
- ADC
- EEPROM read/write
- Sleep modes

## Unsupported Features

- Watchdog timer
- Clock prescaling using CLKPS(3:0) bits, instead clock timing is set via ```AVR_MCU_CLK_SPEED``` during compilation
- External clock sources
- Brown out detection and power reduction
- IVSEL, i.e. moving interrupt vectors to different offset in flash
- Input capture

## Pinout

|UNO   |GPIO |
|:-----|:----|
| 0    | 16  |
| 1    | 17  |
| 2    | 18  |
| 3    | 19  |
| 4    | 20  |
| 5    | 21  |
| 6    | 22  |
| 7    | 23  |
| 8    | 0   |
| 9    | 1   |
| 10   | 2   |
| 11   | 3   |
| 12   | 4   |
| 13   | 5   |
| A0   | 8   |
| A1   | 9   |
| A2   | 10  |
| A3   | 11  |
| A4   | 12  |
| A5   | 13  |

## Build

### Dependencies

```bash
sudo apt-get install build-essential cmake ninja-build
```

```bash
wget https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh
bash install.sh
sudo mv ./bin/arduino-cli /usr/local/bin
arduino-cli config init
arduino-cli core update-index
arduino-cli core install arduino:avr
```

```bash
wget https://github.com/joan2937/pigpio/archive/master.zip
unzip master.zip
cd pigpio-master
make
sudo make install
```

#### Test Sketch Dependencies

```bash
arduino-cli lib install LiquidCrystal@1.0.7
```

### Release

```bash
mkdir -p build && cmake -B build -S . --preset release && cmake --build build
```

### Debug

```bash
mkdir -p build && cmake -B build -S . --preset debug && cmake --build build
```

## Building as a Library

```cmake
add_subdirectory("avr-pi")
target_link_libraries(${YOUR_PROJECT} PRIVATE avr-pi-lib) # public headers are automatically included
```
