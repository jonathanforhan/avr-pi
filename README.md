# avr-pi

ATmega328P emulator enabling Arduino code to run on a Raspberry Pi

## Build

### Dependencies

```bash
sudo apt-get install arduino-cli 
```

### Release

outputs to build/release

```bash
make DEBUG=0
```

### Debug

outputs to build/debug

```bash
make #DEBUG=1
```
