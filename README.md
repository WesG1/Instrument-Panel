# Instrument Panel
Description:
    Digital custom 65 mustang modern insturment panel. Uses six servo motors along with RGB LEDs to display gauge information. Controlled by an ESP32-s2. Motors and warning lights are interfaced through I2C protocall using a PCA9685 controller. Odometer and tripmeter are displayed on an I2C LCD. 
    
    Odo and trip values are stored in an EEPROM chip with a wear leveling algorithm. Odometer value is also stored in a second backup EEPROM. At startup the values of both EEPROMs are read, and if they are different the higher value is set as the current mileage. At shutdown the last stored value and current value for the mileage are compared and the current value is only stored if it is higher than the saved value.

    Each servo motor (execpt voltage) has a deticated sensor. The signals from the oil, temp, and fuel sensors are passed through a calibrated voltage divider to drop them to a level safe for the ESP32 to read. Speed and tach are a digital pulse and are passed through a logic shifter. Voltage is derived by an algorithm and read by a voltage divider from the dash power supply. Volts, oil, temp, and fuel have accompanying warning lights that turn red if the value goes out of a safe range, are off if the value is normal and the headlights are off, and white if the values is normal and the headlights are on. Temp has additionally will turn yellow if the value is too low.

Change Log
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