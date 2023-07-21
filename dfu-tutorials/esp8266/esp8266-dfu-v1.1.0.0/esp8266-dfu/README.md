simplify ino file - remove boilerplate, implementation details to the shared DFU code
    - [x] firmware version functions, DFU initialization moved into library
    - use Button library to remove button handling boilerplate
memory allocation safety for J* and requests
factor DFU code into a class
JSON builder/fluent API
trace logs for debugging (compile-time and runtime)
timer/tasks 
    - https://github.com/arkhipenko/TaskScheduler
    - https://github.com/dok-net/CoopTask
    - how often to send an event
    - how often to poll for DFU (if notifications aren't added)
configurable timeouts in constructor of DFU class
    - activation class (either notification or polling)
reusable with a clear interface (remove main.h -> dfu.h) C++
an easier way to create the metadata using constexpr in C - move version info to version.h for easier version management
more re-entrant rather than tight loops. should work with the task framework or use local state.



ESP8266 board
https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/#:~:text=Install%20ESP8266%20Add%2Don%20in%20Arduino%20IDE&text=Open%20the%20Boards%20Manager.,installed%20after%20a%20few%20seconds.

Requires ESP 3.1.0


ESP8266 Huzzah
https://www.adafruit.com/product/2821?gclid=Cj0KCQiA_P6dBhD1ARIsAAGI7HBsR6Ew1ox_Fu-yVGQ8TJ158oPft3vQbBtZVS1kc3XplQeAzM2sUCIaAidhEALw_wcB

Adafruit Getting Started Guide
https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/using-arduino-ide


Filesystem overview
https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#flash-layout

OTA Memory Update Process
https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html#update-process-memory-view


## Hardware Setup
Notecarrier-F
Notecard
Adafruit ESP8266 Huzzah
