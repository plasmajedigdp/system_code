#include "SevenSegmentTM1637.h"

SevenSegmentTM1637 displaytop(9, 8);
SevenSegmentTM1637 displaybot(7, 6);

void setup()
{
  displaytop.begin();
  displaybot.begin();
  
  displaytop.setBacklight(100);
  displaybot.setBacklight(100);
  
  displaytop.print("8888");
  displaybot.print("8888");
  
  delay(1000);
}

void loop()
{
  displaytop.print(1000);
  displaybot.print(1000);
  
  delay(1000);
  
  displaytop.blink(100, 1);
  
  delay(50000);
}
