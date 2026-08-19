/***********************************************************************************************
************************************************************************************************
******************* CODE SHOULD BE REWRITTEN TO USE RTOS NATIVE ON ESP32 ***********************
****************** ORIGINAL CODE COMMENTED OUT TO BE USED AS REFERENCE ONLY ********************
*********************** RTOS CODE BEGINS AFTER COMMENTED OUT SECTION ***************************
************************************************************************************************
************************************************************************************************

******65 Mustang Custom Modern Gauge Panel******
************************************************
**Description**

* ADD CODE TO REFLECT NEW TFT LCD, Adafruit 
1.14" 240x135 Color Newxie TFT Display - ST7789. CHANGE GROUPINGS OF PINS TO BE
NUMERICAL ORDER TO REDUCE CONFUSION IN WIRING. ORGANIZE PIN DEFINITIONS TO BE CLEANER
DELETE THIS NOTE ONCE CHANGES HAVE BEEN MADE*

#include <Wire.h>
#include <ESP32Servo.h>

******Variables and function delarations******

//i2c addresses
*change addresses based off current dection breakouts
if implemented current boards will use 0x40, 0x41, 0x44, 0x45
current plan is to use Adafruit INA260 breakout boards*
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
*speaker may interfere with serial output, leave disconnected or comment 
out speaker code if not currently using it*

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

*mileage variables, actual value stored in EEPROM and value for when program is running to be incramented
currently ints but change to floats if necessary*
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
void setLEDs(*add variables here if needed*);
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
  *write data to 2-3 lcd displays
  display odometer on 1 lcd
  display tripmeter on 1 lcd
  if not using leds, display high/low beams and brake warning lights*
}

void scanFaultDetection(){
  *read light input pins to see if any lights are currently on
  if yes scan the i2c current sensor for that light
  if current is below threshold for a light, set the warning light red
  if the current is normal, set the light a standard color, green for turn signal, blue for high/low beams, none for brakes
  ~9 amps for low beam, ~ 10 amps for high beam
  ~1 amp for tail lights, ~4 amps for brake lights, ~2 amps for turn light*
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

void writeEEPROM(){*
  if(current value is greater than the stored value){
    //write current value of milage to both EEPROMs via wear leveling algorithm
    //write current trip value to main EEPROM via wear leveling, no need to backup trip reading
  }
  //else do nothing*
}

void setLEDs(*add variables here if needed*){
  *This may not work as a standalone function
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
  *
}

void speed(){*
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
  float speedCalc = (vssCount / 8000.0) * 3600 / ((millis() - lastVSSTime) / 1000.0);*  

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

************************************************************************************************
************************************************************************************************
********************************* END ORIGINAL CODE ********************************************
******************************** RTOS CODE BEGINS BELOW ****************************************
***************** DELETE ORIGINAL CODE SECTIONS AS RTOS SECTIONS COMPLETED *********************
************************************************************************************************
***********************************************************************************************/

// ─────────────────────────────────────────────
//  INCLUDES
// ─────────────────────────────────────────────

// ESP32Servo: Provides Servo control compatible with ESP32's PWM timer system.
// The standard Arduino Servo.h doesn't work on ESP32, so this replaces it.
#include <ESP32Servo.h>

// Adafruit_NeoPixel: Drives WS2812B addressable RGB LEDs over a single data wire.
// Handles the precise timing protocol required by NeoPixels.
#include <Adafruit_NeoPixel.h>

// Wire: Arduino's built-in I2C library. Used here to communicate with the
// 24LC02B EEPROM chips over the I2C bus (SDA + SCL lines).
#include <Wire.h>

// Adafruit_GFX: Base graphics library providing text, shapes, and drawing primitives.
// Required by Adafruit_ST7789 — it acts as the abstract drawing layer.
#include <Adafruit_GFX.h>

// Adafruit_ST7789: Display driver for ST7789-based TFT LCD screens.
// Built on top of Adafruit_GFX and handles the SPI communication to the display.
#include <Adafruit_ST7789.h>

// SPI: Arduino's built-in SPI (Serial Peripheral Interface) library.
// Required to configure and use the hardware SPI bus that drives both TFT displays.
#include <SPI.h>

// ─────────────────────────────────────────────
//  SERVO PIN & ANGLE DEFINITIONS
//  NOTE: GPIO 34 conflict resolved — Servo originally
//        listed on GPIO 34 is moved to GPIO 33 (GPIO 34
//        is used by TFT CS1). Original servo 33 stays on 33.
//        Adjust if your hardware differs.
// ─────────────────────────────────────────────

// Array of 6 GPIO pin numbers, one per servo motor.
// Using an array lets us loop over all servos rather than writing
// individual lines for each one, keeping the code concise and scalable.
const int SERVO_PINS[6] = {10, 11, 12, 13, 14, 15};  // GPIO 34 freed for TFT CS

// Minimum angle (in degrees) for each servo — the "closed" or "home" position.
// Each servo may have a different mechanical range, so per-servo min values
// allow precise calibration rather than assuming all servos share the same limits.
// Order matches SERVO_PINS[] above: S1, S2, S3, S4, S5, S6
const int SERVO_MIN[6] = {118, 116, 115,  97,  95, 100};

// Maximum angle (in degrees) for each servo — the "open" or "extended" position.
// Note these values are intentionally lower than SERVO_MIN because some servos
// are mounted in reverse, so their physical "open" direction is a smaller degree value.
const int SERVO_MAX[6] = {29,  20,   0,  15,  20,  17};

// ─────────────────────────────────────────────
//  GAUGE POTENTIOMETER DEFINITIONS
//  Servos 1–4 (indices 0–3) are now driven live by a potentiometer
//  instead of the preprogrammed sweep. Wire each pot as a simple voltage
//  divider: outer legs to 3.3V and GND, wiper to the ADC pin below.
//  The pot's total resistance value doesn't matter (10K, 100 ohm, etc.) —
//  only the wiper's position along that range, read as a voltage ratio.
//  These are ESP32-S2 ADC1 pins (GPIO1–10), which stay usable even if
//  WiFi is added later (ADC2 pins conflict with WiFi and are avoided here).
// ─────────────────────────────────────────────
const int POT_PINS[4] = {3, 4, 5, 6};  // Pot wipers for gauges 1-4 (servo indices 0-3)

// ─────────────────────────────────────────────
//  NEOPIXEL DEFINITIONS
// ─────────────────────────────────────────────

// GPIO pin connected to the NeoPixel data-in line.
// This single wire carries the timed serial signal for all 10 LEDs in the chain.
#define NEO_PIN        41

