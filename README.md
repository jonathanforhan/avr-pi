# avr-pi

ATmega328P emulator enabling Arduino code to run on a Raspberry Pi

## Build

All build command should be run from project root directory

### Dependencies

```bash
sudo apt-get install build-essential cmake ninja-build arduino-cli
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

It is recommended include avr-pi as a subdirectory and simply call

```cmake
add_subdirectory("avr-pi")
target_link_libraries(${YOUR_PROJECT} PRIVATE avr-pi-lib) # public headers are automatically included
```
