#include <SoftwareSerial.h>

SoftwareSerial comms(3, 2);

uint16_t data_recieved = 0;

void setup()
{
  Serial.begin(115200);
  comms.begin(9600);
  
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

void loop()
{
  while(comms.available())
  {
    data_recieved += comms.read();
    digitalWrite(13, LOW);
  }//////////////////////////////////////////////////////need to decode new protocol -> payload size then payload data

  if(data_recieved > 0)
  {
    Serial.println("Recieved Data:");
    Serial.print("arm_state =\t");
    Serial.println(read_arm_state(data_recieved));
    Serial.print("throttle =\t");
    Serial.println(read_throttle(data_recieved));
    Serial.print("checksum =\t");
    Serial.println(read_checksum(data_recieved));
    Serial.println();
    
    comms.write(data_recieved);
    data_recieved = 0;
  }
}

//Reading the incoming data for the arm state
bool read_arm_state(uint16_t data)
{
  return(bitRead(data, 7));
}

//Reading the incoming data for the throttle value
uint8_t read_throttle(uint16_t data)
{
  uint8_t throttle_input;
  
  bitWrite(throttle_input, 0, bitRead(data, 0));
  bitWrite(throttle_input, 1, bitRead(data, 1));
  bitWrite(throttle_input, 2, bitRead(data, 2));
  bitWrite(throttle_input, 3, bitRead(data, 3));
  bitWrite(throttle_input, 4, bitRead(data, 4));
  bitWrite(throttle_input, 5, bitRead(data, 5));
  bitWrite(throttle_input, 6, bitRead(data, 6));

  return(throttle_input);
}

//Reading the incoming data for the checksum value
uint8_t read_checksum(uint16_t data)
{
  uint8_t checksum_input;

  bitWrite(checksum_input, 0, bitRead(data, 8));
  bitWrite(checksum_input, 1, bitRead(data, 9));
  bitWrite(checksum_input, 2, bitRead(data, 10));
  bitWrite(checksum_input, 3, bitRead(data, 11));
  bitWrite(checksum_input, 4, bitRead(data, 12));
  bitWrite(checksum_input, 5, bitRead(data, 13));
  bitWrite(checksum_input, 6, bitRead(data, 14));
  bitWrite(checksum_input, 7, bitRead(data, 15));

  return(checksum_input);
}
