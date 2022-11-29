// Functionality in this version
//  * Strain gauge amplifier read and averaged
//  * Hardware pushbutton used for tare of reading
// Notes
//  * Number of samples read to average is somewhat arbitrary, I chose 15 as a tradeoff between time and accuracy

// ******************************   Includes      ******************************
#include <Wire.h>
#include "HX711.h"
// ******************************   Variables     ******************************
HX711 scale;          // Initializes the library for the HX711 Scale

// HX711 circuit wiring
const int     LOADCELL_DOUT_PIN = 3;                                          // HX711 hardware data out pin definition
const int     LOADCELL_SCK_PIN = 2;                                           // HX711 hardware clock pin definition
const long    LOADCELL_OFFSET = 115800;                                       // This is the raw ADC value measured with no external weight added
float         adjusted_scale_value_lbs = 0;                                   // This variable holds the scaled mass in lbs
float         adjusted_scale_value_kgs = 0;                                   // This variable holds the scaled mass in kgs
int           tare_pushbutton   = 0;                                          // Variable for charger connection status 0 is no charger 1 is charger

// ******************************   Setup         ******************************

void setup() 
{
  Wire.begin();
  pinMode(13, INPUT);                                                         // This pin is tied to the 'tare' hardware pushbutton
  Serial.begin(115200);                                                       // Serial baudrate set to 115200 baud
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);                           // Tells the HX711 library what pins are going where
  scale.set_scale(53947.f);                                                   // this value is obtained by calibrating the scale with known weight
  scale.set_offset(LOADCELL_OFFSET);                                          // subtracts this value from raw adc value
}

void loop() 
{
  tare_pushbutton = digitalRead(13);                                          // Checks 'Tare' pushbutton status
  if (scale.is_ready()) 
  {
    adjusted_scale_value_lbs = scale.get_units(15), 1;                        // Average of 15 samples
    adjusted_scale_value_kgs = adjusted_scale_value_lbs * 0.453592;           // Converted value to kgs
    Serial.print("Scale Reading: ");
    Serial.print(adjusted_scale_value_lbs);
    Serial.print(" lbs, ");
    Serial.print(adjusted_scale_value_kgs);
    Serial.println(" kgs");
  } else {}

  
  if (tare_pushbutton == HIGH)                                                // If Hardware 'Tare' pushbutton is held, start tare process
    {    
      Serial.println("Hold to Tare");
      delay(2000);        
      if (tare_pushbutton == HIGH)   
      {
        Serial.println("Release and Step Away");                              // Mechanically I found that stepping away from the bike scale helped in getting a stable tare value
        delay(2000);
        scale.tare();                                                         // This performs the scale tare operation
        Serial.println("Tared");
        delay(500);                                                              
      }
    }
}
