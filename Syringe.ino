/*-----( Import needed libraries )-----*/
#include <LiquidCrystal.h>

//a4988 stepper motor driver
//1 revolution = 800 steps

#include <Stepper.h>

/*-----( Declare objects )-----*/
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //These are the pins used on this shield

/*-----( Declare Constants )-----*/
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

/*-----( Declare Arduino Variables )-----*/
int lcd_key       = 0;
int adc_key_in    = 0;
int adc_key_prev  = 0;
int last_lcd_key = 5;
int menuNumber = 1;
int dirPin = 2;
int stepPin = 3;
int MS1 = 10;
int MS2 = 11;
int MS3 = 12;

/*-----( Declare Syringe Variables )-----*/
double flowRate = 1.0;
double diameter = 1.0;
double runTime = 1.0;
int dir = 1; //1 for extend, 0 for retract
double waitTime = 5.0;

void setup()   /*----( SETUP: RUNS ONCE )----*/
{
  lcd.begin(16, 2);              // start the lcd object

  lcd.blink(); //Sets the cursor to blink on the selected text

  Serial.begin(9600);

  //Set Motor Pins
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);

  //Set to move at 1/16 step (All pins HIGH)
  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, HIGH);


}/*--(end setup )---*/

void loop()   /*----( LOOP: RUNS CONSTANTLY )----*/
{
  int numMenus = 4; //Integer to keep track of how many menus there are

  switch (menuNumber)
  {
    case 1:
      {
        menuNumber = menuNumber + numberMenu(flowRate, "Flow Rate:", "uL/min"); //Creates numerical menu for the flow rate. Value passed as reference
        break;
      }

    case 2:
      {
        menuNumber = menuNumber + numberMenu(diameter, "Diameter:", "cm"); //Creates numerical menu for the diameter of the syringe. Value passed as reference
        break;
      }
    case 3:
      {
        menuNumber = menuNumber + numberMenu(runTime, "Run Time:", "min"); //Creates numerical menu for the time to run the syringe. Value passed as reference
        break;
      }
    case 4:
      {
        menuNumber = menuNumber + manualMenu(); //Creates directional menu
        break;
      }

  }

  //Conditions to handle looping around to beginning or end of menus
  if (menuNumber == 0) menuNumber = numMenus; //If the menu index has been set to 0, set it to the index of the last menu
  if (menuNumber == numMenus + 1) menuNumber = 1; //If the menu index has been set one past the max menu, set it to 1 (the beginning)


}/* --(end main loop )-- */

/*-----( Declare User-written Functions )-----*/

int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor
  delay(5); //switch debounce delay. Increase this delay if incorrect switch selections are returned.
  int k = (analogRead(0) - adc_key_in); //gives the button a slight range to allow for a little contact resistance noise
  if (5 < abs(k)) return btnNONE;  // double checks the keypress. If the two readings are not equal +/-k value after debounce delay, it tries again.
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close

  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in < 710)  return btnSELECT;
  if (adc_key_in < 830)   return btnRIGHT;
  if (adc_key_in < 880)  return btnLEFT;
  if (adc_key_in < 915)  return btnDOWN;
  if (adc_key_in < 970)  return btnUP;


  return btnNONE;  // when all others fail, return this...
}

