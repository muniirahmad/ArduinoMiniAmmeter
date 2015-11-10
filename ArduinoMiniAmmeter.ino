/********************************************************************************/
/*
This little piece of software was written to get a current reading from an ACS712 Hall Effect sensor to appear on an I2C LCD display. A mini-Arduino-ammeter of sorts.
Nothing really clever or anything. Just simple code. Hope it helps someone out there! :)

Instructions:
1. Power up the device (of course :P )
2. Use a push button or something to trigger the interrupt function. That should "calibrate" the device around the "zero-current" voltage value.
3. Complete your (external) circuit using the + and - leads (preferably as soon as you've calibrated - so the calibration doesn't drift away again... )
4. Take your reading on the LCD!

If you don't have an LCD display, you can always use the Serial port and have the values displayed on your computer screen.

Note that there is still a lot of noise present. So there's a lot of uncertainty in the readings. But surely there should be a way to fix that...
*/
/********************************************************************************/


#include <Wire.h>               //For I2C Communication
#include <LiquidCrystal_I2C.h>  //I2C Library for supporting the I2C interface on the LCD display
 
#define pinIn A0                //Input pin: reading from ACS712 sensor
#define intrPin D2              //Interrupt pin (int0 on the ATmega8A) for starting calibration

//Define pins for I2C interface
#define I2C_ADDR 0x27 // <<- Add your address here (I2C address of the LCD display).
#define Rs_pin 0
#define Rw_pin 1
#define En_pin 2
#define BACKLIGHT_PIN 3
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

//initialising lcd object?
LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

//defining variables & constants
float intref = 2.602;                //internal reference voltage (from bandgap) for this particular ATmega8A chip
float voltsperval = intref/1023.0;   //converting analog value reading to volts
float zerovalue = 2.5;               //initial value for zero-reading (voltage when no current flows)
float value = 0;                     //initial analogReading

void setup(){
  pinMode(A0, INPUT);
  analogReference(INTERNAL);
  lcd.begin(16,2); //depending on display type
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH); //Backlight ON
  lcd.home (); // go home on LCD
  delay(1000); lcd.home(); lcd.clear();
  attachInterrupt(0, calibrate, FALLING);    //Attaching an interrupt to run the "calibrate()" function on Falling edge.
}

void loop(){
  readAnalog();                              //Read the analog value from the sensor
  float readingvalue = voltsperval*value;    //Convert reading to volts
  lcd.setCursor(0,0);
  lcd.print("Value: "); lcd.print(value);    //Print the analog value (for debugging... )
  lcd.setCursor(0,1);
  lcd.print("mA: "); lcd.print((zerovalue - readingvalue)*(1/0.100)*1000);    //Print the actual current reading (the "0.100" comes from the sensor datasheet - 100mV/A for the 20A version that I have)
  delay(250);                                //Delay... so our eyes don't freak out.
}

void calibrate(){
  detachInterrupt(0);                        //Detach interrupt before calculating, so nothing wrong happens...
  zerovalue = voltsperval*(analogRead(A0));  //Set zero-current value, for reference
  delay(20);                                 //Delay again, just to be on the safe side...
  attachInterrupt(0, calibrate, FALLING);    //Attach interrupt again. Falling edge.
}

//Read sensor output - averaging
void readAnalog(){
  value = 0;
  for(int i=1;i<=50;i++){
    value += analogRead(A0);
  }
  value = value/50.0;
}
