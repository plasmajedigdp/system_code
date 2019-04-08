#include <Servo.h>

Servo m;

String input = "";
uint16_t current_input;
uint16_t new_input = 0;

void setup()
{
  Serial.begin(115200);
  m.attach(9);
}

void loop()
{
  //Read data into input string
  while(Serial.available())
  {
    new_1000
    1input += char(Serial.read());
  }

  //Flush input string
  if(new_input != input.toInt() && input.length() > 0 && input.toInt() > 0)
  {
    new_input = input.toInt();
    input = "";
  }

  //Send new input value to motor
  if(current_input != new_input)
  {
    current_input = new_input;
    m.write(current_input);
  }

  delay(5);
  Serial.println(current_input);
}