double manualMenu()
{

  int selectedDigit = 3; //Tracks cursor index
  int breakLoop = 0;
  int menuVal = 0;

  //Create custom LCD characters for up and down arrows
  byte upArrow[8] =
  {
    0b00100,
    0b01010,
    0b10101,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100
  };

  byte downArrow[8] = { //Custom LCD character
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b10101,
    0b01010,
    0b00100
  };

  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);


  //Create Menu Title
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Position:");

  //Create Direction Arrows
  lcd.setCursor(6, 1);
  lcd.write((uint8_t)0);
  lcd.setCursor(8, 1);
  lcd.write((uint8_t)1);

  //Create directional arrows
  lcd.setCursor(0, 1);
  lcd.print("<");
  lcd.setCursor(15, 1);
  lcd.print(">");

  lcd.blink();
  selectedDigit = 6;
  lcd.setCursor(selectedDigit, 1);


  while (1) //Infinite loop to handle text entry. Will exit loop with a return statement
  {

    lcd_key = read_LCD_buttons();  // read the buttons

    if ( (lcd_key != last_lcd_key) && (lcd_key != btnNONE))
    {

      switch (lcd_key)               // depending on which button was pushed, we perform an action
      {

        case btnRIGHT:
          {
            if (selectedDigit == 15)
            {
              breakLoop = 1;
              menuVal = 1;
            }
            else if (selectedDigit == 8) //Skip white space
            {
              selectedDigit = 15;
            }
            else if (selectedDigit == 6)
            {
              selectedDigit = 8;
            }
            else if (selectedDigit == 0)
            {
              selectedDigit = 6;
            }


            lcd.setCursor(selectedDigit, 1);
            break;

          }

        case btnLEFT:
          {
            Serial.print(selectedDigit);
            if (selectedDigit == 0)
            {
              breakLoop = 1;
              menuVal = -1; //Indicate that the LCD should display the left menu
            }


            if (selectedDigit == 6) //Skip white space
            {
              selectedDigit = 0;
            }
            else if (selectedDigit == 8)
            {
              selectedDigit = 6;
            }
            else if (selectedDigit == 15)
            {
              selectedDigit = 8;
            }

            lcd.setCursor(selectedDigit, 1);
            break;
          }

        case btnUP: //If the user presses the up button, automatically highlight the UP arrow and extend the motor. This is for manual, fast movement
          {
            selectedDigit = 6;
            lcd.setCursor(selectedDigit, 1);

            //Set to highest speed
            digitalWrite(MS1, LOW);
            digitalWrite(MS2, LOW);
            digitalWrite(MS3, LOW);

            while ( (read_LCD_buttons() == btnUP) && (selectedDigit ==  6) )
            {
              //Serial.print("Extending Motor ");
              digitalWrite(dirPin, LOW);
              digitalWrite(3, HIGH);
              delay(1);
              digitalWrite(3, LOW);
              delay(1);
            }

            //Reset to lowest speed
            digitalWrite(MS1, HIGH);
            digitalWrite(MS2, HIGH);
            digitalWrite(MS3, HIGH);

            break;
          }

        case btnDOWN: //If the user presses the down button, automatically highlight the DOWN arrow and retract the motor. This is for manual, fast movement
          {
            selectedDigit = 8;
            lcd.setCursor(selectedDigit, 1);

            //Set to highest speed
            digitalWrite(MS1, LOW);
            digitalWrite(MS2, LOW);
            digitalWrite(MS3, LOW);

            while ( (read_LCD_buttons() == btnDOWN) && (selectedDigit ==  8) )
            {
              //Serial.print("Retracting Motor ");
              digitalWrite(dirPin, HIGH);
              digitalWrite(3, HIGH);
              delay(1);
              digitalWrite(3, LOW);
              delay(1);
            }

            //Reset to lowest speed
            digitalWrite(MS1, HIGH);
            digitalWrite(MS2, HIGH);
            digitalWrite(MS3, HIGH);

            break;
          }

        case btnSELECT: //Runs the motor with the current settings
          {
            //RUN MOTOR//
            double i = 0;

            waitTime =  (((60 * 3.1415 * (diameter / 2) * (diameter / 2) * 0.08 * 1000) / (3200 * 0.001 * flowRate)) - 10); //Converts the flow rate to a time to wait between pulses

            double delayms = floor(waitTime);
            double delayus = (waitTime - delayms) * 1000;

            double codeTimeOffset = 44;
            
            Serial.print("Running Motor");




            
        

            digitalWrite(dirPin, LOW);

            double startTime = millis(); //Save initial time
            double currentTime = startTime; //Save current time

            //while (currentTime - startTime <= 60000 * runTime) //Multiply by 60,000 to convert mins to ms
            for (i = 0; i < 3200 / 2; i++) //1/2 rotation
            {

              //codeTimeStart = millis();

              digitalWrite(3, HIGH);
              delay( 10 ); //"On" pulse
              digitalWrite(3, LOW);
              delay(delayms);
              delayMicroseconds((delayus - codeTimeOffset));
              

              currentTime = millis(); //Save current time
              //codeTimeOffset = (codeTimeStart - millis() );
            }

            //Serial.print(waitTime);
            //Serial.print(" -- ");
            Serial.print(currentTime - startTime);
            //Serial.print(" -- ");
            //Serial.print(delayms);
            //Serial.print(" -- ");
            //Serial.print(delayus);
            //}

            //            while ( (read_LCD_buttons() == btnSELECT) && (selectedDigit ==  8) ) //While the down button is selected, retract the motor
            //            {
            //              Serial.print("Retracting Motor ");
            //              digitalWrite(dirPin, HIGH);
            //              digitalWrite(3, HIGH);
            //              delay(1);
            //              digitalWrite(3, LOW);
            //              delay(waitTime);
            //            }

            break; //Goes to next menu
          }
        case btnNONE:
          {
            //Do nothing
            break;
          }
      }/* --(end switch )-- */

    }/* --(end if )-- */

    last_lcd_key = lcd_key;

    if (breakLoop == 1) break;

  }

  return menuVal;

}

