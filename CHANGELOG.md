Change Log
09/10/26: Schematics
    Added external ADC. Added hazard logic shifter. Removed unused 3.3V converter. 

09/08/26
    Separated ReadMe and ChangeLog into separate files. Combined change logs from different branches into one file
     
09/01/26: Main Code
    Fixed missing tasks in setup. Deleted comments from old code that have been completed in RTOS

08/31/26: Main Code
    Removed chase animation. Added backlight LED when headlights are ON. Added flasher functionality to turn signal LEDs

08/22/26: Main Code
    Attempted fix for potentiometer controls

08/21/26: Schematics
    Added potentiometers to test gauges to the schematic. Changed resistor values on potentiometers to prevent accidental over voltage on gpio pins

08/20/26: Main Code
    Added isr interrupts for headlight switch leds

08/18/26: Main Code
    Removed chase animation. Added headlight indication leds

6/25/26: Schematics
    Reverted to older logic shifter design. Added single stage amplifier. Added voltage dividers for gauges.

06/04/26: Main Code
    Changed code to use rtos instead of super loop format

5/11/26: Schematics
    Updated pin assignments

04/16/26: Main Code
    Added templates for display, and fault detection functions. Deleted unused function. Added mileage variables

04/13/26: Main Code
    Updated comments

04/13/26: Schematics
    Began drawing pcb for prototype circuit. Will need to be changed once desgin is finalized

04/03/26: Main Code
    Fixed errors, noted Volts function needs rewrite

03/31/26: Main Code
    Changed pot variables to sense. Added values for voltage divider code. Changed servo code to not use PCA chips. Added sensor calibration charts.

03/12/2026: Main Code
    Added new servo attach code. Added EEPROM read and write functions

03/11/2026: Main Code
    Removed Adafruit PWM servo code, removed I2C LCD code, removed code of RGB leds since ARGB leds will be used instead, changed SDA and SCL pins to match schematic, added code for new LCD pins, added code for speaker and ultrasonic sensor, changed servo pins to connect directly to esp32, started power on off function

03/11/2026: Schematics
    Placed diodes on gate of power shut off mosfet, one wired to esp32, other wired to keyed power detection

03/09/26: Schematics
    Changed logic shifters back to older version, changed names of outputs for clarification, added shifter for echo pin

03/03/: Main Code
    Added note in code to reflect changes in hardware. Code has not yet been updated.

03/03/26: Schematics
    Removed PCA chips, added additional connectors, added trip reset button, combined LEDs to single bus, added LCD connections, added speaker connection, added ultrasonic senor connections, added connectors for lights and power, fixed pinnouts, added inputs for turn and brake lights

02/24/26: Main Code
    Added definitions for high beams, left and right turn signals, and brakes. changed pins for power

02/24/26: Schematics
    Added connectors, pins for lights, and additional LEDs

2/23/26: Schematics
    Added power flags, servo connectors, optocoupler, and mosfet

2/17/26: Schematics
    Added 3.3V and 5V power regulators. Added 5V-3.3V logic shifters. Added second PCA9685 for dedicated LED control. Added EEPROM chips

02/12/26: Main Code
    Defined second i2c pwm driver for LEDs, and added comments for feuture parts to be used

02/09/26: Main Code
    Defined pins for lights and power detection/switching

01/27/26: Schematics
    Created initial branch

01/26/26: Main Code
    Added code to git