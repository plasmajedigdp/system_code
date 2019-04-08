#include <SoftwareSerial.h>

SoftwareSerial comms(3, 2);

uint16_t data_transmit = 0;
uint16_t data_recieved = 0;

bool arm_state = 0; //One to arm
uint8_t throttle = 0; //0-100% throttle value

bool check = 0;

void setup()
{
  Serial.begin(115200);
  comms.begin(9600);
  
  pinMode(13, OUTPUT);

  digitalWrite(13, HIGH);
}

void loop()
{
  if(check == 0)
  {
    arm_state = 1;
    throttle = 100;
    
    data_transmit = set_output(arm_state, throttle);

    transmit(data_transmit);

    check = 1;
  }


  //Read comms input data
  while(comms.available())
  {
    data_recieved += char(comms.read());
  }

  //All incoming messages are greater than zero
  //True even if throttle and arm_state euqla zero -> checksum != 0
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

    data_recieved = 0;

    digitalWrite(13, LOW);

    //Set some values I guess
  }
}

//Setting the outgoing data (in the form of a 16-bit integer)
uint16_t set_output(bool MS_bit, uint8_t trailing_int)
{
  uint16_t data;
  bitWrite(data, 0, bitRead(trailing_int, 0));
  bitWrite(data, 1, bitRead(trailing_int, 1));
  bitWrite(data, 2, bitRead(trailing_int, 2));
  bitWrite(data, 3, bitRead(trailing_int, 3));
  bitWrite(data, 4, bitRead(trailing_int, 4));
  bitWrite(data, 5, bitRead(trailing_int, 5));
  bitWrite(data, 6, bitRead(trailing_int, 6));
  bitWrite(data, 7, MS_bit);
  
  //Set checksum
  uint8_t checksum = generate_checksum(data);
  
  bitWrite(data, 8, bitRead(checksum, 0));
  bitWrite(data, 9, bitRead(checksum, 1));
  bitWrite(data, 10, bitRead(checksum, 2));
  bitWrite(data, 11, bitRead(checksum, 3));
  bitWrite(data, 12, bitRead(checksum, 4));
  bitWrite(data, 13, bitRead(checksum, 5));
  bitWrite(data, 14, bitRead(checksum, 6));
  bitWrite(data, 15, bitRead(checksum, 7));

  Serial.println("Data Set:");
  Serial.print("arm_state =\t");
  Serial.println(read_arm_state(data));
  Serial.print("throttle =\t");
  Serial.println(read_throttle(data));
  Serial.print("checksum =\t");
  Serial.println(read_checksum(data));
  Serial.println();
  
  return(data);
}

//Generate checksum (the same data but increased by 27 to avoid a zero message)
uint8_t generate_checksum(uint16_t data)
{
  return(data + 27);
}

//Transmitting the outgoing data (in the form of 2x 8-bit integers)
void transmit(uint16_t data)
{
  uint8_t data_byte;

  //Send the payload size
  data_byte = sizeof(data);
  comms.write(data_byte);
  Serial.println(data_byte);

  //Send the payload data as individual 8-bit bytes
  for(uint8_t i=0; i<sizeof(data); i++)
  {
    data_byte = data >> 8*i;
    comms.write(data_byte);
    Serial.println(data_byte, BIN);
  }
  
  Serial.println("Data Sent.");
  Serial.println();
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
