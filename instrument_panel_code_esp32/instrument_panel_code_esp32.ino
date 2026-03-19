/******65 Mustang Custom Modern Gauge Panel******/
/************************************************/
/**Description**/

/*NO LONGER USING PCA CHIPS, REMOVE ADAFRUIT DRIVERS, CHANGE SERVO CODE TO USE
STANDARD SERVO LIBRARIES. CHANGE CODE FOR LEDS TO REFLECT ARGB USE, USE LIBRARIES
NO LONGER USING I2C LCD, REMOVE CODE. ADD CODE TO REFLECT NEW TFT LCD, Adafruit 
1.14" 240x135 Color Newxie TFT Display - ST7789. CHANGE GROUPINGS OF PINS TO BE
NUMERICAL ORDER TO REDUCE CONFUSION IN WIRING. ORGANIZE PIN DEFINITIONS TO BE CLEANER
DELETE THIS NOTE ONCE CHANGES HAVE BEEN MADE*/

#include <Wire.h>
#include <ESP32Servo.h>

/******Variables and function delarations******/

//i2c addresses
/*change addresses based off current dection breakouts
if implemented current boards will use 0x40, 0x41, 0x44, 0x45
current plan is to use Adafruit INA260 breakout boards*/
const int EEPROM_ADDR_1 = 0x50; //Primary EEPROM to store milage and trip
const int EEPROM_ADDR_2 = 0x51; //Backuo EEPROM to store copy of mileage

//lcd

//I2C pins for ESP32-S2
#define SDA_PIN 37
#define SCL_PIN 38

//Servo settings
Servo servoOil;
Servo servoFuel;
Servo servoTemp;
Servo servoBatt;
Servo servoTach;
Servo servoSpd;
/*these may not be needed anymore, delete if not needed, uncomment if needed
#define SERVO_FREQ 50
#define SERVOMIN 150
#define SERVOMAX 600*/

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
const int trip_rest = 7;
bool LightsOnOff = 8;
bool HighsOnOff = 9;
bool LTurnIn = 10;
bool RTurnIn = 11;
bool BrakeIn = 12;
const int SwtichedSense = 13;
bool PowerOnOff = 14;
const int speaker1 = 43;
const int speaker2 = 44;

//LCD pins
const int DA = 39;
const int CS = 40;
const int DC = 41;
const int BL = 42;
const int CL = 45;

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

//Function declarations
void speed();
void tach();
void temp();
void volts();
void fuelLevel();
void oilPress();
void startupAnimation();
void checkPowerOnOff();
void readEEPROM();
void writeEEPROM();

void setup() {
  PowerOnOff = HIGH;
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

  //*********Scan I2C bus********************************
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

  //Servo definitions
  //rename with positions of each servo when updated on schematic
  servoOil.attach(17);
  servoFuel.attach(18);
  servoTemp.attach(21);
  servoBatt.attach(33);
  servoTach.attach(34);
  servoSpd.attach(35);
  
  //Startup
  Serial.println("Setup complete!");
  Serial.println("\n=== STARTING ANIMATION ===");
  startupAnimation();
  Serial.println("=== ANIMATION COMPLETE ===\n");
  Serial.println("Beginning normal operation...\n");
}

void loop(){
  checkPowerOnOff();
  speed();
  tach();
  temp();
  volts();
  fuelLevel();
  oilPress();
  delay(10);
}

void checkPowerOnOff(){
  bool checkKey = digitalRead(SwtichedSense); //check if key has been switched off
  if(checkKey == LOW){  //if yes begin shutdown
    writeEEPROM();  //write milage to EEPROM
    digitalWrite(PowerOnOff, LOW);  //shut down
  }
}

void readEEPROM(){
  //read values stored in EEPROM
  //compare values in both chips
  //if one is higher than the other use the higher value and save to both
  //store read value in VARIABLE to use durring operation
}

void writeEEPROM(){/*
  if(/*current value is greater than the stored value){
    //write current value of milage to both EEPROMs via wear leveling algorithm
    //write current trip value to main EEPROM via wear leveling, no need to backup trip reading
    //set servos to "0" position
  }
  //else do nothing*/
}

/******FUNCTIONS NEED TO MODIFIED OR REMOVED TO FIT CURRENT PLAN******/
//Function to set servo angle
//may not be needed if so delete, if needed modify to work
void setServoAngle(uint8_t channel, int angle) {/*
  //Constrain angle to 0-180
  angle = constrain(angle, 0, 180);
  //Map angle to pulse length (SERVOMIN to SERVOMAX)
  int pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  //Set PWM on the channel
  pwm.setPWM(channel, 0, pulse);*/
}


void speed(){/*
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
  

  //Map from 0-4095 to servo ranges (in degrees)
  //change SpdVal to speedCalc
  Speed = map(SpdVal, 0, 4095, SPDMIN, SPDMAX);
  //Write to servos via PCA9685  
  setServoAngle(SERVO_SPD, Speed);
  //Display output
  Serial.print(" Spd:");
  Serial.print(Speed);*/
}

void tach(){/*
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
  Serial.println(RPM);*/
}

void temp(){/*
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
  Serial.print(Temp);*/
}

void volts(){/*
  //Read raw ADC values (0-4095 for ESP32) 
  //volage does not have a dedicated sensor
  //voltage divider R1=39k, R2=10K
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
  Serial.print(Volts);*/
}

void fuelLevel(){/*
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
  Serial.print(FuelLevel);*/
}

void oilPress(){/*
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
  Serial.print(OilPress);*/  
}

//Startup Animation
//Set all warning lights to red
//Sweep from 0 to max for all gauges
//Set all warnign lights to white
//Sweep from max to 0 for all gauges
void startupAnimation(){/*
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
  delay(500);*/
}
