#include "SevenSegmentTM1637.h"

SevenSegmentTM1637 displaySET(10, 9); //Setup SET display
SevenSegmentTM1637 displayROT(8, 7); //Setup ROT display

uint8_t CLK_PIN = 12; //CLK Encoder pin
uint8_t DT_PIN = 11; //DT Encoder pin

bool old_clk; //Value used to check encoder rotation
bool new_clk; //Value used to check encoder rotation

uint16_t throttle_set = 1000; //Throttle value sent to the motor controller
uint16_t throttle_rot = 1000; //Throttle value changed by the rotary encoder
uint16_t throttle_limit_up = 2000; //Upper Limit of the throttle
uint16_t throttle_limit_dn = 1000; //Lower Limit of the throttle
uint8_t throttle_increment = 5; //Real value is double due to the design of the encoder

void setup()
{
  Serial.begin(115200);
  
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);

  //Setup display
  displaySET.begin();
  displayROT.begin();

  //Setup display backlight
  displaySET.setBacklight(100);
  displayROT.setBacklight(100);

  //Fill display segments - used to find faults in the display
  displaySET.print("8888");
  displayROT.print("8888");
  
  delay(1000);

  //Clear display to prevent persistent values
  displaySET.clear();
  displayROT.clear();

  //Set old_clk to current CLK value
  old_clk = digitalRead(CLK_PIN);

  //Set initial display value
  displayROT.print(throttle_rot);
}

void loop()
{
  //Set new_clk to current CLK value
  new_clk = digitalRead(CLK_PIN);

  //Check if CLK has changed
  if(new_clk != old_clk)
  {
    //Check direction and upper limit
    if(digitalRead(DT_PIN) != new_clk
    && throttle_rot < throttle_limit_up)
    {
      //Increment throttle_rot value up
      throttle_rot += throttle_increment;
    }
    //Check direction and lower limit
    else if(digitalRead(DT_PIN) == new_clk
    && throttle_rot > throttle_limit_dn)
    {
      //Increment throttle_rot value down
      throttle_rot -=throttle_increment;
    }

    displayROT.print(throttle_rot);
    
    //Set olc_clk value for next loop
    old_clk = new_clk;

    //Print current throttle_rot value
    Serial.println(throttle_rot);
  }
}
