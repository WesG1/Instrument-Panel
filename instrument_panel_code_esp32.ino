/******65 Mustang Custom Modern Gauge Panel******/
/************************************************/
/**Description**/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

/******Variables and function delarations******/

//i2c addresses
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
//const int LCD_ADDR = 0x27;
//const int EEPROM_ADDR_1 = 0x50; //Primary EEPROM to store milage and trip
//const int EEPROM_ADDR_2 = 0x51; //Backuo EEPROM to store copy of mileage

//lcd
//LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

//I2C pins for ESP32-S2
#define SDA_PIN 33
#define SCL_PIN 34

//Servo and light channels on PCA9685
#define SERVO_BATT 0
#define SERVO_TEMP 1
#define SERVO_TACH 2
#define SERVO_SPD 3
#define SERVO_FUEL 4
#define SERVO_OIL 5
#define BATT_R 8
#define BATT_G 9
#define TEMP_R 10
#define TEMP_G 11
#define FUEL_R 12
#define FUEL_G 13
#define OIL_R 14
#define OIL_G 15

//Servo settings
#define SERVO_FREQ 50
#define SERVOMIN 150
#define SERVOMAX 600

//Gauge ranges (in degrees 0-180)
#define OILMAX 29
#define OILMIN 118
#define FUELMAX 20
#define FUELMIN 116
#define BATTMAX 15
#define BATTMIN 97
#define TEMPMAX 0
#define TEMPMIN 115
#define SPDMAX 17
#define SPDMIN 100
#define TACHMAX 20
#define TACHMIN 95

//ESP32 analog pins
//Change these to sensors with correct ranges
const int OilPot = 6;
const int FuelPot = 5;
const int BattPot = 4;
const int TempPot = 3;
const int SpdPot = 2;
const int TachPot = 1;

//Additional I/O Pins
const int LightsOnOff = 8;
const int SwtichedSense = 9;
const int PowerOnOff = 10;

//Raw ADC values
int OilVal;
int FuelVal;
int BattVal;
int TempVal;
int SpdVal;
int TachVal;

//Gauge values (mapped angles)
int OilPress;
int FuelLevel;
int RPM;
int Speed;
int Volts;
int Temp;

//Digital values
//int vssCount = 0 //variable to store number of pulses from VSS sensor, may be changed as needed
const int trip_rest = 7;

//Function declarations
void speed();
void tach();
void temp();
void volts();
void fuelLevel();
void oilPress();
void startupAnimation();

void setup() {
  Serial.begin(115200);
  //Delay to pause startup when key is first turned
  //This insures the car is running before the startup begins
  delay(2000);
  
  //Initialize I2C with specific pins
  Serial.print("Initializing I2C on pins SDA=");
  Serial.print(SDA_PIN);
  Serial.print(", SCL=");
  Serial.println(SCL_PIN);
  Wire.begin(SDA_PIN, SCL_PIN);

  //Scan I2C bus
  //Remove once prototyping is complete
  Serial.println("Scanning I2C bus...");
  byte count = 0;
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  Found device at 0x%02X\n", i);
      count++;
    }
  }
  if (count == 0) {
    Serial.println("  ERROR: No I2C devices found!");
  }
  //*****************************************************

  //Set ADC attenuation for full 0-3.3V range
  Serial.println("Setting ADC attenuation...");
  analogSetAttenuation(ADC_11db); 

  //Initialize PCA9685
  Serial.println("Initializing PCA9685...");
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);
  delay(10);

  //Startup
  Serial.println("Setup complete!");
  Serial.println("\n=== STARTING ANIMATION ===");
  startupAnimation();
  Serial.println("=== ANIMATION COMPLETE ===\n");
  Serial.println("Beginning normal operation...\n");
}

void loop(){
  speed();
  tach();
  temp();
  volts();
  fuelLevel();
  oilPress();
  delay(10);
}

