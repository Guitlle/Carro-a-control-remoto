# Carro a control remoto con la ESP32

Tenía un par de placas con ESP32 por ahí y me puse a aprender y desempolvar el C++ con este proyecto. Todo muy fácil gracias a las librerías que la comunidad mantiene. La ESP32 se conecta al WiFi, pone un servidor HTTP en el puerto 80 que muestra una UI simple para controlar el carro desde algún dispositivo conectado a la misma red. Para el hardware utilicé dos motores DC y un servo. El firmware activa dos canales con PWM, uno para los dos motores y el otro para el servo que maneja la dirección de la tercera llanta. El PWM de los motores conecta con un módulo ULN2003AN, que se alimenta de la batería para no hacer pasar mucha corriente por el micro por el pin 33 y el servo con el pin 32.

#### TODO
* Usar un sistema de archivos para el servidor web (probé LittleFS y parece que ya no está soportado por Arduino ESP32)
* Enviar la salida de los logs vía Web para poder debuggear remotamente
* Aprender a actualizar vía OTA
* Agregar un puente H para la reversa
* Averiguar eso de proteger el micro de los picos de voltaje de los motores
* ¿Usar la consola de Esp-IDF? ¿Vía Web?
* ...

## Build

La compilación se hace con `pio run`, como dice en la documentación de PlatformIO, y debe verse algo así:

```
Processing heltec_wifi_lora_32_V2 (platform: espressif32; board: heltec_wifi_lora_32_V2; framework: arduino)
-------------------------------------------------------------------------------
Verbose mode can be enabled via `-v, --verbose` option
CONFIGURATION: https://docs.platformio.org/page/boards/espressif32/heltec_wifi_lora_32_V2.html
PLATFORM: Espressif 32 (5.2.0) > Heltec WiFi LoRa 32 (V2)
HARDWARE: ESP32 240MHz, 320KB RAM, 8MB Flash
DEBUG: Current (cmsis-dap) External (cmsis-dap, esp-bridge, esp-prog, iot-bus-jtag, jlink, minimodule, olimex-arm-usb-ocd, olimex-arm-usb-ocd-h, olimex-arm-usb-tiny-h, olimex-jtag-tiny, tumpa)
PACKAGES:
 - framework-arduinoespressif32 @ 3.20005.220925 (2.0.5)
 - tool-esptoolpy @ 1.40201.0 (4.2.1)
 - toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch3
LDF: Library Dependency Finder -> https://bit.ly/configure-pio-ldf
LDF Modes: Finder ~ chain, Compatibility ~ soft
Found 36 compatible libraries
Scanning dependencies...
Dependency Graph
|-- ESP Async WebServer @ 1.2.3
|   |-- AsyncTCP @ 1.1.1
|   |-- FS @ 2.0.0
|   |-- WiFi @ 2.0.0
|-- Heltec ESP32 Dev-Boards @ 1.1.1
|   |-- SPI @ 2.0.0
|   |-- Wire @ 2.0.0
|-- WiFi @ 2.0.0
Building in release mode
Compiling .pio/build/heltec_wifi_lora_32_V2/src/main.cpp.o
Retrieving maximum program size .pio/build/heltec_wifi_lora_32_V2/firmware.elf
Checking size .pio/build/heltec_wifi_lora_32_V2/firmware.elf
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [=         ]  11.8% (used 38576 bytes from 327680 bytes)
Flash: [==        ]  23.9% (used 797161 bytes from 3342336 bytes)
========================= [SUCCESS] Took 6.37 seconds =========================
```
