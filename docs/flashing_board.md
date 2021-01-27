
# Flashing board

In case of my device (MacOS Big Sur), driver was alread embedded in os, and port was `/dev/cu.usbserial-1420`

Python3 is required, and the easiest way to install it on macOS is brew:

```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew install python
```

To program, esptool was used:

```
sudo pip3 install esptool
sudo esptool.py -p /dev/cu.usbserial-1420 erase_flash
sudo esptool.py -p /dev/cu.usbserial-1420 --baud 460800 write_flash --flash_size=detect 0 ./bin/esp8266-20200911-v1.13.bin
```

To push latest code and run:

```
sudo pip3 install mpy-repl-tool
sudo python3 -m there push ./src/*.py ./ --verbose
sudo python3 -m there -i 
```
