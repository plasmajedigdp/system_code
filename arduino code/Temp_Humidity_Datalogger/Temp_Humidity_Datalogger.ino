#include "SD.h"
#include"SPI.h"
#include <dht.h>
#define DHT11_PIN 7
const int CSpin = 10;
String dataString ="";  // holds the data to be written to the SD card
String fileName = "data.csv"; //the name of the file to be created
int fileNumber = 0; 
dht DHT;
File sensorData;

void setup() {
  Serial.begin(9600);
  Serial.print("Initializing SD card...");
  pinMode(CSpin, OUTPUT);
  // see if the card is present and can be initialized:
  if (!SD.begin(CSpin)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  createFile();
}

void loop(){
  // build the data string
  int chk = DHT.read11(DHT11_PIN);
  dataString = String(DHT.temperature) + "," + String(DHT.humidity);
  saveData(); // save to SD card
  delay(1000); // delay before next write to SD Card
}

void createFile(){
  while(SD.exists(fileName)){ //check if the new filename is valid
    fileNumber++;
    fileName = "data" + String(fileNumber) + ".csv";
  }
  sensorData = SD.open(fileName, FILE_WRITE); //open new file and add column headings
  if (sensorData){
    sensorData.println("Temperature,Humidity");
    sensorData.close(); // close the file
  }
  else{
    Serial.println("error creating file");
    }
}
void saveData(){
  if(SD.exists(fileName)){ // check the card is still there
    // now append new data file
    sensorData = SD.open(fileName, FILE_WRITE);
      if (sensorData){
      sensorData.println(dataString);
      sensorData.close(); // close the file
      }
  }
  else{
  Serial.println("Error writing to file !");
  }
}
