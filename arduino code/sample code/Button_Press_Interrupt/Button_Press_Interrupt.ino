void setup()
{
  Serial.begin(115200);
  
  //Configure Pins
  DDRD |= B00000000;

  //Set Pin 2 To PULLUP
  PORTD |= B00000100;

  attachInterrupt(digitalPinToInterrupt(2), button_press, FALLING);
}

void loop()
{
  
}

void button_press()
{
  Serial.println("a");
}
