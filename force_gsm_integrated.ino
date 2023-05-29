#include <LiquidCrystal_I2C.h>  //library for LCD

//libraries for SD card and data logging with time stamps
#include <SD.h>  
#include <SPI.h>
#include "RTClib.h"

//library for gsm module
#include "SoftwareSerial.h"

//Initializing LCD with its address
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD HEX address 0x27

//initializing RTC for TimeStamp
RTC_DS1307 rtc;

//Initializing GSM Module
SoftwareSerial mySerial(2, 3);
String cmd = "";

//Initializing variables for Force Sensor
int Force_VAL = 0;
int temp = 0;
int btn = 0;
int BookLED = 13;
int ForceSensor = A0;

//Initializing variables for data logging
File dataFile;
const int chipSelect = 10;

void setup() {
  Serial.begin(9600);  //beginning printing on Serial Monitor
  mySerial.begin(9600);  //initilizing gsm module's operation

  Serial.println("Initializing GSM SIM800L...");
  delay(1000);

  mySerial.println("AT");                 // Sends an ATTENTION command, reply is OK
  updateSerial();
  mySerial.println("AT+CMGF=1");          // Configuration for sending SMS
  updateSerial();
  mySerial.println("AT+CNMI=1,2,0,0,0");  // Configuration for receiving SMS
  updateSerial();

  //initial code for setting of TimeStamp
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

  //initializing pins for connections
  pinMode(ForceSensor, INPUT);
  pinMode(4, INPUT);
  pinMode(BookLED, OUTPUT);

  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.print("Force Sensor");

  pinMode(chipSelect, OUTPUT);

  //initializing SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }

  //create a file: data.csv
  dataFile = SD.open("data.csv", FILE_WRITE);

  if (dataFile) {
    dataFile.println("Serial, TimeStamp, LPG gas value");  //printing headers of rows in CSV file
    dataFile.close();
  } else {
    Serial.println("Error opening data.csv");
  }
}

void loop() {
  DateTime time = rtc.now();  //setting time to current time of PC

  //Code for Force Sensor
  Force_VAL = analogRead(ForceSensor);
  btn = digitalRead(4);

  //State of button is printed:
  Serial.print("The Button is: ");
  if (btn == 0) {
    Serial.println("OFF");
  } else if (btn == 1) {
    Serial.println("ON");
  }

  //Value of Force Sensor is printed on LED, updating after 0.2 sec
  lcd.setCursor(0, 1);
  lcd.print("Value:");
  lcd.setCursor(7, 1);
  lcd.print(Force_VAL);

//threshold value is set to 10, if it goes below 10, control goes to conditional statement of temp == 1 else it goes to temp == 0
  if (Force_VAL < 10)  //Enter Force Value Range
  {
    temp = 1;
  } else {
    temp = 0;
  }

  if (btn == 1) {
    temp = 0;
  }

//if temp is 1, the indication LED glows and notification is sent to the LPG agent (through updateSerial() function)
  if (temp == 1) {
    updateSerial();
    digitalWrite(BookLED, HIGH);
  }
  else if (temp == 0) {
    digitalWrite(BookLED, LOW);
  }

//Full Timestamp
  Serial.println(String("DateTime::TIMESTAMP_FULL:\t")+time.timestamp(DateTime::TIMESTAMP_FULL));

//Values that are to be updated and printed in the csv file
  String dataString = String(millis()) + "," + String("DateTime::TIMESTAMP_FULL:\t")+time.timestamp(DateTime::TIMESTAMP_FULL) + "," + String(Force_VAL);

//data.csv is opened
  dataFile = SD.open("data.csv", FILE_WRITE);

//if the file gets opened, then data is stored in it, else an error message is displayed
  if (dataFile) {
    dataFile.println(dataString);
    Serial.println(dataString);
    dataFile.close();
  } else {
    Serial.println("Error opening data.csv");
  }
  
  delay(200);
}

//This function updates the Serial Monitor with current situation
void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {

    cmd+=(char)Serial.read();
 
    if(cmd!=""){
      cmd.trim();  // Remove added LF in transmit
      if (cmd.equals("S")) {
        sendSMS();
      } else {
        mySerial.print(cmd);
        mySerial.println("");
      }
    }
  }
  
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

//This function sends an SMS to the LPG agent whose contact number is provided below.
void sendSMS()
{
  mySerial.println("AT+CMGF=1");
  delay(500);
  mySerial.println("AT+CMGS=+919041166867\r");
  delay(500);
  mySerial.print("Hey, this is a request from House no. 44, Brookbell Estate, Australia to book an LPG gas cylinder for the registered mobile number: 6735803690 and send it to the same address as soon as possible.");
  delay(500);
  mySerial.write(26);
}
