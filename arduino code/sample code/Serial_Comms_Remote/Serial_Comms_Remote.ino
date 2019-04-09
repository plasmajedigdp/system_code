#include <SoftwareSerial.h>

SoftwareSerial comms(6, 5);

uint16_t data_transmit = 0;
uint16_t data_recieved = 0;

uint8_t byte_size = 0;

uint8_t checksum = 0;

bool arm_state = 0; //One to arm
uint16_t throttle = 1000; //Throttle value (max 2047, 11-bits)

void setup()
{
  Serial.begin(115200);
  comms.begin(57600);

  data_transmit = set_output(arm_state, throttle);
  transmit(data_transmit);
}

void loop()
{
  
}

//Setting the outgoing data (in the form of a 16-bit integer)
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
    data_byte = data >> 8*(1-i);
    comms.write(data_byte);
  }
}
