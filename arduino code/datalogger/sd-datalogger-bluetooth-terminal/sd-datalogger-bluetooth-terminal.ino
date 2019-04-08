#include <SoftwareSerial.h>

#define rxPin 2
#define txPin 3
SoftwareSerial mySerial(rxPin, txPin); // RX, TX

#include <DHT.h>
#include <SD.h>
#include<SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <DS3231.h>
#define DHTPIN 7
#define DHTTYPE DHT22

const int CSpin = 10;
String dataString ="";  // holds the data to be written to the SD card
String fileName = "data.csv"; //the name of the file to be created
String dataDate = "";
String dataTime = "";
int fileNumber = 0; 

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
DS3231  rtc(SDA, SCL);
DHT dht(DHTPIN, DHTTYPE);
File sensorData;

void setup() {
  mySerial.begin(9600);
  
  Serial.begin(9600);
  dht.begin();
  bmp.begin();
  rtc.begin();
  pinMode(CSpin, OUTPUT);
  SD.begin(CSpin);
  createFile();
}

void loop(){
  sensors_event_t event;
  bmp.getEvent(&event);
  dataString =  String(rtc.getDateStr()) + "," + rtc.getTimeStr() + "," + dht.readTemperature() + "," + dht.readHumidity() + "," + event.pressure; 

  bluetoothComms();
   
  saveData();
  delay(1000);
}

void createFile(){
  while(SD.exists(fileName)){
    fileNumber++;
    fileName = "data" + String(fileNumber) + ".csv";
  }
  sensorData = SD.open(fileName, FILE_WRITE);
  if (sensorData){
    sensorData.println("Date,Time,Temperature,Humidity,Pressure");
    sensorData.close();
  }
}

void bluetoothComms(){
    mySerial.println(dataString);
    delay(10);

    
  }

void saveData(){
  if(SD.exists(fileName)){
    sensorData = SD.open(fileName, FILE_WRITE);
      if (sensorData){
      sensorData.println(dataString);
      sensorData.close();
      }
  }
}
