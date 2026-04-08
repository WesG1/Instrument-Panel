/**************Dash Test Bench**************/
/*******************************************/

/*Program to run alongside the main instrument cluster
that simulates the signals from the oil, fuel, temp, tach,
and VSS sensors. The signals will be adjusted with 
potentiometers and the outputs will be displayed
on LCDs as well as communicating with the ESP32 on the actual 
instrument cluster. The tach and VSS signals will be output 
as square wave pulses whereas the oil, temp, and fuel will
just be an analog signal directly from the potemtiometers.
This will also control whether or not the vehicle lights
are turned on, turn signals, as well as brakes, and fog
light for fault detection (if implemented).This code is 
initially being written for an Arduino board, but can be 
changed if a different board is required.*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD Configuration
//Multiple LCDs may be used, change address as needed
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Variables
//Oil, temp, and fuel pots are all 100ohms as part of a voltage divider
//Each has the same range, 78-10ohms, inversly proportional to its value
//Oil 78=5psi, 50=15psi, 26=45psi, 16=70psi, 10=85psi
//Temp 78=130*F, 50=160*F, 26=200*F, 16=230*F, 10=250*F
//Fuel 78=E, 50=3/4, 26=1/2, 16=3/4, 10=F
//Tach and speed values do not matter
//Specific pin assignments may change based on board used
//If fault detection is not implemented, remove Brakes, Fog, and LowHiHBm

//Inputs
const int OilPot = A0;
const int TempPot = A1;
const int FuelPot = A2;
const int TachPot = A3;
const int SpdPot = A6;
const int LowBeams = 13;
const int LeftTurn = 12;
const int RightTurn = 8;
const int Hazard = 7;
const int Brakes = 4;
const int HighBeams = 3;
//Outputs
const int LftTrnOut = 11;
const int RgtTrnOut = 10;
const int HazdOut = 9;
const int RPM = 6;
const int MPH = 5

//Pin Values
int OilVal = 0;
int TempVal = 0;
int FuelVal = 0;
int TachVal = 0;
int SpdVal = 0;
bool LowBeamVal = false;
bool LftTrnVal = false;
bool RgtTrnVal = false;
bool HazdVal = false;
bool BrkVal = false;
bool HighBeamVal = false;

//Function declarations
void setSquareWave(int pin);
void setTachPulse();
void setVSSPulse();
void displayLCD();

void setup() {
  //Begin Serial
  Serial.begin(9600);
  delay(100);
  //Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  //Set pinModes
  pinMode(OilPot,INPUT);
  pinMode(TempPot,INPUT);
  pinMode(FuelPot,INPUT);
  pinMode(TachPot,INPUT);
  pinMode(SpdPot,INPUT);
  pinMode(LowBeams,INPUT);
  pinMode(LeftTurn,INPUT);
  pinMode(RightTurn,INPUT);
  pinMode(Hazard,INPUT);
  pinMode(Brakes,INPUT);
  pinMode(HighBeams,INPUT);
  pinMode(LftTrnOut,OUTPUT);
  pinMode(RgtTrnOut,OUTPUT);
  pinMode(HazdOut,OUTPUT);
  pinMode(RPM,OUTPUT);
  pinMode(MPH,OUTPUT);
}

void loop() {
  //Read all inputs
  OilVal = analogRead(OilPot);
  TempVal = analogRead(TempPot);
  FuelVal = analogRead(FuelPot);
  TachVal = analogRead(TachPot);
  SpdVal = analogRead(SpdPot);
  LowBeamVal = digitalRead(LowBeams);
  LftTrnVal = digitalRead(LeftTurn);
  RgtTrnVal = digitalRead(RightTurn);
  HazdVal = digitalRead(Hazard);
  BrkVal = digitalRead(Brakes);
  HighBeamVal = digitalRead(HighBeams);
  //Process speed
  setVSSPulse();
  //Process tach
  setTachPulse();
  //Process turn and hazard lights
  if(LftTrnVal == HIGH){
    setSquareWave(LftTrnOut);
  }
  else{
    digitalWrite(LftTrnOut,LOW);
  }
  if(RgtTrnVal == HIGH){
    setSquareWave(RgtTrnOut);
  }
  else{
    digitalWrite(RgtTrnOut,LOW);
  }
  //??????
  //was there more?
}

//function may need to be editted once tested
void setVSSPulse(){
  // VSS Configuration
  const long PULSES_PER_MILE = 55400;  // VSS pulses per mile
  const int MAX_SPEED_MPH = 120;        // Maximum speed in MPH
  unsigned long lastPulseTime = 0;
  unsigned long pulsePeriodMicros = 0;
  bool pulseState = LOW;
  // Read potentiometer value (0-1023)
  int SpdVal = analogRead(SpdPot);
  // Map potentiometer to speed (0 to MAX_SPEED_MPH)
  int speedMPH = map(SpdVal, 0, 1023, 0, MAX_SPEED_MPH);
  // Calculate pulse frequency based on speed
  if(speedMPH > 0){
    // Convert speed to pulses per second
    // Speed (mph) * pulses_per_mile / 3600 seconds_per_hour = pulses per second
    float pulsesPerSecond = (float)speedMPH * PULSES_PER_MILE / 3600.0;
    // Period between pulses in microseconds
    // We toggle twice per pulse (HIGH then LOW), so divide by 2
    pulsePeriodMicros = (unsigned long)(1000000.0 / (pulsesPerSecond * 2.0));
  } 
  else{
    pulsePeriodMicros = 0;  // No pulses when speed is 0
  }
  // Generate VSS square wave
  if(pulsePeriodMicros > 0) {
    unsigned long currentMicros = micros();
    if(currentMicros - lastPulseTime >= pulsePeriodMicros){
      pulseState = !pulseState;
      digitalWrite(MPH, pulseState);
      lastPulseTime = currentMicros;
    }
  } 
  else{
    digitalWrite(MPH, LOW);  // No signal at 0 mph
  }
}

void setTachPulse(){
  unsigned long lastPulseTime = 0;
  unsigned long pulseInterval = 0;
  //0 on pot = very long interval (essentially no pulses)
  //1023 on pot = 7ms interval (fastest pulse rate)
  //100000ms = no pulses, 7ms = max speed
  TachVal = map(TachPot, 0, 1023, 100000, 7);  
  //Generate pulse based on interval
  unsigned long currentTime = millis();
  if (currentTime - lastPulseTime >= pulseInterval) {
    // Toggle the pulse pin
    digitalWrite(RPM, HIGH);
    delayMicroseconds(100);  // Pulse width of 100 microseconds
    digitalWrite(RPM, LOW);
    lastPulseTime = currentTime;
  }
}

void setSquareWave(int pin){
  digitalWrite(pin,HIGH);
  delay(500);
  digitalWrite(pin,LOW);
  delay(500);
}

//LCD will cycle through different values
//If multiple LCDs are used then each should have
//a dedicated function to display its data
//Print to serial monitor as well as LCD
void displayLCD(){
  lcd.setCursor(0, 0);
  lcd.print("Oil ");
  lcd.print(OilVal);
  lcd.setCursor(8, 0);
  lcd.print("Temp ");
  lcd.print(TempVal);
  lcd.setCursor(0, 1);
  lcd.print("Fuel ");
  lcd.print(FuelVal);
  delay(500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Speed Pulses ");
  lcd.print(MPH);
  lcd.setCursor(0, 1);
  lcd.print("Tach Pulses ");
  lcd.print(RPM);
  delay(500);
  lcd.clear();
  //add logic for displaying if lights are on or off
}