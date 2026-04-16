/******65 Mustang Custom Modern Gauge Panel******/
/************************************************/
/**Description**/

/* ADD CODE TO REFLECT NEW TFT LCD, Adafruit 
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
const int OilSense = 6;
const int FuelSense = 5;
const int BattVoltage = 4;
const int TempSense = 3;
const int SpdPot = 2;
const int TachPot = 1;

//Additional I/O Pins
const int trip_rest = 7;      //Trip meter reset button
const int LightsOnOff = 8;    //Input from low bean switch
const int HighsOnOff = 9;     //Input from high beam swtich
const int LTurnIn = 10;       //Input from left turn signal
const int RTurnIn = 11;       //Input from right turn signal
const int BrakeIn = 12;       //Input forn brake switch
const int SwtichedSense = 13; //Sensor for key position
const int PowerOnOff = 14;    //Main power shut off
const int echoPin = 15;       //Echo pin for ultrasonic sensor
const int trigPin = 16;       //Trigger pin for ultrasonic sensor
const int LEDdata = 36;       //Data pin for ARGB LEDs
const int speaker = 44;       //Speaker for turn signal and hazard beeps
/*speaker may interfere with serial output, leave disconnected or comment 
out speaker code if not currently using it*/

//LCD pins
//add additional pins for extra LCDs
const int DA = 39;
const int CS = 40;
const int DC = 41;
const int BL = 42;
const int CL = 46;

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

/*mileage variables, actual value stored in EEPROM and value for when program is running to be incramented
currently ints but change to floats if necessary*/
//int milesCount //variable to store odometer and trip values. gets incramented by 1 for each mile driven, not the actual mileage value
//int storedMilage //variable that stores mileage in EEPROM

const int R1 = 39000; //Voltage divider R1 39K
const int R2 = 10000; //Voltage diider R2 10K
bool powerState = false;

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
void setLEDs(/*add variables here if needed*/);
void readUltraSonic();
void generateTone();
void scanFaultDetection();
void displayLCD();

