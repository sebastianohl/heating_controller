# Homie Heating Controller for an Underfloor Heating

This software provides a firmware for a ESP32 Node MCU to controller a 3-point actuator for an underfloor heating.

It uses DB18B20 sensors to read up to four temperatures from your fluid system. The goal of the controller is to reduce the inflow temperature of the underfloow heating.

The uC is controllable via MQTT over WiFi and implements the Homie protocol to integrate into a home automation system like openhab >=2.5.

The PCB can be found in the repository.

Needed Hardware:
* e.g. FUSSBODENREGELEINHEIT EASYFLOW MIX, MISCHER GRUNDFOS UPM3 HYBRID 15-70
* e.g. Stellmotor Typ 230-473 für Mischer
* EPS32 NodeMCU

Configuration:
* use idy.py menuconfig to set the RomCodes of your DB18B20 sensors and your other parameters (e.g. WiFi Password, MQTT Settings...)

Software:
* ESP-IDF Package
* vscode (or any other IDE)

JTAG Debugging:
use a adapter FTDI 2232HL board.
Connect the pins as following:
* GPIO12 — AD1 (TDI)
* GPIO15 — AD2 (TDO)
* GPIO13 — AD0 (TCK)
* GPIO14 — AD3 (TMS)
* GND — GND

