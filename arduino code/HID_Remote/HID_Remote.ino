//Decided not to use an interrupt for button presses as this limits the number of pins
//available

//Include Libraries
#include "SevenSegmentTM1637.h"
#include <SoftwareSerial.h>

//Define Displays
SevenSegmentTM1637 displaySET(10, 9); //Setup SET display
SevenSegmentTM1637 displayROT(8, 7); //Setup ROT display

//Define Software Serial
SoftwareSerial comms(6, 5);

uint8_t ARM_PIN = 3; //Arm Switch Pin

uint8_t CLK_PIN = 12; //Encoder CLK pin
uint8_t DT_PIN = 11; //Encoder DT pin
uint8_t SW_PIN = 2; //Encoder Switch pin

bool old_arm_pos = LOW; //Value used to check arm switch
bool new_arm_pos = LOW; //Value used to check arm switch - compared against old_arm_pos

bool old_clk; //Value used to check encoder rotation
bool new_clk; //Value used to check encoder rotation - compared against old_clk

uint16_t throttle_limit_up = 2000; //Upper Limit of the throttle
uint16_t throttle_limit_dn = 1000; //Lower Limit of the throttle
uint16_t throttle_set = throttle_limit_dn; //Throttle value sent to the motor controller
uint16_t throttle_rot = throttle_limit_dn; //Throttle value changed by the rotary encoder
uint8_t throttle_increment = 5; //Real value is double due to the design of the encoder

//Button event variables
uint32_t button_hold = 0; //Counter to store how long the button has been held
uint32_t length_hold = 70000; //How long (in clock ticks) before the display is reset
bool button_toggle = false; //Makes sure the button cannot be held continuously

//Communication variable
uint16_t data_transmit = 0; //Variable to hold the data when it is sent from comms

//Variable to express whether or not the motor is armed
bool arm_state = false; //The arm state sent to the motor controller

//Heartbeat variables
uint16_t heartbeat_count = 0; //Counter to store how long since the last heartbeat
uint16_t heartbeat_max = 800; //How long (in clock ticks) between each heartbeat
//heartbeat_max will have to be varied to cope with more functions in the loop
// - More functions in the loop = fewer ticks between the heartbeats sent as
//   heartbeat_count will be incremented slower

//##############################################################################Setup
void setup()
{
  //Setup Functions
  setup_connections();
  setup_pins();
  setup_display();

  //Set old_clk to current CLK value
  old_clk = digitalRead(CLK_PIN);
  
  //Set initial display value
  update_throttle_set(arm_state, throttle_limit_dn);
  update_throttle_rot(throttle_limit_dn);
}

//////////////////////////////////////////////////////////////////////Setup Functions
void setup_connections()
{
  //Setup serial connection
  Serial.begin(115200);

  //Setup software serial connection
  comms.begin(57600);
}

void setup_pins()
{
  //Configure pins
  pinMode(ARM_PIN, INPUT);
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);
}

void setup_display()
{
  //Setup display
  displaySET.begin();
  displayROT.begin();

  //Setup display backlight
  displaySET.setBacklight(100);
  displayROT.setBacklight(100);

  //Fill display segments - used to find faults in the display
  displaySET.print("8888");
  displayROT.print("8888");

  //Allow time for the user to inspect the display segments
  delay(1000);

  //Clear display to prevent persistent values
  displaySET.clear();
  displayROT.clear();
}