//Function to set servo angle
void setServoAngle(uint8_t channel, int angle) {
  //Constrain angle to 0-180
  angle = constrain(angle, 0, 180);
  //Map angle to pulse length (SERVOMIN to SERVOMAX)
  int pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  //Set PWM on the channel
  pwm.setPWM(channel, 0, pulse);
}

//Helper function to set LED
//This will be changed or removed once led are replaced with ARGB leds
void setLED(uint8_t channel, bool state){
  if(state){
    //Full ON
    pwm.setPWM(channel, 4095, 0);  
  } 
  else{
    //Full OFF
    pwm.setPWM(channel, 0, 4096);  
  }
}

//Helper function to set RGB LED color
//This will be changed or removed once led are replaced with ARGB leds
void setRGBLED(uint8_t redChannel, uint8_t greenChannel, bool red, bool green){
  setLED(redChannel, red);
  setLED(greenChannel, green);
}

void speed(){
  //Read raw ADC values (0-4095 for ESP32)
  //change to VSS sensor, or simulated signal
  SpdVal = analogRead(SpdPot);
  
  /*template for speed signal calculation, change variables to match
  //Count pulses and calculate speed
  if (SpdVal == HIGH) {
    vssCount++;
  }
  //Calculate speed based on VSS pulses (example: 8000 pulses per mile, actual value depends on sensor)
  //8000 can be used if signal is simulated for demo
  float speedCalc = (vssCount / 8000.0) * 3600 / ((millis() - lastVSSTime) / 1000.0); 
  */

  //Map from 0-4095 to servo ranges (in degrees)
  //change SpdVal to speedCalc
  Speed = map(SpdVal, 0, 4095, SPDMIN, SPDMAX);
  //Write to servos via PCA9685  
  setServoAngle(SERVO_SPD, Speed);
  //Display output
  Serial.print(" Spd:");
  Serial.print(Speed);
}

void tach(){
  //Read raw ADC values (0-4095 for ESP32)
  //read value form 'Tach" pin on distributor
  //or from simulated signal for demo
  TachVal = analogRead(TachPot);
  //Map from 0-4095 to servo ranges (in degrees)
  RPM = map(TachVal, 0, 4095, TACHMIN, TACHMAX);
  //Write to servos via PCA9685
  setServoAngle(SERVO_TACH, RPM);
  //Display output
  Serial.print(" Tach:");
  Serial.println(RPM);
}

void temp(){
  //Read raw ADC values (0-4095 for ESP32)
  //change to sensor
  TempVal = analogRead(TempPot);
  //Map from 0-4095 to servo ranges (in degrees)  
  Temp = map(TempVal, 0, 4095, TEMPMIN, TEMPMAX);
  //Write to servos
  setServoAngle(SERVO_TEMP, Temp); 
  //Set warning lights 
  //warning light functions will change when leds change to ARGBs
  if(Temp < 10){
    //Yellow (red + green)
    setRGBLED(TEMP_R, TEMP_G, true, true);  
  }
  else if(Temp > 100){
    //Red only
    setRGBLED(TEMP_R, TEMP_G, true, false);  
  }
  else{
    //Off
    setRGBLED(TEMP_R, TEMP_G, false, false);  
  }
  //display output
  Serial.print(" Temp:");
  Serial.print(Temp);
}

void volts(){
  //Read raw ADC values (0-4095 for ESP32) 
  //volage does not have a dedicated sensor
  //change to algoritm to calculate value form Vin 
  BattVal = analogRead(BattPot);
  //Map from 0-4095 to servo ranges (in degrees)
  Volts = map(BattVal, 0, 4095, BATTMIN, BATTMAX);
  //Write to servos  
  setServoAngle(SERVO_BATT, Volts); 
  // Set warning lights
  //warning light functions will change when leds change to ARGBs
  if(Volts < 20 || Volts > 80){
    //Red
    setRGBLED(BATT_R, BATT_G, true, false);  
  }
  else{
    //Off
    setRGBLED(BATT_R, BATT_G, false, false);  
  }
  //Display output
  Serial.print(" Batt:");
  Serial.print(Volts);
}

