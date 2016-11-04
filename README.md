**ESP8266 Pinewood Derby Timer**

Build an affordable WiFi-controlled Pinewood Derby timer for your Cub Scout or Boy Scout pack!

Hardware Requirements:
* ESP8266 development board (Adafruit Feather Huzzah)
* Adafruit Feather LED Alphanumeric Display (I2C)
* Micro switches (Adafruit NO game switches)
* Proto board (Adafruit Tin sized board)
* Wire block terminals (Adafruit 2.54mm x.1mm terminal blocks)
* Piezo (3v+)

Software Requirements:
* Arduino IDE
* Arduino ESP8266 board configuration
* Adafruit LEDBackpack, GFX libraries
* Derby_timer sketch


**Uploading Switch**
Pin 2 to piezo leg a
Piezo leg b to switch1 pole left/right
Switch1 pole center to ground

**Power Switch**
EN pin to switch2 pole left/right
Switch2 pole center to ground

**Inputs**
Pin 14 to micro switch to ground
Pin 12 to micro switch to ground
Pin 13 to micro switch to ground
Pin 15 to micro switch to 3v3