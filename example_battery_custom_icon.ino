// Functionality in this version
// * Measures single LiPo cell voltage and displays battery status as custom character / icon.
// * Voltage measurement has a proportional filter and is defined under the function battery_measure()

// Notes

// ******************************   Includes      ************************************************************
#include <Wire.h>
#include <SerLCD.h> 
// ******************************   Variables     ************************************************************

SerLCD lcd;                                                                 // Initialize the library with default I2C address 0x72
int       charger_detect = 0;                                               // Variable for charger connection status 0 is no charger 1 is charger
const int battery_sense_pin = A3;                                           // Analog input pin that connects directly to 1S 18650 Cell
int       battery_voltage_sense_initial         = 0;
int       battery_voltage_filtered              = 0;
int       battery_mapped_voltage                = 0;
float     tare_value                            = 0;

byte charger_con[8]       =  {0b01110,0b11111,0b10101,0b10001,0b11011,0b11011,0b11111,0b11111,};
byte battery_100[8]       =  {0b01110,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,};
byte battery_080[8]       =  {0b01110,0b10001,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,};
byte battery_060[8]       =  {0b01110,0b10001,0b10001,0b11111,0b11111,0b11111,0b11111,0b11111,};
byte battery_040[8]       =  {0b01110,0b10001,0b10001,0b10001,0b11111,0b11111,0b11111,0b11111,};
byte battery_020[8]       =  {0b01110,0b10001,0b10001,0b10001,0b10001,0b11111,0b11111,0b11111,};
byte battery_010[8]       =  {0b01110,0b10001,0b10001,0b10001,0b10001,0b10001,0b11111,0b11111,};
byte battery_002[8]       =  {0b01110,0b10001,0b10001,0b10001,0b10001,0b10001,0b10001,0b11111,};

void setup() {
  Serial.begin(115200);                                                       //115200 baudrate 
  Wire.begin();
  lcd.begin(Wire);                                                            //Set up the LCD for I2C communication
  lcd.setBacklight(255, 255, 255);                                            //Set backlight to bright white
  lcd.setContrast(4);                                                         //Set contrast. Lower to 0 for higher contrast.
  lcd.clear();                                                                //Clear the display - this moves the cursor to home position as well
  pinMode(11, INPUT);                                                         //Pin 11 is charger detection input logic high = charger present, logic low no charger present
  }

void loop() {                                                                 // This loop grabs mapped filtered battery voltage and displays the according custom character
  battery_measure();
  charger_detect = digitalRead(11);
  
  if (charger_detect == HIGH)                                                 // Charger is connected
  {    
    lcd.createChar(0,charger_con);
    lcd.setCursor(15,0);
    lcd.writeChar(charger_con);
    battery_mapped_voltage = 5000;
  }
  if(battery_mapped_voltage <= 4200 && battery_mapped_voltage > 4000)         // Cell between 4.2 and 4.0v (100% state of charge)
  {    
    lcd.createChar(0,battery_100);
    lcd.setCursor(15,0);
    lcd.writeChar(battery_100);
  }
  if(battery_mapped_voltage <= 4000 && battery_mapped_voltage > 3800)         // Cell between 4.0 and 3.8v (80% state of charge)
  {    
    lcd.createChar(0,battery_080);
    lcd.setCursor(15,0);
    lcd.writeChar(battery_080);
  }
  if(battery_mapped_voltage <= 3800 && battery_mapped_voltage > 3600)         // Cell between 3.8 and 3.6v (60% state of charge)
  {    
    lcd.createChar(0,battery_060);
    lcd.setCursor(15,0);
    lcd.writeChar(battery_060);
  }    
  if(battery_mapped_voltage <= 3600 && battery_mapped_voltage > 3400)         // Cell between 3.6 and 3.4v (40% state of charge)
  {    
    lcd.createChar(0,battery_040);
    lcd.setCursor(15,0);
    lcd.writeChar(battery_040);
  }  
  if(battery_mapped_voltage <= 3400 && battery_mapped_voltage > 3200)         // Cell between 3.4 and 3.2v (20% state of charge)
  {    
    lcd.createChar(0,battery_020);
    lcd.setCursor(15,0);
    lcd.writeChar(battery_020);
  }    
  if(battery_mapped_voltage <= 3200 && battery_mapped_voltage > 3000)         // Cell between 3.2 and 3.0v (10% state of charge)
  {    
    lcd.createChar(0,battery_010);
    lcd.setCursor(15,0);
    lcd.writeChar(battery_010);
  }       
  if(battery_mapped_voltage < 3000)                                           // Cell less than 3.0V       (2% state of charge)
  {    
    lcd.createChar(0,battery_002);
    lcd.setCursor(15,0);
    lcd.writeChar(battery_002);
  }
  delay(250);
}


void battery_measure()
  {
  int filter_weight = .2;                                                     // Filter range is 0.0 to 1.0 the higher the value the quicker the response to impulses
  battery_voltage_sense_initial = analogRead(battery_sense_pin);
  delay(20);
  battery_voltage_filtered = (filter_weight*(analogRead(battery_sense_pin)))+((1-filter_weight)*(battery_voltage_sense_initial));
  battery_mapped_voltage = map(battery_voltage_filtered, 0, 1023, 0, 5000);
  Serial.println(battery_mapped_voltage);                                     // Print out measured voltage of battery pack
  }
