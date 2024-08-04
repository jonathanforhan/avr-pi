# avr-pi

ATmega328P emulator enabling Arduino code to run on a Raspberry Pi

## Supported Features

- Entire AVR instruction set supported by the ATmega328P
- Interrupts
- PWM output pins
- Accurate simulated frequency of up to 16MHz

## Planned Features

- Pin mapping to Raspberry Pi GPIO
- USART to stdout/stdin
- SPI and I2C
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

## Build

### Dependencies

```bash
sudo apt-get install build-essential cmake ninja-build arduino-cli
```

```bash
wget https://github.com/joan2937/pigpio/archive/master.zip
unzip master.zip
cd pigpio-master
make
sudo make install
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
