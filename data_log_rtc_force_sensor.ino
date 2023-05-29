#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <SPI.h>
#include "RTClib.h"

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD HEX address 0x27
RTC_DS1307 rtc;

int Force_VAL = 0;
int temp = 0;
int btn = 0;
int BookLED = 2;
int ForceSensor = A0;

File dataFile;
const int chipSelect = 10;

void setup() {
  Serial.begin(9600);

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
    
  pinMode(ForceSensor, INPUT);
  pinMode(4, INPUT);
  pinMode(BookLED, OUTPUT);

  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.print("Force Sensor");

  pinMode(chipSelect, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }

  dataFile = SD.open("data.csv", FILE_WRITE);

  if (dataFile) {
    dataFile.println("Serial, TimeStamp, LPG gas value");
    dataFile.close();
  } else {
    Serial.println("Error opening data.csv");
  }
}

void loop() {
  DateTime time = rtc.now();
  
  Force_VAL = analogRead(ForceSensor);
  btn = digitalRead(4);
  Serial.print("The Button is: ");
  if (btn == 0) {
    Serial.println("OFF");
  } else if (btn == 1) {
    Serial.println("ON");
  }

  lcd.setCursor(0, 1);
  lcd.print("Value:");
  lcd.setCursor(7, 1);
  lcd.print(Force_VAL);

  if (Force_VAL < 10)  //Enter Force Value Range
  {
    temp = 1;
  } else {
    temp = 0;
  }

  if (btn == 1) {
    temp = 0;
  }

  if (temp == 1) {
    digitalWrite(BookLED, HIGH);  //to be changed with GSM
    delay(1000);
  }

  if (temp == 0) {
    digitalWrite(BookLED, LOW);  //to be changed with GSM
  }

//Full Timestamp
  Serial.println(String("DateTime::TIMESTAMP_FULL:\t")+time.timestamp(DateTime::TIMESTAMP_FULL));

  String dataString = String(millis()) + "," + String("DateTime::TIMESTAMP_FULL:\t")+time.timestamp(DateTime::TIMESTAMP_FULL) + "," + String(Force_VAL);

  dataFile = SD.open("data.csv", FILE_WRITE);

  if (dataFile) {
    dataFile.println(dataString);
    Serial.println(dataString);
    dataFile.close();
  } else {
    Serial.println("Error opening data.csv");
  }
  
  delay(200);
}
