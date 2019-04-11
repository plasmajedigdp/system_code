#include <SoftwareSerial.h>

SoftwareSerial comms(6, 5);

uint16_t data_recieved;

uint8_t byte_size = 0;

uint8_t checksum = 0;

void setup()
{
  Serial.begin(115200);
  comms.begin(57600);
  
  pinMode(13, OUTPUT);
}

void loop()
{
  //Read comms input data
  while(comms.available())
  {
    data_recieved = data_recieved << 8;
    data_recieved += comms.read();
    
    byte_size++;
  }

  if(byte_size >= 2)
  {
    //Data Recieved
    if(read_checksum(data_recieved) == generate_checksum(data_recieved))
    {
      Serial.println("DATA APPROVED");
      print_current_state();
      Serial.println();
    }
    //Data Corrupted
    else
    {
      Serial.println("DATA REJECTED");
      Serial.println();
    }

    byte_size = 0;
  }
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

void print_current_state()
{
  Serial.print("arm_state =\t");
  Serial.println(read_arm_state(data_recieved));
  Serial.print("throttle =\t");
  Serial.println(read_throttle(data_recieved));
  Serial.print("checksum =\t");
  Serial.println(read_checksum(data_recieved));
}
