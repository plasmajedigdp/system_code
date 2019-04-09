#include "SevenSegmentTM1637.h"
#include <SoftwareSerial.h>

SevenSegmentTM1637 displaySET(10, 9); //Setup SET display
SevenSegmentTM1637 displayROT(8, 7); //Setup ROT display

SoftwareSerial comms(6, 5); //Setup software serial

uint8_t CLK_PIN = 12; //Encoder CLK pin
uint8_t DT_PIN = 11; //Encoder DT pin
uint8_t SW_PIN = 2; //Encoder Switch pin - must be on an interrupt pin

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

//Transmission variables
uint16_t data_transmit = 0;
uint16_t data_recieved = 0;

//Value to check the size of the incoming byte
uint8_t byte_size = 0;

//Variable to express whether or not the motor is armed
bool arm_state = 0; //One to arm

////////////////////////////////////////////////////////////////////////////////Setup
void setup()
{
  //Setup serial connection
  Serial.begin(115200);

  //Setup software serial connection
  comms.begin(57600);

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

  //Set arm state as one to begin ESC arming procedure
  arm_state = 1;
  
  //Set initial display value
  update_throttle_set(arm_state, throttle_limit_dn);
  update_throttle_rot(throttle_limit_dn);
}

//////////////////////////////////////////////////////////////Loop - Encoder Rotation
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
    
    //Set olc_clk value for next loop
    old_clk = new_clk;
  }

  ///////////////////////////////////////////////////////////////////Loop - Read Data
  //Read comms input data
  while(comms.available())
  {
    
  }
}

///////////////////////////////////////////////////////////////////Interrupt Handling
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
  bitWrite(data, 0, bitRead(trailing_int, 0));
  bitWrite(data, 1, bitRead(trailing_int, 1));
  bitWrite(data, 2, bitRead(trailing_int, 2));
  bitWrite(data, 3, bitRead(trailing_int, 3));
  bitWrite(data, 4, bitRead(trailing_int, 4));
  bitWrite(data, 5, bitRead(trailing_int, 5));
  bitWrite(data, 6, bitRead(trailing_int, 6));
  bitWrite(data, 7, bitRead(trailing_int, 7));
  bitWrite(data, 8, bitRead(trailing_int, 8));
  bitWrite(data, 9, bitRead(trailing_int, 9));
  bitWrite(data, 10, bitRead(trailing_int, 10));
  bitWrite(data, 11, MS_bit);
  
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
