# AMIGA 500/1000/2000 Keyboard Interface to USB

This uses an Arduino Leonardo as a USB HID keyboard interface that will enable an Amiga keyboard to be used on a modern PC system.


Based on the code posted by Olaf on the [Arduino forums](https://forum.arduino.cc/index.php?topic=139358.0).


The Keyboard must be connected to digital inputs 8,9,10 , GND and 5V. The Power LED can be directly connected to 5V
This is the pin assignment:
```
Keyboard   Leonardo
Connector  IO           

1   KBCLK   8
2   KBDATA  9
3   KBRST   10
4   5v      5v
5   NC
6   GND     GND
7   LED1    5V
8   LED2    -
```