// Total number of NeoPixel LEDs in the strip.
// The library needs this at construction time to allocate the right buffer size.
#define NEO_COUNT      10

// Construct the NeoPixel strip object.
// NEO_COUNT sets how many LEDs, NEO_PIN is the data pin,
// NEO_GRB means data is sent Green-Red-Blue order (WS2812B spec),
// NEO_KHZ800 sets the 800 kHz signal rate required by WS2812B chips.
Adafruit_NeoPixel strip(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

// Array of 10 pre-defined 24-bit RGB colors, one assigned to each LED.
// Stored as hex values in 0xRRGGBB format. Having a unique color per LED
// makes it visually obvious which specific LED is active during the chase pattern.
const uint32_t LED_COLORS[10] = {
  0x00FF00,  // Red
  0xFF6600,  // Orange
  0xFFFF00,  // Yellow
  0xFF0000,  // Green
  0x0000FF,  // Blue
  0x8B00FF,  // Purple
  0xFFFFFF,  // White
  0x00FFFF,  // Cyan
  0xFF00FF,  // Magenta
  0x4B2000   // Brown
};

// ─────────────────────────────────────────────
//  BUTTON / INTERRUPT DEFINITIONS
//  GPIO 18 drives LED 5 (strip index 4).
//  GPIO 21 drives LED 6 (strip index 5).
//  Both pins use the internal pull-up resistor so no external
//  resistor is needed; pressing the button pulls the line LOW.
// ─────────────────────────────────────────────

// GPIO pin for the button that controls LED 5.
// Wired as active-LOW: pin reads HIGH at rest, LOW when pressed.
#define BTN_LED5_PIN   18

// GPIO pin for the button that controls LED 6.
// Same wiring convention as BTN_LED5_PIN.
#define BTN_LED6_PIN   21

// GPIO pin for the button that controls LED 7.
// Wired as active-LOW: pin reads HIGH at rest, LOW when pressed.
#define BTN_LED7_PIN   46

// GPIO pin for the button that controls LED 8.
// Same wiring convention as BTN_LED5_PIN.
#define BTN_LED8_PIN   38

// Color applied to LEDs 5 and 6 when their respective button is held.
// 0xFF0000 is the "Green" entry already defined in LED_COLORS[] (index 3).
// Using the same constant keeps the color consistent across the codebase.
#define TURN_LED_COLOR  0xFF0000   // Green (matches LED_COLORS[3])

// Color applied to LEDs 7 and 8 when their respective button is held.
// 0x0000FF is the "BLUE" entry already defined in LED_COLORS[] (index 4).
// Using the same constant keeps the color consistent across the codebase.
#define HEADLIGHT_LED_COLOR  0x0000FF

// ─────────────────────────────────────────────
//  I2C / EEPROM DEFINITIONS
// ─────────────────────────────────────────────

// GPIO pin used for I2C SDA (Serial Data). This is the bidirectional data line
// over which bytes are transferred between the ESP32 and the EEPROM chips.
#define I2C_SDA        8

// GPIO pin used for I2C SCL (Serial Clock). This line is driven by the ESP32
// (as I2C master) to pace all data transfers on the bus.
#define I2C_SCL        9

// I2C address of the first 24LC02B EEPROM. The 24LC02B has 3 address pins (A0-A2)
// that set the lower bits of the address. 0x50 corresponds to all address pins LOW.
#define EEPROM_ADDR_0  0x50

// I2C address of the second 24LC02B EEPROM. 0x51 means A0 is HIGH, A1 and A2 LOW.
// This allows both chips to share the same I2C bus without address collision.
#define EEPROM_ADDR_1  0x51

// ─────────────────────────────────────────────
//  TFT DISPLAY DEFINITIONS
//  Both displays share: DA(MOSI)=39, DC=41, BL=42, CL(SCK)=46
//  CS: Display 0 = GPIO 34, Display 1 = GPIO 26
// ─────────────────────────────────────────────

// MOSI (Master Out Slave In) pin — carries pixel data FROM the ESP32 TO the displays.
// Both displays share this line since only one CS is active at a time.
#define TFT_DA         35   // MOSI

// SCK (SPI Clock) pin — synchronizes data transfer between ESP32 and the TFT displays.
// Shared between both displays, again safe because CS controls which is listening.
#define TFT_CL         36   // SCK

// Data/Command pin — tells the display whether the incoming byte is a command
// (e.g., set cursor) or pixel data. Both displays share this line.
#define TFT_DC         37

// Backlight pin — drives the LED backlight of both TFT panels.
// Setting this HIGH turns the backlight on; the displays would be invisible without it.
#define TFT_BL         42

// Chip Select for Display 0 — when pulled LOW, this selects Display 0 for SPI communication.
// When HIGH, Display 0 ignores SPI bus traffic, allowing Display 1 to be addressed.
#define TFT_CS0        34   // Display 0  (EEPROM 0x50)

// Chip Select for Display 1 — same role as TFT_CS0, but for the second display.
// The library manages toggling CS0 and CS1 automatically when you call tft0 vs tft1.
#define TFT_CS1        45   // Display 1  (EEPROM 0x51)

// Reset pin — set to -1 because the display's RESET pin is tied to 3.3V (always-high)
// or shared with the ESP32's EN line, meaning no software-controlled reset is needed.
#define TFT_RST        -1   // Tie to 3.3 V or share ESP32 EN

// Create a custom SPI bus instance using the ESP32's HSPI peripheral.
// The ESP32-S2 has multiple SPI hardware blocks; HSPI is used here so the
// TFT displays get their own dedicated bus, separate from any other SPI devices.
SPIClass tftSPI(HSPI);

// Instantiate the first ST7789 display object, bound to the shared SPI bus.
// Passing TFT_CS0 and TFT_DC tells the driver which pins control this specific display.
// TFT_RST = -1 tells the driver not to perform a hardware reset.
Adafruit_ST7789 tft0 = Adafruit_ST7789(&tftSPI, TFT_CS0, TFT_DC, TFT_RST);

// Instantiate the second ST7789 display object on the same SPI bus.
// Only the CS pin differs (TFT_CS1), which is how the library targets each display separately.
Adafruit_ST7789 tft1 = Adafruit_ST7789(&tftSPI, TFT_CS1, TFT_DC, TFT_RST);

// ─────────────────────────────────────────────
//  SERVO OBJECTS
// ─────────────────────────────────────────────

// Array of 6 Servo objects, one for each physical servo motor.
// Grouping them in an array allows all servos to be controlled with loops
// instead of separate variables and duplicate code blocks.
Servo servos[6];

// ─────────────────────────────────────────────
//  TASK HANDLES
// ─────────────────────────────────────────────

// Handle for the servo FreeRTOS task. TaskHandle_t is an opaque pointer
// that lets you reference, suspend, resume, or delete the task later if needed.
TaskHandle_t hServoTask;

// Handle for the NeoPixel FreeRTOS task. Stored globally so it can be
// managed from setup() or loop() if future control logic is needed.
TaskHandle_t hNeoTask;

// Handle for the EEPROM + TFT display FreeRTOS task.
// This task is given extra stack (8192 bytes) due to the heavier display operations.
TaskHandle_t hEepromTask;

// ─────────────────────────────────────────────
//  BUTTON INTERRUPT STATE
//  Written inside ISRs (interrupt context), read by neoTask.
//  volatile prevents the compiler from caching these in a register —
//  the ISR and the task run in different execution contexts, so every
//  read must go back to actual memory to get the latest value.
// ─────────────────────────────────────────────

// True while the GPIO 18 button is physically held down.
// Updated unconditionally on every neoTask tick by polling the pin directly,
// so the LED state always matches the live hardware level regardless of any
// interrupt timing edge cases.
volatile bool leftTurnState = false;

// True while the GPIO 21 button is physically held down.
// Same polling strategy as btn5State.
volatile bool rightTurnState = false;

// True while the GPIO 46 button is physically held down.
// Same polling strategy as lowBeamState.
volatile bool lowBeamState = false;

// True while the GPIO 38 button is physically held down.
// Same polling strategy as highBeamState.
volatile bool highBeamState = false;

// ─────────────────────────────────────────────
//  SHARED GAUGE STATE
//  Written by servoTask, read by neoTask.
//  volatile tells the compiler to re-read from memory on every access
//  rather than caching the value in a CPU register — essential when two
//  FreeRTOS tasks running on separate scheduler time-slices share a variable.
// ─────────────────────────────────────────────

// Current angle (in degrees) of gauges 1–4 (servo indices 0–3).
// Pre-initialized to each servo's SERVO_MIN value so the LED state is valid
// before the servo task executes its first sweep step.
volatile int gaugeAngle[4] = { SERVO_MIN[0], SERVO_MIN[1], SERVO_MIN[2], SERVO_MIN[3] };

// ════════════════════════════════════════════════════════════
//  HARDWARE INTERRUPT SERVICE ROUTINES
//  Attached to the two button GPIOs via attachInterrupt() in setup().
//  IRAM_ATTR places each function in IRAM (Internal RAM) so it can
//  execute even when the flash cache is temporarily unavailable —
//  a hard requirement for ISRs on the ESP32/ESP32-S2.
//  The ISRs are intentionally empty stubs. Button state is read by
//  neoTask via direct pin polling every 50 ms — this is reliable and
//  more than fast enough for human button interaction. The interrupts
//  are still registered and fire correctly; they simply don't need to
//  carry state because the polling covers it without any timing
//  ambiguity about when the GPIO level is sampled.
// ════════════════════════════════════════════════════════════

// ISR for the GPIO 18 button (LED 5 controller). State is read by neoTask.
void IRAM_ATTR isr_btn5() { /* state read by neoTask polling */ }

// ISR for the GPIO 21 button (LED 6 controller). State is read by neoTask.
void IRAM_ATTR isr_btn6() { /* state read by neoTask polling */ }

// ════════════════════════════════════════════════════════════
//  EEPROM HELPERS
// ════════════════════════════════════════════════════════════

// Writes a single byte of data to a specific memory address on a 24LC02B EEPROM.
// devAddr: the I2C address of the target EEPROM chip (0x50 or 0x51)
// memAddr: the byte location within the EEPROM to write (0x00–0xFF, 256 bytes total)
// data:    the byte value to store at that location
void eepromWrite(uint8_t devAddr, uint8_t memAddr, uint8_t data) {
  // Begin an I2C transmission to the specified device address.
  // This pulls SDA low (START condition) and sends the 7-bit address + write bit.
  Wire.beginTransmission(devAddr);

  // Send the memory address byte — tells the EEPROM which internal register to write to.
  // The 24LC02B expects the memory address as the first byte after the device address.
  Wire.write(memAddr);

  // Send the actual data byte to be stored at memAddr inside the EEPROM.
  Wire.write(data);

  // End the transmission by sending a STOP condition on the I2C bus.
  // This triggers the EEPROM to commit the byte to non-volatile memory.
  Wire.endTransmission();

  // Wait 5 ms to respect the 24LC02B's internal write cycle time.
  // The datasheet specifies a maximum write cycle of 5 ms; writing again before
  // this completes could corrupt the data or cause a NACK from the device.
  delay(5);  // 24LC02B write cycle time ≤ 5 ms
}

// Reads a single byte from a specific memory address on a 24LC02B EEPROM.
// Returns the byte stored at memAddr, or 0xFF if no byte was received (error sentinel).
// devAddr: the I2C address of the target EEPROM chip
// memAddr: the byte location within the EEPROM to read from
uint8_t eepromRead(uint8_t devAddr, uint8_t memAddr) {
  // Begin a write transmission to tell the EEPROM which address we want to read.
  // On I2C EEPROMs, a read is a two-phase operation: first write the address pointer,
  // then do a read request. This first phase sets the internal address pointer.
  Wire.beginTransmission(devAddr);

  // Send the memory address we want to read from.
  // This sets the EEPROM's internal "current address" register.
  Wire.write(memAddr);

  // End transmission with false = send a REPEATED START instead of a full STOP.
  // This keeps the I2C bus "owned" by the master (ESP32) so the next requestFrom
  // doesn't lose the address pointer we just set. A full STOP would also work on
  // most EEPROMs, but repeated start is cleaner and more spec-compliant.
  Wire.endTransmission(false);

  // Request exactly 1 byte from the EEPROM device.
  // The cast to uint8_t is needed because Wire.requestFrom is overloaded and
  // passing plain integer literals can cause ambiguity warnings on ESP32.
  Wire.requestFrom((uint8_t)devAddr, (uint8_t)1);

  // If a byte is available in the receive buffer, read and return it.
  // If nothing arrived (device not present or NAK), return 0xFF as an error indicator.
  // 0xFF is a natural "blank" value for EEPROMs (all bits high = erased state),
  // so receiving it could mean either "blank cell" or "no response" — use Serial
  // debug output to distinguish in practice.
  return Wire.available() ? Wire.read() : 0xFF;
}

// ════════════════════════════════════════════════════════════
//  TFT HELPERS
// ════════════════════════════════════════════════════════════

// Renders EEPROM read-back information onto one TFT display.
// Designed to be called with either tft0 or tft1, making it reusable for both screens.
// tft:     reference to the display object to draw on (tft0 or tft1)
// devAddr: the EEPROM's I2C address, shown in the header so you know which chip
// memAddr: the memory address that was read, displayed for traceability
// value:   the byte value that was read back from the EEPROM, shown prominently
void tftShowEEPROM(Adafruit_ST7789 &tft, uint8_t devAddr,
                   uint8_t memAddr, uint8_t value) {
  // Clear the entire screen by filling it with solid black.
  // This erases any previous content before drawing the new frame of data.
  tft.fillScreen(ST77XX_BLACK);

  // ── Header row: shows which EEPROM chip this display is monitoring ──

  // Set the text color to cyan for the header — visually distinct from other rows.
  tft.setTextColor(ST77XX_CYAN);

  // Set font scale to 2x (each character is 12×16 pixels at this scale).
  // Larger text is easier to read at a glance during hardware testing.
  tft.setTextSize(2);

  // Position the text cursor 10 pixels from the left, 10 pixels from the top.
  // This gives a small margin so text doesn't start at the very edge of the panel.
  tft.setCursor(10, 10);

  // Print the label "EEPROM 0x" as a static string prefix.
  tft.print("EEPROM 0x");

  // Append the device I2C address in hexadecimal (e.g., "50" or "51").
  // HEX is a format specifier telling print() to use base-16 representation.
  tft.print(devAddr, HEX);

  // ── Address row: shows which memory cell was written/read ──

  // Switch to yellow for the address row — distinct from the cyan header above.
  tft.setTextColor(ST77XX_YELLOW);

  // Keep text size at 2x for consistent readability.
  tft.setTextSize(2);

  // Move cursor down to y=50, leaving space between header and this row.
  tft.setCursor(10, 50);

  // Print the memory address label and value in hex.
  tft.print("Addr: 0x");
  tft.print(memAddr, HEX);

  // ── Value row: the actual byte read from EEPROM, shown large ──

  // Green for the value — draws the eye to the most important data point.
  tft.setTextColor(ST77XX_GREEN);

  // Scale 4x (each character is 24×32 pixels) to make the value clearly visible.
  tft.setTextSize(4);

  // Move cursor further down to y=100 to separate visually from the address row.
  tft.setCursor(10, 100);

  // Print "Val: " label followed by the numeric byte value in decimal.
  tft.print("Val: ");
  tft.print(value);

  // ── Decimal & Hex row: same value shown in two bases for cross-reference ──

  // White for supplementary information — less visually dominant than the green value above.
  tft.setTextColor(ST77XX_WHITE);

  // Back to 2x size for the smaller supplementary row.
  tft.setTextSize(2);

  // Position at y=160, below the large value block.
  tft.setCursor(10, 160);

  // Print decimal representation.
  tft.print("Dec:");
  tft.print(value);

  // Print hex representation on the same line for side-by-side comparison.
  tft.print("  Hex:0x");
  tft.print(value, HEX);
}

// ════════════════════════════════════════════════════════════
//  FREERTOS TASK: SERVOS
//  Servos 1–4 (indices 0–3): driven LIVE by their potentiometers.
//  Servos 5–6 (indices 4–5): unchanged — still sweep min→max→min,
//    1 second per direction, exactly as before.
// ════════════════════════════════════════════════════════════

// Reads potentiometer i, low-pass filters it, and maps it onto that
// gauge's servo range. Returns the new angle and updates gaugeAngle[i]
// so neoTask's LED-color logic keeps working unchanged.
// potSmooth[] must be a 4-element float array persisted by the caller.
int readGaugeServoAngle(int i, float potSmooth[4]) {
  // Raw ADC reading. ESP32-S2's ADC is 12-bit by default → range 0-4095.
  int raw = analogRead(POT_PINS[i]);

  // Exponential moving-average filter to smooth out ADC/wiper noise so the
  // needle doesn't twitch. ALPHA closer to 1.0 = snappier but jitterier;
  // closer to 0.0 = smoother but slower to respond to fast pot movements.
  const float ALPHA = 0.15;
  potSmooth[i] += ALPHA * ((float)raw - potSmooth[i]);

  // Map the smoothed 0–4095 ADC range onto this gauge's calibrated angle
  // range. map() handles the case where SERVO_MIN > SERVO_MAX correctly
  // (some servos are mounted in reverse), so no special-casing is needed.
  int angle = map((int)potSmooth[i], 0, 4095, SERVO_MIN[i], SERVO_MAX[i]);

  // Clamp defensively in case of ADC noise at the extremes (e.g. a stray
  // reading just past 0 or 4095 rounding outside the intended angle range).
  int lo = min(SERVO_MIN[i], SERVO_MAX[i]);
  int hi = max(SERVO_MIN[i], SERVO_MAX[i]);
  angle  = constrain(angle, lo, hi);

  // Publish for neoTask (LED proximity-color logic reads this).
  gaugeAngle[i] = angle;
  return angle;
}

// FreeRTOS task function. The void* param argument is required by the
// FreeRTOS API but unused here (it allows passing data into the task at
// creation time if needed).
void servoTask(void *param) {
  // Number of discrete angle steps per sweep direction (servos 5-6 only).
  // 100 steps over 1000 ms = 10 ms per step, giving smooth motion.
  const int STEPS       = 100;

  // Delay in milliseconds between each step (1000 ms / 100 steps = 10 ms per step).
  // This also sets how often the potentiometers (servos 1-4) are re-read.
  const int STEP_DELAY  = 1000 / STEPS; // ms per step = 10 ms

  // Attach all 6 servos to their assigned GPIO pins.
  // attach() sets up the PWM signal on that pin.
  for (int i = 0; i < 6; i++) {
    servos[i].attach(SERVO_PINS[i]);
  }

  // Servos 5-6 still need a known starting position for the sweep animation.
  // Servos 1-4 don't — they'll immediately take whatever angle their pot reads.
  for (int i = 4; i < 6; i++) {
    servos[i].write(SERVO_MIN[i]);
  }

  // Wait 500 ms after attaching to give the servos time to physically reach their
  // start positions before the main loop begins.
  vTaskDelay(pdMS_TO_TICKS(500));

  // Per-channel smoothing state for the 4 potentiometers, seeded with a real
  // first reading so the gauges don't sweep up from 0 on power-up.
  static float potSmooth[4];
  for (int i = 0; i < 4; i++) {
    potSmooth[i] = (float)analogRead(POT_PINS[i]);
  }

  bool sweepingUp = true; // Direction for the servo 5-6 sweep
  int  step       = 0;    // Current step within the current sweep direction

  // Infinite loop — FreeRTOS tasks must never return, so they loop forever.
  for (;;) {
    // ── Servos 1–4 (indices 0–3): live potentiometer control ──
    // Re-read and re-write every tick (every STEP_DELAY ms) so the gauges
    // track the physical knobs in near real-time.
    for (int i = 0; i < 4; i++) {
      int angle = readGaugeServoAngle(i, potSmooth);
      servos[i].write(angle);
    }

    // ── Servos 5–6 (indices 4–5): original sweep animation, unchanged ──
    float t = (float)step / STEPS; // 0.0 → 1.0 across the current direction
    for (int i = 4; i < 6; i++) {
      int angle = sweepingUp
        ? (int)(SERVO_MIN[i] + t * (SERVO_MAX[i] - SERVO_MIN[i])) // Min → Max
        : (int)(SERVO_MAX[i] + t * (SERVO_MIN[i] - SERVO_MAX[i])); // Max → Min
      servos[i].write(angle);
    }

    // Advance the sweep; flip direction once a full 1-second pass completes.
    step++;
    if (step > STEPS) {
      step = 0;
      sweepingUp = !sweepingUp;
    }

    // Yield to the FreeRTOS scheduler for STEP_DELAY milliseconds.
    // pdMS_TO_TICKS() converts milliseconds to RTOS tick counts. This is the
    // correct RTOS-aware delay — unlike Arduino's delay(), it lets other
    // tasks run during the wait.
    vTaskDelay(pdMS_TO_TICKS(STEP_DELAY));
  }
}

// ════════════════════════════════════════════════════════════
//  NEO HELPERS
// ════════════════════════════════════════════════════════════

// Returns a short human-readable label for the three possible gauge LED colors.
// Used exclusively by the Serial debug output in neoTask so the monitor shows
// "WHITE", "YELLOW", or "RED" instead of raw hex values like "0xFFFFFF".
// Any unexpected color value falls through to "UNKNOWN" as a safety catch.
const char* colorName(uint32_t color) {
  switch (color) {
    case 0xFFFFFF: return "WHITE ";   // Trailing space aligns columns in Serial output
    case 0xFFFF00: return "YELLOW";
    case 0x00FF00: return "RED   ";   // Trailing spaces align columns in Serial output
    default:       return "UNKNOWN";
  }
}

// ════════════════════════════════════════════════════════════
//  FREERTOS TASK: NEOPIXELS
//  LEDs 1–4  (indices 0–3): gauge proximity indicators.
//    White  = normal range  (> 20% from either limit)
//    Yellow = approaching   (≤ 20% but > 10% from either limit)
//    Red    = near limit    (≤ 10% from either limit)
//  LEDs 5–10 (indices 4–9): classic chase animation.
// ════════════════════════════════════════════════════════════

// FreeRTOS task function that drives the NeoPixel strip.
// LEDs 1–4 reflect the live position of their paired gauge servo relative
// to the servo's mechanical limits. LEDs 5–10 continue the original
// chase pattern: all lit in their unique colors, one dark position moving
// along the segment each second.
void neoTask(void *param) {
  // Brief startup delay to let servoTask attach its servos and write the first
  // angles into gaugeAngle[] before neoTask tries to read them.
  vTaskDelay(pdMS_TO_TICKS(600));

  // Clear the strip and push the blank state before entering the main loop.
  strip.clear();
  strip.show();

  // Tracks which position in the 6-LED chase segment (LEDs 5–10, indices 4–9)
  // is currently dark. Starts at 0 (= LED index 4) and advances each second.
  int chaseOffset = 0;  // 0 → LED 5 is dark, 5 → LED 10 is dark

  // How often gauge colors are re-evaluated. 50 ms gives ~20 samples across the
  // 1-second servo sweep, so the LED color tracks the needle position in near
  // real-time rather than only once per full sweep cycle.
  const int NEO_SAMPLE_MS = 50;

  // The chase advance is kept at 1 second regardless of NEO_SAMPLE_MS.
  // This counter counts 50 ms ticks; once it reaches CHASE_TICKS the dark
  // position moves forward and the counter resets. Decoupling the two rates
  // means changing NEO_SAMPLE_MS never accidentally speeds up the chase.
  const int CHASE_TICKS = 1000 / NEO_SAMPLE_MS;  // = 20 ticks per second
  int       chaseTick   = 0;  // Ticks elapsed since last chase advance

  // Infinite loop — FreeRTOS tasks must never return.
  for (;;) {

    // ── Button state: direct pin poll ──
    // Rather than relying on the ISR to capture the exact transition moment,
    // the pin level is read unconditionally on every 50 ms tick. At that
    // rate the LED responds within one tick of any press or release —
    // imperceptible to a human — while completely avoiding any ambiguity
    // about whether the GPIO settled before the ISR read it.
    // INPUT_PULLUP means the pin rests HIGH; pressing the button pulls it
    // LOW through GND, so LOW == pressed, HIGH == released.
    // If your buttons are wired to VCC instead of GND, change LOW to HIGH.
    leftTurnState = (digitalRead(BTN_LED5_PIN) == HIGH);  // true = switch on left
    rightTurnState = (digitalRead(BTN_LED6_PIN) == HIGH);  // true = switch on right
    lowBeamState = (digitalRead(BTN_LED7_PIN) == HIGH); //true = low beams on
    highBeamState = (digitalRead(BTN_LED8_PIN) == HIGH);  //true = high beams on

    // ── LEDs 1–4 (indices 0–3): gauge proximity indicators ──

    for (int i = 0; i < 4; i++) {
      // Compute the full angular span of this gauge (always positive regardless
      // of whether SERVO_MIN is numerically larger than SERVO_MAX, as some
      // servos are mounted in reverse and have inverted min/max values).
      int range = abs(SERVO_MAX[i] - SERVO_MIN[i]);

      // Read the most-recently-published angle for this gauge.
      // The volatile qualifier on gaugeAngle ensures we always get the latest
      // value written by servoTask rather than a stale cached copy.
      int angle = gaugeAngle[i];

      // Distance (in degrees) from each mechanical limit.
      // Using abs() makes this direction-independent for reversed servos.
      int distFromMin = abs(angle - SERVO_MIN[i]);
      int distFromMax = abs(angle - SERVO_MAX[i]);

      // The proximity warning is based on the closer of the two limits.
      // A gauge near either end is equally at risk, so we take the minimum.
      int nearestEdge = min(distFromMin, distFromMax);

      // Select LED color based on proximity to the nearest limit.
      uint32_t color;
      if (nearestEdge <= range / 10) {
        // Within 10% of min or max → Red (critical: gauge is very close to its limit)
        color = 0x00FF00;
      } else if (nearestEdge <= range / 5) {
        // Within 20% but more than 10% away → Yellow (caution: approaching limit)
        color = 0xFFFF00;
      } else {
        // More than 20% from either limit → White (normal operating range)
        color = 0xFFFFFF;
      }

      // Print one debug line per gauge to the Serial monitor each update cycle.
      // Printed on every tick so you can see the angle moving in real-time.
      // Format: [NEO] G1  angle=  95°  range=89°  edge=  0°  → RED
      // Columns are fixed-width (%3d) so values stay vertically aligned across
      // the four gauge lines, making it easy to compare them at a glance.
      Serial.printf("[NEO] G%d  angle=%3d\xC2\xB0  range=%3d\xC2\xB0  edge=%3d\xC2\xB0  -> %s\n",
                    i + 1, angle, range, nearestEdge, colorName(color));
      // Note: \xC2\xB0 is the UTF-8 encoding of the degree symbol (°).
      // Most serial monitors (Arduino IDE, VSCode, PlatformIO) handle UTF-8 correctly.

      // Write the computed status color into the NeoPixel buffer for this LED.
      // strip.show() at the end of the loop pushes all buffered changes at once.
      strip.setPixelColor(i, color);
    }

    // ── LEDs 5–6 (indices 4–5): hardware interrupt button indicators ──

    // LED 5 (index 4) is controlled exclusively by the GPIO 18 button interrupt.
    // When leftTurnState is true (button held), the LED lights green (BTN_LED_COLOR).
    // When leftTurnState is false (button released), 0 turns the LED fully off.
    // This LED does NOT participate in the chase animation.
    strip.setPixelColor(4, leftTurnState ? TURN_LED_COLOR : 0);

    // LED 6 (index 5) is controlled exclusively by the GPIO 21 button interrupt.
    // Same active-LOW logic: green while pressed, off while released.
    // This LED does NOT participate in the chase animation.
    strip.setPixelColor(5, rightTurnState ? TURN_LED_COLOR : 0);

    // ── LEDs 7–8 (indices 6–7): hardware interrupt button indicators ──

    // LED 7 (index 6) is controlled exclusively by the GPIO 46 button interrupt.
    // When lowBeamState is true (button held), the LED lights blue (BTN_LED_COLOR).
    // When lowBeamState is false (button released), 0 turns the LED fully off.
    // This LED does NOT participate in the chase animation.
    strip.setPixelColor(6, lowBeamState ? HEADLIGHT_LED_COLOR : 0);

    // LED 8 (index 7) is controlled exclusively by the GPIO 38 button interrupt.
    // Same active-LOW logic: blue while pressed, off while released.
    // This LED does NOT participate in the chase animation.
    strip.setPixelColor(7, highBeamState ? HEADLIGHT_LED_COLOR : 0);

    // Transmit the complete updated buffer — gauge LEDs (0–3) and chase LEDs (4–9) —
    // to all NeoPixels in one call. Batching into one show() call prevents visible
    // flicker that would occur if the two segments were pushed separately.
    strip.show();

    // Sleep for NEO_SAMPLE_MS (50 ms) before the next gauge color evaluation.
    // At 50 ms per tick the gauge color is re-evaluated ~20 times per sweep cycle,
    // giving near real-time LED response to needle position changes.
    vTaskDelay(pdMS_TO_TICKS(NEO_SAMPLE_MS));
  }
}

// ════════════════════════════════════════════════════════════
//  FREERTOS TASK: EEPROM + TFT DISPLAY
//  Writes incrementing values to both EEPROMs, cycling
//  through all 256 bytes. Displays result on each TFT.
// ════════════════════════════════════════════════════════════

// FreeRTOS task function that writes incrementing values to both EEPROM chips,
// reads them back, and displays the results on the paired TFT display.
// This tests EEPROM write/read integrity and TFT rendering simultaneously.
void eepromDisplayTask(void *param) {
  // Current memory address for EEPROM 0x50. Starts at 0x00 (first byte).
  // Will increment each cycle to walk through all 256 addresses of the chip.
  uint8_t addr0  = 0;   // Current memory address for EEPROM 0x50

  // Current memory address for EEPROM 0x51. Also starts at 0x00.
  // Both chips are walked in lockstep — same address, different values written.
  uint8_t addr1  = 0;   // Current memory address for EEPROM 0x51

  // Value to write to EEPROM 0x50. Starts at 0 and increments each cycle.
  // Using uint8_t means it naturally wraps from 255 back to 0, cycling through all byte values.
  uint8_t value0 = 0;   // Value written to EEPROM 0x50

  // Value to write to EEPROM 0x51. Starts at 128 (midpoint) to make it distinguishable
  // from value0 on the display — if both showed the same number, you couldn't tell them apart.
  uint8_t value1 = 128; // Different starting value for EEPROM 0x51

  // Infinite loop — continuously cycles through write → read → display → increment.
  for (;;) {
    // ── Write to both EEPROMs ──

    // Write value0 to address addr0 on EEPROM at I2C address 0x50.
    // This stores the byte in non-volatile memory — it will survive a power cycle.
    eepromWrite(EEPROM_ADDR_0, addr0, value0);

    // Write value1 to address addr1 on EEPROM at I2C address 0x51.
    // Sequential writes are fine because eepromWrite() includes the 5 ms write-cycle delay.
    eepromWrite(EEPROM_ADDR_1, addr1, value1);

    // ── Verify read-back ──

    // Read back the byte we just wrote to EEPROM 0x50 at addr0.
    // Comparing the written and read-back values confirms the write succeeded
    // and the EEPROM is functioning correctly. Any mismatch indicates a fault.
    uint8_t readBack0 = eepromRead(EEPROM_ADDR_0, addr0);

    // Read back the byte written to EEPROM 0x51 at addr1 for the same verification purpose.
    uint8_t readBack1 = eepromRead(EEPROM_ADDR_1, addr1);

    // ── Update both TFT displays ──

    // Render the EEPROM 0x50 read-back data onto Display 0 (tft0).
    // The helper function clears the screen and lays out the address and value in color-coded text.
    tftShowEEPROM(tft0, EEPROM_ADDR_0, addr0, readBack0);

    // Render the EEPROM 0x51 read-back data onto Display 1 (tft1).
    // Each display shows only its paired EEPROM's data, making both simultaneously readable.
    tftShowEEPROM(tft1, EEPROM_ADDR_1, addr1, readBack1);

    // Print the same information to Serial for monitoring/debugging over USB.
    // %02X formats the address as a 2-digit uppercase hex value (e.g., "0A" not "A").
    // %d formats the value as a decimal integer for easy comparison with the TFT readout.
    Serial.printf("[EEPROM] 0x50 @ 0x%02X = %d  |  0x51 @ 0x%02X = %d\n",
                  addr0, readBack0, addr1, readBack1);

    // Wait 1 second before the next write cycle.
    // This makes each EEPROM write event visible on the TFT long enough to read,
    // and also prevents writing too rapidly (the EEPROM could handle faster with care,
    // but 1 Hz is appropriate for a human-observable hardware test).
    vTaskDelay(pdMS_TO_TICKS(1000));

    // ── Increment address and value for next cycle ──

    // Advance addr0 to the next EEPROM memory location, wrapping at 0xFF → 0x00.
    // The & 0xFF mask enforces 8-bit rollover (same as uint8_t overflow, but explicit).
    // The 24LC02B has exactly 256 bytes, so this cycles through the entire chip repeatedly.
    addr0  = (addr0  + 1) & 0xFF;  // 24LC02B has 256 bytes (0x00–0xFF)

    // Advance addr1 in lockstep with addr0 — both chips are tested at the same address.
    addr1  = (addr1  + 1) & 0xFF;

    // Increment value0 with 8-bit rollover so it cycles 0→255→0 continuously.
    // Each address in EEPROM 0x50 gets a unique and predictable value on each pass.
    value0 = (value0 + 1) & 0xFF;

    // Increment value1 with rollover — stays offset from value0 by 128, making it
    // easy to visually distinguish the two EEPROMs' data on the respective TFT screens.
    value1 = (value1 + 1) & 0xFF;
  }
}

// ════════════════════════════════════════════════════════════
//  SETUP
// ════════════════════════════════════════════════════════════

// Arduino's setup() function — runs once after power-on or reset.
// Initializes all hardware peripherals and launches the FreeRTOS tasks.
void setup() {
  // Start Serial communication at 115200 baud for debug output over USB.
  // 115200 is the standard high-speed baud rate that matches most serial monitors.
  Serial.begin(115200);

  // Short delay to let the Serial connection stabilize before printing.
  // Without this, the first few characters can be garbled on some systems.
  delay(500);

  // Print a startup banner so you can identify in the serial monitor
  // that the ESP32 has freshly booted and reached this point in setup().
  Serial.println("\n=== ESP32-S2 N4R2 Component Test ===");

  // ── Gauge potentiometers ──

  // Set ADC attenuation to 11dB so analogRead() covers the full 0–3.3V range.
  // Without this the ADC's usable input range is much narrower (~0-1.1V default),
  // which would make the potentiometers only sweep through part of the gauge's angle.
  Serial.println("Setting ADC attenuation for gauge pots...");
  analogSetAttenuation(ADC_11db);

  // ── Backlight ON ──

  // Configure the TFT backlight pin as a digital output.
  // Without this, writing to the pin would have no effect.
  pinMode(TFT_BL, OUTPUT);

  // Drive the backlight pin HIGH to turn on the LED backlight of both TFT displays.
  // The ST7789 panel is transmissive — without the backlight, the screen appears dark
  // even when correctly initialized and displaying content.
  digitalWrite(TFT_BL, HIGH);

  // ── SPI for TFTs ──

  // Initialize the HSPI bus with custom pin assignments.
  // Parameters: SCK pin, MISO pin (-1 = not used, displays are write-only),
  // MOSI pin, SS/CS pin (-1 = managed manually per-display via CS0/CS1 pins).
  tftSPI.begin(TFT_CL, -1, TFT_DA, -1);  // SCK, MISO(none), MOSI, SS(none)

  // ── Init Display 0 ──

  Serial.println("Init TFT 0...");

  // Initialize Display 0 with its pixel dimensions: 135 wide × 240 tall.
  // This tells the driver the panel's physical resolution so it calculates
  // memory addresses correctly when drawing to specific pixel coordinates.
  tft0.init(135, 240);

  // Rotate Display 0 by 90° (rotation=1) to put it in landscape orientation.
  // The ST7789 is naturally portrait; rotation=1 makes width=240, height=135 in software.
  tft0.setRotation(1);

  // Fill the screen with black to clear any garbage pixels from initialization.
  // The display memory is undefined on first power-up; filling it prevents visual noise.
  tft0.fillScreen(ST77XX_BLACK);

  // Set the text color to white for good contrast against the black background.
  tft0.setTextColor(ST77XX_WHITE);

  // Set font scale to 2x for readable startup text.
  tft0.setTextSize(2);

  // Move text cursor to the top-left area (10, 10) with a small margin.
  tft0.setCursor(10, 10);

  // Print an identification label on Display 0 so you know which physical screen is which.
  // This persists briefly before the EEPROM task overwrites it with live data.
  tft0.print("TFT 0 - EEPROM 0x50");

  // ── Init Display 1 ──
  // Same initialization sequence as Display 0, targeting tft1 and its paired EEPROM.

  Serial.println("Init TFT 1...");

  // Initialize Display 1 with the same 135×240 resolution.
  tft1.init(135, 240);

  // Rotate to landscape (same as Display 0 for visual consistency).
  tft1.setRotation(1);

  // Clear Display 1 to black.
  tft1.fillScreen(ST77XX_BLACK);

  // White text on black background.
  tft1.setTextColor(ST77XX_WHITE);

  // 2x font scale.
  tft1.setTextSize(2);

  // Position cursor at top-left with margin.
  tft1.setCursor(10, 10);

  // Print identification label for Display 1 / EEPROM 0x51 pairing.
  tft1.print("TFT 1 - EEPROM 0x51");

  // ── I2C for EEPROMs ──

  Serial.println("Init I2C...");

  // Start the I2C bus using the defined SDA and SCL pins.
  // Wire.begin() sets up the ESP32 as I2C master, which means it drives
  // the clock line and initiates all transactions.
  Wire.begin(I2C_SDA, I2C_SCL);

  // Set the I2C clock speed to 100 kHz (standard mode).
  // The 24LC02B supports up to 400 kHz (fast mode), but 100 kHz is more
  // reliable over longer wires and is more than adequate for this test.
  Wire.setClock(100000);  // 100 kHz — safe for 24LC02B

  // Scan the I2C bus for both expected EEPROM addresses and report results.
  // This is a diagnostic step — if an EEPROM is missing or miswired,
  // the warning on Serial lets you catch it immediately.
  for (uint8_t addr : {EEPROM_ADDR_0, EEPROM_ADDR_1}) {
    // Attempt to open a transmission to this I2C address.
    Wire.beginTransmission(addr);

    // Immediately end the transmission (no data sent) and capture the error code.
    // A return value of 0 means a device acknowledged at this address (ACK received).
    // Non-zero means no device responded (NACK) or a bus error occurred.
    uint8_t err = Wire.endTransmission();

    if (err == 0) {
      // Device responded — EEPROM is present and reachable on the I2C bus.
      Serial.printf("  EEPROM found at 0x%02X\n", addr);
    } else {
      // No ACK received — either the chip is absent, miswired, or at a different address.
      // Printing the error code (1=too long, 2=NACK on addr, 3=NACK on data, 4=other error)
      // helps narrow down the cause.
      Serial.printf("  WARNING: No device at 0x%02X (err %d)\n", addr, err);
    }
  }

  // ── NeoPixels ──

  Serial.println("Init NeoPixels...");

  // Initialize the NeoPixel strip (sets up the data pin and internal state).
  // Must be called before any color operations will have any effect.
  strip.begin();

  // Set global brightness to 60 out of 255 (~24%).
  // Full brightness (255) at 10 LEDs can draw ~600 mA, which may exceed USB power limits.
  // A moderate brightness keeps current draw safe while still being clearly visible.
  strip.setBrightness(60);  // 0–255; keep modest for testing

  // Set all LED colors in the buffer to 0 (off/black).
  // Ensures no leftover color data from a previous session causes unexpected LED states.
  strip.clear();

  // Transmit the cleared (all-off) buffer to the physical LEDs.
  // show() must be called after any buffer changes for them to take effect on the hardware.
  strip.show();

  // ── Button Interrupts ──

  Serial.println("Init button interrupts...");

  // Configure GPIO 18 as an input with the internal pull-up resistor enabled.
  // INPUT_PULLUP holds the pin HIGH when the button is open, so connecting the
  // other side of the button to GND naturally pulls it LOW when pressed —
  // no external resistor required.
  pinMode(BTN_LED5_PIN, INPUT_PULLDOWN);

  // Configure GPIO 21 with the same input pull-up arrangement for the LED 6 button.
  pinMode(BTN_LED6_PIN, INPUT_PULLDOWN);

  // Attach the ISR for LED 5's button on GPIO 18.
  // digitalPinToInterrupt() translates the GPIO number to the correct hardware
  // interrupt number for this MCU — always use it instead of hard-coding the value.
  // CHANGE fires on both falling (press) and rising (release) edges so the volatile
  // btn5State always reflects the actual live state of the button.
  attachInterrupt(digitalPinToInterrupt(BTN_LED5_PIN), isr_btn5, CHANGE);

  // Attach the ISR for LED 6's button on GPIO 21 using the same strategy.
  attachInterrupt(digitalPinToInterrupt(BTN_LED6_PIN), isr_btn6, CHANGE);

  // ── Allocate servo timers ──

  // Reserve hardware timer 0 for ESP32 servo PWM generation.
  // The ESP32-S2 has 4 independent hardware timers. The ESP32Servo library uses them
  // to generate the precise 50 Hz PWM signals that servos require.
  ESP32PWM::allocateTimer(0);

  // Reserve hardware timer 1 for servo use.
  // With 6 servos and 4 timers, the library maps multiple servos to each timer automatically.
  ESP32PWM::allocateTimer(1);

  // Reserve hardware timer 2 for servo PWM.
  ESP32PWM::allocateTimer(2);

  // Reserve hardware timer 3 for servo PWM.
  // Allocating all 4 timers before attaching any servos prevents timer conflicts
  // with other peripherals (like NeoPixels) that might also request timers.
  ESP32PWM::allocateTimer(3);

  Serial.println("Launching FreeRTOS tasks...\n");

  // Create the servo sweep task and assign it to the FreeRTOS scheduler.
  // Parameters: task function, name (for debug), stack size (bytes), parameter (NULL = none),
  // priority (1 = lowest non-idle), and handle pointer to store the task reference.
  // 4096 bytes of stack is sufficient for simple math and servo calls.
  xTaskCreate(servoTask,         "ServoTask",  4096, NULL, 1, &hServoTask);

  // Create the NeoPixel animation task.
  // 4096 bytes is enough for the simple color-buffer loop with no deep call chains.
  xTaskCreate(neoTask,           "NeoTask",    4096, NULL, 1, &hNeoTask);

  // Create the EEPROM + display task with a larger 8192-byte stack.
  // The display helper function (tftShowEEPROM) uses multiple layers of Adafruit_GFX
  // call chains and string formatting, which consumes more stack space than the other tasks.
  xTaskCreate(eepromDisplayTask, "EepromTask", 8192, NULL, 1, &hEepromTask);

  // Confirm all tasks have been submitted to the scheduler.
  // At this point FreeRTOS begins preemptively running all three tasks concurrently.
  Serial.println("All tasks running. Monitoring on Serial at 115200 baud.");
}

// ════════════════════════════════════════════════════════════
//  LOOP  — tasks own all work; loop just prints a heartbeat
// ════════════════════════════════════════════════════════════

// Arduino's loop() runs repeatedly on the main task (Core 1 by default on ESP32).
// Since all real work is done in FreeRTOS tasks, loop() is kept minimal —
// it only prints a memory health report every 5 seconds as a system heartbeat.
void loop() {
  // Print current and historical minimum free heap memory to Serial.
  // ESP.getFreeHeap() shows how much dynamic RAM is available right now.
  // ESP.getMinFreeHeap() shows the lowest it has ever been since boot —
  // if this approaches 0, you risk stack overflows or heap allocation failures.
  // Monitoring this confirms the tasks aren't leaking memory over time.
  Serial.printf("[Heap] Free: %u bytes  |  Min free: %u bytes\n",
                ESP.getFreeHeap(), ESP.getMinFreeHeap());

  // Wait 5 seconds before printing again.
  // Using Arduino's delay() here is fine since the main loop task doesn't
  // do anything critical — the FreeRTOS tasks keep running during this delay.
  delay(5000);
}
