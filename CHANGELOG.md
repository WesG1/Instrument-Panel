Change Log
09/01/26
    Fixed missing tasks in setup. Deleted comments from old code that have been completed in RTOS

08/31/26
    Removed chase animation. Added backlight LED when headlights are ON. Added flasher functionality to turn signal LEDs

08/22/26
    Attempted fix for potentiometer controls

08/20/26
    Added isr interrupts for headlight switch leds

08/18/26
    Removed chase animation. Added headlight indication leds

06/04/26
    Changed code to use rtos instead of super loop format

04/16/26
    Added templates for display, and fault detection functions. Deleted unused function. Added mileage variables

04/13/26
    Updated comments

04/03/26
    Fixed errors, noted Volts function needs rewrite

03/31/26
    Changed pot variables to sense. Added values for voltage divider code. Changed servo code to not use PCA chips. Added sensor calibration charts.

03/12/2026
    Added new servo attach code. Added EEPROM read and write functions

03/11/2026
    Removed Adafruit PWM servo code, removed I2C LCD code, removed code of RGB leds since ARGB leds will be used instead, changed SDA and SCL pins to match schematic, added code for new LCD pins, added code for speaker and ultrasonic sensor, changed servo pins to connect directly to esp32, started power on off function

03/03/26
    Added note in code to reflect changes in hardware. Code has not yet been updated.

02/24/26
    Added definitions for high beams, left and right turn signals, and brakes. changed pins for power

02/12/26
    Defined second i2c pwm driver for LEDs, and added comments for feuture parts to be used

02/09/26
    Defined pins for lights and power detection/switching

01/26/26:
    Added code to git