//###############################################################################Loop
void loop()
{
  ////////////////////////////////////////////////////////////Loop - Encoder Rotation
  //Set new_clk to current CLK value
  new_clk = digitalRead(CLK_PIN);

  //Check if CLK has changed
  if(new_clk != old_clk)
  {
    //Call function that handles rotation event
    encoder_rotation();
  }

  //////////////////////////////////////////////////////////////Loop - Encoder Button
  //Check if button is pressed
  if(digitalRead(SW_PIN) == LOW
  && button_toggle == false)
  {
    //Call function that handles button press event
    button_press();
  }
  //Ensure the button cannot be held down
  if(digitalRead(SW_PIN) == HIGH)
  {
    //Reset button toggle variable
    button_toggle = false;
  }

  //////////////////////////////////////////////////////////////////Loop - Arm Switch
  //Determine arm switch position
  new_arm_pos = digitalRead(ARM_PIN);

  //Compare position to the one recorded last time
  //The conditions make sure the event is only triggered once
  if(old_arm_pos != new_arm_pos
  && new_arm_pos == HIGH)
  {
    arm_state = true;
    update_throttle_set(arm_state, throttle_set);
  }
  if(old_arm_pos != new_arm_pos
  && new_arm_pos == LOW)
  {
    arm_state = false;
    update_throttle_set(arm_state, throttle_set);
  }

  old_arm_pos = new_arm_pos;

  ///////////////////////////////////////////////////////////////////Loop - Heartbeat
  //Check to see whether the heartbeat is due to be sent
  if(heartbeat_count == heartbeat_max)
  {
    send_heartbeat();
    heartbeat_count = 0;
  }
  //Otherwise increment the heartbeat count
  else
  {
    heartbeat_count++;
  }

  //////////////////////////////////////////////////////////////////Loop - Comms Data
  //Read the comms serial data
  /*while(comms.available())
  {
    
  }*/
}

///////////////////////////////////////////////////////////////Encoder Event Handling
//Function for when encoder has rotated
void encoder_rotation()
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
  
  //Set olc_clk value for next loop
  old_clk = new_clk;
}

//Function for when the button is pressed
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
    update_throttle_set(arm_state, throttle_limit_dn);
    update_throttle_rot(throttle_limit_dn);
  }

  //If button is not held long enough to reset
  else
  {
    //Set current throttle value
    update_throttle_set(arm_state, throttle_rot);
  }

  //Reset hold counter
  button_hold = 0;
  button_toggle = true;
}

//////////////////////////////////////////////////////Throttle Value Update Functions
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
void update_throttle_set(bool arm_toggle, uint16_t new_value)
{
  //Update to new throttle value
  throttle_set = new_value;

  //Set new value on display
  displaySET.print(throttle_set);

  //Transmit current settings
  data_transmit = set_output(arm_toggle, throttle_set);
  transmit(data_transmit);

  //Print current throttle_set value
  Serial.print("~Throttle Set: ");
  Serial.println(throttle_set);
}

///////////////////////////////////////////////////////////////Transmission Functions
//Construct the outgoing data (in the form of a 16-bit integer)
uint16_t set_output(bool MS_bit, uint16_t trailing_int)
{
  uint16_t data;
  bitWrite(data, 0, MS_bit);
  bitWrite(data, 1, bitRead(trailing_int, 0));
  bitWrite(data, 2, bitRead(trailing_int, 1));
  bitWrite(data, 3, bitRead(trailing_int, 2));
  bitWrite(data, 4, bitRead(trailing_int, 3));
  bitWrite(data, 5, bitRead(trailing_int, 4));
  bitWrite(data, 6, bitRead(trailing_int, 5));
  bitWrite(data, 7, bitRead(trailing_int, 6));
  bitWrite(data, 8, bitRead(trailing_int, 7));
  bitWrite(data, 9, bitRead(trailing_int, 8));
  bitWrite(data, 10, bitRead(trailing_int, 9));
  bitWrite(data, 11, bitRead(trailing_int, 10));
  
  //Set checksum
  uint8_t checksum = generate_checksum(data);

  //Write checksum to datastring
  bitWrite(data, 12, bitRead(checksum, 0));
  bitWrite(data, 13, bitRead(checksum, 1));
  bitWrite(data, 14, bitRead(checksum, 2));
  bitWrite(data, 15, bitRead(checksum, 3));
  
  return(data);
}

//Generate checksum
uint8_t generate_checksum(uint8_t data)
{
  //The same data but increased by 27 to avoid zero size
  data = data + 27;

  //Remove leading zeros
  data = data << 4;
  data = data >> 4;
  
  return(data);
}

//Transmitting the outgoing data (in the form of 2x 8-bit integers)
void transmit(uint16_t data)
{
  uint8_t data_byte;

  //Send the payload data as individual 8-bit bytes
  for(uint8_t i=0; i<2; i++)
  {
    //Use 1-i to send bytes in reverse order
    data_byte = data >> 8*(1-i);
    comms.write(data_byte);
  }
}

void send_heartbeat()
{
  transmit(0);
}