void setup() {
  pinMode(trip_rest, INPUT_PULLUP);
  pinMode(LightsOnOff, INPUT_PULLUP);
  pinMode(HighsOnOff, INPUT_PULLUP);
  pinMode(LTurnIn, INPUT_PULLUP);
  pinMode(RTurnIn, INPUT_PULLUP);
  pinMode(BrakeIn, INPUT_PULLUP);
  pinMode(SwtichedSense, INPUT_PULLUP);
  pinMode(PowerOnOff, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LEDdata, OUTPUT);
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

void displayLCD(){
  /*write data to 2-3 lcd displays
  display odometer on 1 lcd
  display tripmeter on 1 lcd
  if not using leds, display high/low beams and brake warning lights*/
}

void scanFaultDetection(){
  /*read light input pins to see if any lights are currently on
  if yes scan the i2c current sensor for that light
  if current is below threshold for a light, set the warning light red
  if the current is normal, set the light a standard color, green for turn signal, blue for high/low beams, none for brakes
  ~9 amps for low beam, ~ 10 amps for high beam
  ~1 amp for tail lights, ~4 amps for brake lights, ~2 amps for turn light*/
}

void generateTone(){
  //code not needed for turn or hazard signal, speaker will click on its own
  //rapid beeping for distance warning from ultrasonic sensor
  //slow beeping for gauge warning, if red light is triggered
}

void readUltraSonic(){
  //read data from sensor
  //convert distance to "real life values" for demo
}

void checkPowerOnOff(){
  bool checkKey = digitalRead(SwtichedSense); //check if key has been switched off
  if(checkKey == LOW){  //if yes begin shutdown
    //sweep all gauges to 0 position
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
  if(current value is greater than the stored value){
    //write current value of milage to both EEPROMs via wear leveling algorithm
    //write current trip value to main EEPROM via wear leveling, no need to backup trip reading
  }
  //else do nothing*/
}

void setLEDs(/*add variables here if needed*/){
  /*This may not work as a standalone function
  if not, split the code for each LED to its own function
  using neopixel leds for warning lights
  using argb strip for backlights
  use adafruit libray

  sets backlight on if headlights are on
  set backilight off if headligths are off
  set gauge lights to white if headlights are on and no warning conditions
  set gauge lights to off if headlights are off and no warning conditions

  set batt light to red if voltage is too low or too high
  set oil light to red if pressure is too low or too high
  set temp light to yellow if too low, red if too high
  set fuel light to yellow if too low
  */
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
  float speedCalc = (vssCount / 8000.0) * 3600 / ((millis() - lastVSSTime) / 1000.0);*/  

  //Map from 0-4095 to servo ranges (in degrees)
  //change SpdVal to speedCalc
  Speed = map(SpdVal, 0, 4095, SPDMIN, SPDMAX);
  servoSpd.write(Speed);
  delay(10);
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
  servoTach.write(RPM);
  //Display output
  Serial.print(" Tach:");
  Serial.println(RPM);
}

void temp(){
  //Read raw value from temp sensor
  TempVal = analogRead(TempSense);
  //prevents a divide-by-zero error
  if (TempVal == 0){
    servoTemp.write(TEMPMIN); 
    return; 
  }
  //Convert ADC reading to resistance (560Ω pull-up)
  float resistance = 560.0 * ((4095.0 / TempVal) - 1.0);
  //Map resistance to pressure (PSI) using lookup table
  //78Ω@ 130*, 50Ω@ 160*, 26Ω@ 200*, 16Ω@ 230*, 10Ω@ 250*
  if(resistance >= 78) Temp = 5;
  else if (resistance >= 50) Temp = map(resistance, 50, 78, 15, 5);
  else if (resistance >= 26) Temp = map(resistance, 26, 50, 45, 15);
  else if (resistance >= 16) Temp = map(resistance, 16, 26, 70, 45);
  else if (resistance >= 10) Temp= map(resistance, 10, 16, 85, 70);
  else                       Temp = 85;
  // Map PSI to servo angle and write
  Temp = map(Temp, 0, 85, TEMPMIN, TEMPMAX);
  servoTemp.write(Temp);  
}

void fuelLevel(){
  //Read raw value from level sensor
  FuelVal = analogRead(FuelSense);
  //prevents a divide-by-zero error
  if (FuelVal == 0){
    servoFuel.write(FUELMIN); 
    return; 
  }
  //Convert ADC reading to resistance (560Ω pull-up)
  float resistance = 560.0 * ((4095.0 / FuelVal) - 1.0);
  //Map resistance to pressure (PSI) using lookup table
  //78Ω@ Empty, 50Ω@ 1/4, 26Ω@ 1/2, 16Ω@ 3/4, 10Ω@ Full
  if(resistance >= 78) FuelLevel = 5;
  else if (resistance >= 50) FuelLevel = map(resistance, 50, 78, 15, 5);
  else if (resistance >= 26) FuelLevel = map(resistance, 26, 50, 45, 15);
  else if (resistance >= 16) FuelLevel = map(resistance, 16, 26, 70, 45);
  else if (resistance >= 10) FuelLevel = map(resistance, 10, 16, 85, 70);
  else                       FuelLevel = 85;
  // Map PSI to servo angle and write
  FuelLevel = map(FuelLevel, 0, 85, FUELMIN, FUELMAX);
  servoFuel.write(FuelLevel);
}

void oilPress(){
  //Read raw value from pressure sensor
  OilVal = analogRead(OilSense);
  //prevents a divide-by-zero error
  if (OilVal == 0){
    servoOil.write(OILMIN); 
    return; 
  }
  //Convert ADC reading to resistance (560Ω pull-up)
  float resistance = 560.0 * ((4095.0 / OilVal) - 1.0);
  //Map resistance to pressure (PSI) using lookup table
  //78Ω@5psi, 50Ω@15psi, 26Ω@45psi, 16Ω@70psi, 10Ω@85psi
  if(resistance >= 78) OilPress = 5;
  else if (resistance >= 50) OilPress = map(resistance, 50, 78, 15, 5);
  else if (resistance >= 26) OilPress = map(resistance, 26, 50, 45, 15);
  else if (resistance >= 16) OilPress = map(resistance, 16, 26, 70, 45);
  else if (resistance >= 10) OilPress = map(resistance, 10, 16, 85, 70);
  else                       OilPress = 85;
  // Map PSI to servo angle and write
  OilPress = map(OilPress, 0, 85, OILMIN, OILMAX);
  servoOil.write(OilPress); 
}

void startupAnimation(){
  //Startup Animation
  //Set all warning lights on
  //Sweep from 0 to max for all gauges
  //Set all warnign lights to white
  //Sweep from max to 0 for all gauges
}