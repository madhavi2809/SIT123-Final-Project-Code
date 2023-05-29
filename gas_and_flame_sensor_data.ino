#include <SD.h>
#include <SPI.h>
#include "RTClib.h"

RTC_DS1307 rtc;

int LED = 7;
int GasBuzzer = 9;
int FlameBuzzer = 7;
int sensorPin = A0;
int sensorData;
bool ChimneyStatus = 0;
bool FanStatus = 0;
const long manInterval = 2592000000; // 30 days in milliseconds

File dataFile;
const int chipSelect = 10;

void setup()
{
  Serial.begin(9600);// initialize serial communication @ 9600 baud:

  #ifndef ESP8266
  while(!Serial);
  #endif

  if(! rtc.begin()){
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  pinMode(2, INPUT); //initialize Flame sensor output pin connected pin as input.
  pinMode(LED, OUTPUT); // initialize digital pin of LED as an output.
  pinMode(GasBuzzer, OUTPUT); // initialize digital pin of buzzer integrated with Gas sensor as an output.
  pinMode(FlameBuzzer, OUTPUT); // initialize digital pin of buzzer integrated with Flame sensor as an output.
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }

  dataFile = SD.open("File.csv", FILE_WRITE);

  if (dataFile) {
    dataFile.println("Serial, TimeStamp, Gas Sensor Value, Chimney Status, Exhaust Fan Status");
    dataFile.close();
  } else {
    Serial.println("Error opening File.csv");
  }
}

void loop()
{  
  DateTime time = rtc.now();
  
  if (digitalRead(2) == HIGH)
  {
    digitalWrite(LED, HIGH); // Led ON
    Serial.println("CHIMNEY ON");
    ChimneyStatus = 1;
    digitalWrite(6, HIGH);
    digitalWrite(5, LOW);
    delay(1000);
  }
  else
  {
    ChimneyStatus = 0;
    digitalWrite(6, LOW);
    digitalWrite(5, LOW);
    digitalWrite(LED, LOW); // Led OFF
  }
  
  for (int i = 0; i < manInterval; i++)
  {
    if(i == manInterval)
    {
      Serial.println("30 DAYS OVER! CHIMNEY NEEDS MAINTENANCE!");
      digitalWrite(FlameBuzzer, HIGH);
      delay(10000);
    }
    else
    {
      digitialWrite(FlameBuzzer, LOW);
    }
  }
  
  sensorData = analogRead(sensorPin);
  Serial.println(sensorData);
  delay(1000);
  
  if (sensorData >= 260)
  {
    FanStatus = 1;
    Serial.println("LPG detected");
    digitalWrite(3, HIGH);
    digitalWrite(4, LOW);
    digitalWrite(GasBuzzer, HIGH);
  }
  else
  {
    FanStatus = 0;
    Serial.println("No LPG detected");
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(GasBuzzer, LOW);
  }

  //Full Timestamp
  Serial.println(String("DateTime::TIMESTAMP_FULL:\t")+time.timestamp(DateTime::TIMESTAMP_FULL));

  String dataString = String(millis()) + "," + String("DateTime::TIMESTAMP_FULL:\t")+time.timestamp(DateTime::TIMESTAMP_FULL) + "," + String(sensorData) + "," + String(ChimneyStatus) + "," + String(FanStatus);

  dataFile = SD.open("File.csv", FILE_WRITE);

  if (dataFile) {
    dataFile.println(dataString);
    Serial.println(dataString);
    dataFile.close();
  } else {
    Serial.println("Error opening File.csv");
  }
  
  delay(1000);
}
