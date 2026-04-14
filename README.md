# Instrument Panel
KiCAD schematics and PCB files for instrument pannel project

Change Log
04/13/26
    Began drawing pcb for prototype circuit. Will need to be changed once desgin is finalized

03/11/2026
    Placed diodes on gate of power shut off mosfet, one wired to esp32, other wired to keyed power detection

03/09/26
    Changed logic shifters back to older version, changed names of outputs for clarification, added shifter for echo pin

03/03/26
    Removed PCA chips, added additional connectors, added trip reset button, combined LEDs to single bus, added LCD connections, added speaker connection, added ultrasonic senor connections, added connectors for lights and power, fixed pinnouts, added inputs for turn and brake lights

02/24/26
    Added connectors, pins for lights, and additional LEDs

2/23/26
    Added power flags, servo connectors, optocoupler, and mosfet

2/17/26
    Added 3.3V and 5V power regulators. Added 5V-3.3V logic shifters. Added second PCA9685 for dedicated LED control. Added EEPROM chips

01/27/26
    Created initial branch