uint8_t CLK = 12; //CLK Pin On Encoder
uint8_t DT = 11; //DT Pin On Encoder

int RotPosition = 0;
int rotation;

int current_clk;

void setup()
{
  Serial.begin (115200);
  
  pinMode (CLK,INPUT);
  pinMode (DT,INPUT);
  
  rotation = digitalRead(CLK);
}

void loop()
{
  current_clk = digitalRead(CLK);
  
  if(current_clk != rotation) //We use the DT pin to find out which way we're turning
  {
    if(digitalRead(DT) != value) //Clockwise
    {  
      RotPosition += 5;
    }
    else //Anticlockwise
    {
      RotPosition -=5;
    }
    
    Serial.println(RotPosition);
  }
  
  rotation = current_clk;
}
