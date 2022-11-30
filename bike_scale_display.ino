// Functionality in this version
//  * Strain gauge amplifier read and averaged
//  * Hardware pushbutton used for tare of reading
//  * Custom character used for indicating internal battery state of charge
// Notes
//  * I had tried to use 'big character' libraries but none seem to be directly compatible with the sparkfun I2C lcd, worth looking into for version 2
// ******************************   Includes      *************************************************************************

#include <Wire.h>
#include "HX711.h"
#include <SerLCD.h>                                                           // Library: http://librarymanager/All#SparkFun_SerLCD

// ******************************   Variables     *************************************************************************
HX711 scale;                                                                  // Initializes the library for the HX711 Scale
SerLCD lcd;                                                                   // Initialize the library with default I2C address 0x72

// HX711 circuit wiring
const int     LOADCELL_DOUT_PIN = 3;                                          // HX711 hardware data out pin definition
const int     LOADCELL_SCK_PIN = 2;                                           // HX711 hardware clock pin definition

// Byte arrays for custom characters 
byte charger_con[8]       =  {0b01110,0b11111,0b10101,0b10001,0b11011,0b11011,0b11111,0b11111,};    // Battery on charger
byte battery_100[8]       =  {0b01110,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,};    // Battery 100%
byte battery_080[8]       =  {0b01110,0b10001,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,};    // Battery 80%
byte battery_060[8]       =  {0b01110,0b10001,0b10001,0b11111,0b11111,0b11111,0b11111,0b11111,};    // Battery 60%
byte battery_040[8]       =  {0b01110,0b10001,0b10001,0b10001,0b11111,0b11111,0b11111,0b11111,};    // Battery 40%
byte battery_020[8]       =  {0b01110,0b10001,0b10001,0b10001,0b10001,0b11111,0b11111,0b11111,};    // Battery 20%
byte battery_010[8]       =  {0b01110,0b10001,0b10001,0b10001,0b10001,0b10001,0b11111,0b11111,};    // Battery 10%
byte battery_002[8]       =  {0b01110,0b10001,0b10001,0b10001,0b10001,0b10001,0b10001,0b11111,};    // Battery 2%

// Scale and battery variables
const long    LOADCELL_OFFSET                       = 115800;                 // This is the raw ADC value measured with no external weight added
float         adjusted_scale_value_lbs              = 0;                      // This variable holds the scaled mass in lbs
float         adjusted_scale_value_kgs              = 0;                      // This variable holds the scaled mass in kgs
int           tare_pushbutton                       = 0;                      // Variable for charger connection status 0 is no charger 1 is charger
int           charger_detect                        = 0;                      // Variable for charger connection status 0 is no charger 1 is charger
const int     battery_sense_pin                     = A3;                     // Analog input pin that connects directly to 1S 18650 Cell
int           battery_voltage_sense_initial         = 0;                      // Variable for batt sense filter
int           battery_voltage_filtered              = 0;                      // Variable for batt sense filter
int           battery_mapped_voltage                = 0;                      // Variable for batt in milivolts

// ******************************   SETUP        *************************************************************************

void setup() 
{
  Wire.begin();
  pinMode(13, INPUT);                                                         // This pin is tied to the 'tare' hardware pushbutton
  pinMode(11, INPUT);                                                         // Pin 11 is charger detection input logic high = charger present, logic low no charger present
  Serial.begin(115200);                                                       // Serial baudrate set to 115200 baud
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);                           // Tells the HX711 library what pins are going where
  scale.set_scale(53947.f);                                                   // This value is obtained by calibrating the scale with known weight
  scale.set_offset(LOADCELL_OFFSET);                                          // Subtracts this value from raw adc value
  lcd.begin(Wire);                                                            // Set up the LCD for I2C communication
  lcd.setBacklight(255, 255, 255);                                            // Set backlight to bright white
  //lcd.setContrast(5);                                                       // Set contrast. Lower to 0 for higher contrast.
  lcd.createChar(0,charger_con);                                              // Create custom character 0 for battery on charger from byte array charger_con
  lcd.createChar(1,battery_100);                                              // Create custom character 1 for battery at 100% from byte array battery_100
  lcd.createChar(2,battery_080);                                              // Create custom character 1 for battery at 80% from byte array battery_80
  lcd.createChar(3,battery_060);                                              // Create custom character 1 for battery at 60% from byte array battery_60
  lcd.createChar(4,battery_040);                                              // Create custom character 1 for battery at 40% from byte array battery_40
  lcd.createChar(5,battery_020);                                              // Create custom character 1 for battery at 20% from byte array battery_20
  lcd.createChar(6,battery_010);                                              // Create custom character 1 for battery at 10% from byte array battery_10
  lcd.createChar(7,battery_002);                                              // Create custom character 1 for battery at 02% from byte array battery_02
  lcd.clear();                                                                // Clear the display - this moves the cursor to home position as well
}
// ******************************   Main Loop     *************************************************************************