double numberMenu (double &value, String menuText, String unit)
{

  int selectedDigit = 3; //Tracks cursor index
  int breakLoop = 0;
  int menuVal = 0;
  int unitIndex = ((13 - unit.length()) + 1); //Offsets unit index based on length

  lcd.clear();
  lcd.noBlink(); //Don't want the LCD to blink while we're writing the menu info
  lcd.setCursor(0, 0);
  lcd.print(menuText);

  //Create directional arrows
  lcd.setCursor(0, 1);
  lcd.print("<");
  lcd.setCursor(15, 1);
  lcd.print(">");

  //Create units text
  lcd.setCursor(unitIndex, 1); //Offsets starting position
  lcd.print(unit);

  //Print variable value
  printNumber(value, selectedDigit);

  //Enable blinking cursor
  lcd.blink();

  while (1) //Infinite loop to handle text entry. Will exit loop with a return statement
  {

    //Serial.print(selectedDigit);
    lcd_key = read_LCD_buttons();  // read the buttons

    if ( (lcd_key != last_lcd_key) && (lcd_key != btnNONE))
    {

      switch (lcd_key)               // depending on which button was pushed, we perform an action
      {
        case btnRIGHT: //Moves the cursor to the right
          {
            if (selectedDigit == 15) //If the cursor is at the very right, indicate that the menu should move to the right
            {
              breakLoop = 1;
              menuVal = 1;
            }

            if (selectedDigit < 15)
            {
              selectedDigit = selectedDigit + 1;
            }

            //Condition to check for decimal
            if (selectedDigit == 4)
            {
              selectedDigit++;
            }



            //Condition to check for moving to right arrow
            if (selectedDigit == 7) selectedDigit = 15;
            if (selectedDigit == 1) selectedDigit = 2;
            lcd.setCursor(selectedDigit, 1);

            break;
          }
        case btnLEFT: //Moves the cursor to the left
          {

            if (selectedDigit == 0)
            {
              breakLoop = 1;
              menuVal = -1;
            }

            if (selectedDigit > 0) //If in range, increment the selected digit
            {
              selectedDigit = selectedDigit - 1;
            }

            //Condition to check for decimal
            if (selectedDigit == 4)
            {
              selectedDigit--;
            }

            //Condition to check for moving to left arrow, to skip whitespace
            if (selectedDigit == 1) selectedDigit = 0;
            if (selectedDigit == 14) selectedDigit = 6;


            lcd.setCursor(selectedDigit, 1);



            break;
          }
        case btnUP: //Increses the selected digit
          {
            switch (selectedDigit) //Increases the value based on which digit is selected
            {

              case 2:
                value = value + 10;

                break;

              case 3:
                value = value + 1;

                break;

              case 5:
                value = value + 0.1;

                break;

              case 6:
                value = value + 0.01;

                break;
            }

            if (value >= 100) value = 99;

            printNumber(value, selectedDigit);


            break;
          }

        case btnDOWN: //Decreases the selected digit based on which digit is selected
          {

            switch (selectedDigit)
            {

              case 2:
                value = value - 10;

                break;

              case 3:
                value = value - 1;

                break;

              case 5:
                value = value - 0.1;

                break;

              case 6:
                value = value - 0.01;

                break;
            }

            if (value <= 0.01)
            {
              value = 0.01;
            }

            printNumber(value, selectedDigit); //Update

            break;
          }
        case btnSELECT: //Goes to the next menu
          {

            breakLoop = 1;
            menuVal = 1;

            break; //Goes to next menu
          }
        case btnNONE:
          {
            //Do nothing
            break;
          }
      }/* --(end switch )-- */

    }/* --(end if )-- */

    last_lcd_key = lcd_key;

    if (breakLoop == 1) break;

  }

  return menuVal; //Returns either -1 or 1 to indicate which menu to switch to

}

//Prints the variable number to the LCD screen after it has been updated
void printNumber(double number, int selectedDigit)
{
  //Print Number
  if (number >= 10) //If the number is greater than 10, print the number starting at index 2
  {
    lcd.setCursor(2, 1);
    lcd.print(number);
  }
  else if (number < 10) //If the number is less than 10, sets a leading 0 at index 2 and then prints the number at index 3
  {
    lcd.setCursor(2, 1);
    lcd.print("0");
    lcd.setCursor(3, 1);
    lcd.print(number);
  }

  lcd.setCursor(selectedDigit, 1); //Returns the cursor to the original position
}

//Function to move the motor a specified number of steps. If moving with a flow rate, convert the flow rate to steps first.
//void moveMotor(string direction, double speed)
////{
//   / digitalWrite(3, HIGH);
//    delay(10);
//    digitalWrite(3, LOW);
//    delay(500);
//}


