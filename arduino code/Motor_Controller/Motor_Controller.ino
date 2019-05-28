//Include libraries
#include <SoftwareSerial.h>
#include <Servo.h>

//Define Software Serial
SoftwareSerial comms(6, 5); //RX, TX

//Define Motor
Servo motor;

uint8_t ESC_PIN = 9; //Speed controller pin

//Communication variables
uint16_t data_recieved; //Variable to hold the data when it is read from comms
uint8_t byte_size = 0; //Used to measure the size of the incoming bytes
// - set at 0, equals 1 for 8 incoming bits, 2 for 16 incoming bits

uint16_t throttle_limit_up = 2000; //Upper Limit of the throttle
uint16_t throttle_limit_dn = 1000; //Lower Limit of the throttle
uint16_t throttle_value_new = 0; //Throttle value used to run the motor (zero until set)
uint16_t throttle_value_old = 0; //Used to check whether the throttle value has changed

//Variable to express whether or not the motor is armed
bool arm_state = false; //The arm state sent to the motor controller

//Heartbeat variables
uint16_t heartbeat_count = 0; //Counter to store how long since the last heartbeat
uint16_t heartbeat_max = 25000; //How long (in clock ticks) between each heartbeat check
//heartbeat_max will have to be varied to cope with more functions in the loop
// - More functions in the loop = fewer ticks needed to wait for the heartbeat as
//   heartbeat_count will be incremented slower

//##############################################################################Setup
void setup()
{
  setup_connections();
  setup_pins();
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
  motor.attach(ESC_PIN);
}

//###############################################################################Loop
void loop()
{
  ///////////////////////////////////////////////////////////////////Loop - Arm Check
  if(arm_state == false)
  {
    set_throttle(0); //Set ESC signal as unarmed
    throttle_value_old = 0; //Make sure the throttle is restored when rearmed
    Serial.println("CONTROLLER NOT ARMED");
  }
  
  //////////////////////////////////////////////////////////////////Loop - Comms Read
  //Check to see if there is incoming data
  if(comms.available())
  {
    comms_read();
  }

  /////////////////////////////////////////////////////////////Loop - Update Throttle
  if(throttle_value_old != throttle_value_new
  && arm_state == true)
  {
    set_throttle(throttle_value_new);
  }

  throttle_value_old = throttle_value_new;

  ///////////////////////////////////////////////////////////////////Loop - Heartbeat
  //Heartbeat Handler (boom boom)
  heartbeat();
}

/////////////////////////////////////////////////////////////////////Throttle Control
//Function that sets the throttle to the ESC Servo pin
void set_throttle(uint16_t throttle)
{
  motor.write(throttle);
  
  Serial.println();
  Serial.print("Throttle Set: ");
  Serial.println(throttle);
}

///////////////////////////////////////////////////////////////Transmission Functions
//Read incoming data
void comms_read()
{
  while(comms.available())
  {
    data_recieved = data_recieved << 8;
    data_recieved += comms.read();
    
    byte_size++;
  }

  if(byte_size >= 2)
  {
    uint8_t checksum = read_checksum(data_recieved);
    
    //Data Recieved
    if(checksum == generate_checksum(data_recieved))
    {
      throttle_value_new = read_throttle(data_recieved);
      arm_state = read_arm_state(data_recieved);

      Serial.println();
      Serial.println("DATA APPROVED");
      print_current_state(arm_state, throttle_value_new, checksum);
    }
    //Heartbeat Pulse
    else if(checksum == 0)
    {
      heartbeat_count = 0;
    }
    //Data Corrupted
    else
    {
      Serial.println();
      Serial.println("DATA REJECTED");
    }

    byte_size = 0;
  }
}

///////////////////////////////////////////////////////////////////Heartbeat Handlers
//Function called each loop to determine heartbeat condition
void heartbeat()
{
  if(heartbeat_count == heartbeat_max)
  {
    heartbeat_failure();
  }

  if(heartbeat_count < heartbeat_max)
  {
    heartbeat_count++;
  }
}

//When the heartbeat pulse isn't receieved for a time
void heartbeat_failure()
{
  arm_state = 0;
  throttle_value_new = 0;
  
  Serial.println("Heartbeat Missing");
}

/////////////////////////////////////////////////////////////////////////Reading Data
//Reading the incoming data for the arm state
bool read_arm_state(uint16_t data)
{
  return(bitRead(data, 0));
}

//Reading the incoming data for the throttle value
uint16_t read_throttle(uint16_t data)
{
  uint16_t throttle_input;
  
  bitWrite(throttle_input, 0, bitRead(data, 1));
  bitWrite(throttle_input, 1, bitRead(data, 2));
  bitWrite(throttle_input, 2, bitRead(data, 3));
  bitWrite(throttle_input, 3, bitRead(data, 4));
  bitWrite(throttle_input, 4, bitRead(data, 5));
  bitWrite(throttle_input, 5, bitRead(data, 6));
  bitWrite(throttle_input, 6, bitRead(data, 7));
  bitWrite(throttle_input, 7, bitRead(data, 8));
  bitWrite(throttle_input, 8, bitRead(data, 9));
  bitWrite(throttle_input, 9, bitRead(data, 10));
  bitWrite(throttle_input, 10, bitRead(data, 11));

  return(throttle_input);
}

//Reading the incoming data for the checksum value
uint8_t read_checksum(uint16_t data)
{
  uint8_t checksum_input;

  bitWrite(checksum_input, 0, bitRead(data, 12));
  bitWrite(checksum_input, 1, bitRead(data, 13));
  bitWrite(checksum_input, 2, bitRead(data, 14));
  bitWrite(checksum_input, 3, bitRead(data, 15));

  return(checksum_input);
}

////////////////////////////////////////////////////////////////////Generate Checksum
uint8_t generate_checksum(uint8_t data)
{
  //The same data but increased by 27 to avoid zero size
  data = data + 27;

  //Remove leading zeros
  data = data << 4;
  data = data >> 4;
  
  return(data);
}

/////////////////////////////////////////////////////////////////////////Print Values
void print_current_state(uint8_t arm, uint16_t throttle, uint8_t check)
{
  Serial.print("arm_state =\t");
  Serial.println(arm);
  Serial.print("throttle =\t");
  Serial.println(throttle);
  Serial.print("checksum =\t");
  Serial.println(check);
}