void loop() 
{
  tare_pushbutton = digitalRead(13);                                          // Checks 'Tare' pushbutton status
  battery_measure();
  if (scale.is_ready()) 
  {  
    lcd.home();
    adjusted_scale_value_lbs = scale.get_units(10), 1;                        // Average of 10 samples
    adjusted_scale_value_kgs = adjusted_scale_value_lbs * 0.453592;           // Converted value to kgs
    lcd.setCursor(1, 0);                                                      // Column 1, row 0
    lcd.print(adjusted_scale_value_lbs);                                      // Prints 
    lcd.print("  ");                                                          // Stupid lcd trick to write over leftover variables. When mass goes past 10.0 and then returns to less than 10.0 it leaves over the previous character. 
    lcd.setCursor(6, 0);                                                      
    lcd.print(" lbs");                                                        // Keeps unit of measure in fixed position
    lcd.setCursor(1, 1);                                                      // Column 1, row 1
    lcd.print(adjusted_scale_value_kgs);
    lcd.print("  ");
    lcd.setCursor(6, 1);                                                      // Keeps unit of measure in fixed position
    lcd.print(" kgs");   
  } else {}
  
  if (tare_pushbutton == HIGH)                                                // If Hardware 'Tare' pushbutton is held, start tare process
    { 
      lcd.clear();                                                            // Clear the display - this moves the cursor to home position as well
      lcd.setCursor(0,0);                                                     // Column 0, row 0
      lcd.print("  Hold To Tare  ");                                          
      delay(2000);                                                            // For making sure you actually want to tare the scale and not that you bumped the tare button
      if (tare_pushbutton == HIGH)   
      {
        lcd.setCursor(0,0);                                                   // Column 0, row 0
        lcd.print("Release And Wait");
        delay(3000);
        scale.tare();                                                         // This performs the scale tare operation
        lcd.setCursor(0,0);                                                   // Column 0, row 0
        lcd.print("      Tare      ");
        lcd.setCursor(0,1);                                                   // Column 0, row 1
        lcd.print("    Complete!   ");
        delay(1000);  
        lcd.clear();                                                          // Clear the display - this moves the cursor to home position as well                                               
      }
    }
      charger_detect = digitalRead(11);
  if (charger_detect == HIGH)                                                 // Charger is connected
    {    
      lcd.setCursor(15,0);
      lcd.write(byte(0));
      battery_mapped_voltage = 5000;
    }
    if(battery_mapped_voltage <= 4200 && battery_mapped_voltage > 4000)       // Cell between 4.2 and 4.0v (100% state of charge)
    {    
      lcd.setCursor(15,0);
      lcd.write(byte(1));
    }
    if(battery_mapped_voltage <= 4000 && battery_mapped_voltage > 3800)       // Cell between 4.0 and 3.8v (80% state of charge)
    { 
      lcd.setCursor(15,0);
      lcd.write(byte(2));
    }
    if(battery_mapped_voltage <= 3800 && battery_mapped_voltage > 3600)       // Cell between 3.8 and 3.6v (60% state of charge)
    { 
      lcd.setCursor(15,0);
      lcd.write(byte(3));
    }    
    if(battery_mapped_voltage <= 3600 && battery_mapped_voltage > 3400)       // Cell between 3.6 and 3.4v (40% state of charge)
    {    
      lcd.setCursor(15,0);
      lcd.write(byte(4));
    }  
    if(battery_mapped_voltage <= 3400 && battery_mapped_voltage > 3200)       // Cell between 3.4 and 3.2v (20% state of charge)
    {    
      lcd.setCursor(15,0);
      lcd.write(byte(5));
    }    
    if(battery_mapped_voltage <= 3200 && battery_mapped_voltage > 3000)       // Cell between 3.2 and 3.0v (10% state of charge)
    {    
      lcd.setCursor(15,0);
      lcd.write(byte(6));
    }       
    if(battery_mapped_voltage < 3000)                                         // Cell less than 3.0V       (2% state of charge)
    {    
      lcd.setCursor(15,0);
      lcd.write(byte(7));
    }
}

void battery_measure()
  {
  int filter_weight = .2;                                                     // Filter range is 0.0 to 1.0 the higher the value the quicker the response to impulses
  battery_voltage_sense_initial = analogRead(battery_sense_pin);
    delay(12);
  battery_voltage_filtered = (filter_weight*(analogRead(battery_sense_pin)))+((1-filter_weight)*(battery_voltage_sense_initial));
  battery_mapped_voltage = map(battery_voltage_filtered, 0, 1023, 0, 5000);
  }
