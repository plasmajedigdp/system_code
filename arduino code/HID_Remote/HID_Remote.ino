#include "SevenSegmentTM1637.h"

SevenSegmentTM1637 displaySET(10, 9); //Setup SET display
SevenSegmentTM1637 displayROT(8, 7); //Setup ROT display

uint8_t CLK_PIN = 12; //Encoder CLK pin
uint8_t DT_PIN = 11; //Encoder DT pin
uint8_t SW_PIN = 2; //Encode Switch pin - must be on an interrupt pin

bool old_clk; //Value used to check encoder rotation
bool new_clk; //Value used to check encoder rotation

uint16_t throttle_limit_up = 2000; //Upper Limit of the throttle
uint16_t throttle_limit_dn = 1000; //Lower Limit of the throttle
uint16_t throttle_set = throttle_limit_dn; //Throttle value sent to the motor controller
uint16_t throttle_rot = throttle_limit_dn; //Throttle value changed by the rotary encoder
uint8_t throttle_increment = 5; //Real value is double due to the design of the encoder

//Can't use a delay function in an interrupt so must use large counter numbers
uint32_t button_hold = 0; //Counter to store how long the button has been held
uint32_t length_hold = 70000; //How long (in clock ticks) before the display is reset

void setup()
{
  //Setup Serial Connection
  Serial.begin(115200);

  //Configure Pins
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);

  //Attach the interrupt for the encoder button
  attachInterrupt(digitalPinToInterrupt(SW_PIN), button_press, FALLING);

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
  update_throttle_set(throttle_limit_dn);
  update_throttle_rot(throttle_limit_dn);
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
      //Increment throttle_rot value up and update
      update_throttle_rot(throttle_rot + throttle_increment);
    }
    //Check direction and lower limit
    else if(digitalRead(DT_PIN) == new_clk
    && throttle_rot > throttle_limit_dn)
    {
      //Increment throttle_rot value down and update
      update_throttle_rot(throttle_rot - throttle_increment);
    }

    //Display the new rotation throttle on the rotation display
    update_throttle_rot(throttle_rot);
    
    //Set olc_clk value for next loop
    old_clk = new_clk;
  }
}

//Interrupt routine when the button is pressed
void button_press()
{
  //While the button is held down - count up
  while(!digitalRead(SW_PIN)
  && button_hold < length_hold)
  {
    //Increment
    button_hold++;
  }

  //If the button is held down long enough
  if(button_hold >= length_hold)
  {
    //Print reset message first
    Serial.println("~Throttle Reset");
    
    //Reset the throttles to minimum
    update_throttle_set(throttle_limit_dn);
    update_throttle_rot(throttle_limit_dn);
  }

  //If button is not held long enough to reset
  else
  {
    //Set current throttle value
    update_throttle_set(throttle_rot);
  }

  //Reset hold counter
  button_hold = 0;
}

//Updates the value of the rotational encoder throttle
void update_throttle_rot(uint16_t new_value)
{
  //Update to new throttle value
  throttle_rot = new_value;

  //Set new value on display
  displayROT.print(throttle_rot);

  //Print current throttle_rot value
  Serial.print("Current Rotation: ");
  Serial.println(throttle_rot);
}

//Updates the value of the set throttle
void update_throttle_set(uint16_t new_value)
{
  //Update to new throttle value
  throttle_set = new_value;

  //Set new value on display
  displaySET.print(throttle_set);

  //Print current throttle_set value
  Serial.print("~Throttle Set: ");
  Serial.println(throttle_set);
}