void fuelLevel(){
  //Read raw ADC values   
  //change to sensor value
  FuelVal = analogRead(FuelPot);
  //Map from 0-4095 to servo ranges (in degrees)
  FuelLevel = map(FuelVal, 0, 4095, FUELMIN, FUELMAX);
  //Write to servos  
  setServoAngle(SERVO_FUEL, FuelLevel);
  // Set warning lights
  //warning light functions will change when leds change to ARGBs
  if(FuelLevel > 100){
    //Red    
    setRGBLED(FUEL_R, FUEL_G, true, false);  
  }
  else{
    //Off
    setRGBLED(FUEL_R, FUEL_G, false, false);  
  }
  //Display output
  Serial.print(" Fuel:");
  Serial.print(FuelLevel);
}

void oilPress(){
  //Read raw ADC values (0-4095 for ESP32)
  //change to sensor value
  OilVal = analogRead(OilPot);
  //Map from 0-4095 to servo ranges (in degrees)  
  OilPress = map(OilVal, 0, 4095, OILMIN, OILMAX);
  //Write to servos
  setServoAngle(SERVO_OIL, OilPress);
  // Set warning light
  //warning light functions will change when leds change to ARGBs
  if(OilPress < 40 || OilPress > 100){
    //Red
    setRGBLED(OIL_R, OIL_G, true, false);  
  }
  else{
    //Off
    setRGBLED(OIL_R, OIL_G, false, false);  
  }
  //Display output
  Serial.print("Oil:");
  Serial.print(OilPress);  
}

//Startup Animation
//Set all warning lights to red
//Sweep from 0 to max for all gauges
//Set all warnign lights to white
//Sweep from max to 0 for all gauges
void startupAnimation(){
  //make sure all guages sweep properly, may need to change min and max values
  //led functions will have to change once leds are changed to ARGBs
  Serial.println("  Setting servo ranges...");
  int servoMin[6] = {OILMIN, FUELMIN, BATTMIN, TEMPMIN, SPDMIN, TACHMIN};
  int servoMax[6] = {OILMAX, FUELMAX, BATTMAX, TEMPMAX, SPDMAX, TACHMAX};
  
  Serial.println("  Turning all lights RED...");
  //All warning lights turn red
  setRGBLED(OIL_R, OIL_G, true, false);
  setRGBLED(FUEL_R, FUEL_G, true, false);
  setRGBLED(BATT_R, BATT_G, true, false);
  setRGBLED(TEMP_R, TEMP_G, true, false);
  delay(500);
  
  Serial.println("  Sweeping gauges...");
  //Sweep each gauge from min to max
  int steps = 20;
  for (int step = 0; step <= steps; step++) {
    for (int i = 0; i < 6; i++) {
      int pos = map(step, 0, steps, servoMin[i], servoMax[i]);
      setServoAngle(i, pos);
    }
    delay(50);
  }
  //Change to white when leds are swithced to ARGB leds
  Serial.println("  Turning all lights GREEN...");
  //All warning lights turn green
  setRGBLED(OIL_R, OIL_G, false, true);
  setRGBLED(FUEL_R, FUEL_G, false, true);
  setRGBLED(BATT_R, BATT_G, false, true);
  setRGBLED(TEMP_R, TEMP_G, false, true);
  delay(500);
  
  Serial.println("  Returning gauges to zero...");
  //Return gauges to zero position (min values)
  for (int i = 0; i < 6; i++) {
    setServoAngle(i, servoMin[i]);
  }
  delay(500);
  
  Serial.println("  Turning all lights OFF...");
  //All lights off
  setRGBLED(OIL_R, OIL_G, false, false);
  setRGBLED(FUEL_R, FUEL_G, false, false);
  setRGBLED(BATT_R, BATT_G, false, false);
  setRGBLED(TEMP_R, TEMP_G, false, false);
  delay(500);
}