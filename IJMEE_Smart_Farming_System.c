/*************************************************************
 * FOLLOWING MESSAGE IS FROM BLYNK DOCUMENTATION
  WARNING!
    It's very tricky to get it working. Please read this article:
    http://help.blynk.cc/hardware-and-libraries/arduino/esp8266-with-at-firmware

  You’ll need:
   - Blynk IoT app (download from App Store or Google Play)
   - Arduino Uno board
   - Decide how to connect to Blynk
     (USB, Ethernet, Wi-Fi, Bluetooth, ...)

  There is a bunch of great example sketches included to show you how to get
  started. Think of them as LEGO bricks  and combine them as you wish.
  For example, take the Ethernet Shield sketch and combine it with the
  Servo example, or choose a USB sketch and add a code from SendData
  example.
 *************************************************************/


//Himali Mistry ES327 
#define BLYNK_TEMPLATE_ID "TMPLhYEibNDp"                          //Provided by Blynk.Cloud - can be found in profile section under device info tab 
#define BLYNK_DEVICE_NAME "ES327 Blynk v1"                        //Blynk Project name 
#define BLYNK_AUTH_TOKEN "Tfec-z9EIWplslDIJ92o8carq6zsfLp8"       //Provided by Blynk.Cloud - can be found in welcome email 
#define BLYNK_PRINT Serial                                        //Prints Blynk connection graphic in Arduino IDE


#include <ESP8266_Lib.h>                                          //Libraries required for ESP8266-01
#include <BlynkSimpleShieldEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;                     

//Wi-fi Credentials 
char ssid[] = "CourtStreetStudentInternet";                       
char pass[] = "T2MBOCOURT";                                       //Set password to "" for open networks.



#include <SoftwareSerial.h>                                       //Library needed for software serial connection to Arduino Uno 
SoftwareSerial EspSerial(2, 3);                                   // RX, TX Hardware Connections 


#define ESP8266_BAUD 38400                                        // Defined ESP8266 baud rate - specific to Wi-Fi settings
ESP8266 wifi(&EspSerial);


// Declaring a global variable for TMP36 sensor data
int Temperature_Pin = A0;                                         //TMP36 connected to A0
float Temperature_Value;
float A0_Vout; 
#define fan_pwm 8                                                 //Axial fan connected to D8

// Declaring a global variable for LM393 sensor data
int   Soil_Moisture_Pin = A1;                                     //LM393 connected to A1
float Soil_Moisture_Value ;
float A1_Vout;
#define pump_pwm 9                                               //Water pump connected to D9 


BlynkTimer timer;                                               //Creates the 'timer object' part of Blynk library (can't use 'delay' anymore)

void pump_on() {
   Blynk.virtualWrite(V4, 1);                                   //Writes to V4 datastream in app that pump is on 
   digitalWrite(pump_pwm, LOW);
   timer.setInterval(500L, Moisture_Control)                   //Pump will turn on for 500L = 500ms = 0.5seconds 
   Blynk.virtualWrite(V4, 0);                                  //Writes to V4 datastream in app that pump is off
 }

void Temperature_Control() 
{
  A0_Vout = analogRead(A0);                                   //read the analog sensor and store it
  Temperature_Value = A0_Vout / 1024;                         //input gives us a value between 0 and 1023 so find voltage per single unit
  Temperature_Value = Temperature_Value * 5;                  //multiply by 5V to get voltage (finding the percentage of 5V)
  Temperature_Value = Temperature_Value - 0.5;                //subtract the offset as we want  0° – 175° range not standard setting -50° to 125°
  Temperature_Value = Temperature_Value * 100;                //multiply by 100 to convert to degrees
  Serial.print(A0_Vout);
  Serial.print("Current Temperature: ");
  Serial.println(Temperature_Value);
  Blynk.virtualWrite(V1, Temperature_Value);                  //Writes to V1 datastream in app temperature sensor value

      if (Temperature_Value < 21)                             
    {
      analogWrite(fan_pwm,0);
      Blynk.virtualWrite(V3, '0');                            //Writes V3 datastream in app fan speed
    }
    else if (Temperature_Value>23)
    {
      analogWrite (fan_pwm, 153)                              //fan on to 60%
      Blynk.virtualWrite(V3, '60');                           //Writes V3 datastream in app fan speed
    }
    else if(Temperature_Value>26)
    {
      analogWrite(fan_pwm,204);                              //80% fan on to 80%
      Blynk.virtualWrite(V3, '80');                          //Writes V3 datastream in app fan speed
    } 
}

void Moisture_Control() {
  Soil_Moisture_Pin = analogRead(A1);                       //read the analog sensor and store it
  Soil_Moisture_Value = map(Soil_Moisture_Pin,813,454,0,100);// maps voltage reading to a percentage based on dry/wet soil calibration respectively
  Blynk.virtualWrite(V2, Soil_Moisture_Value);              //Writes to 'virtual' datastream pin in app 
  Serial.print("Moisture : ");
  Serial.print(Soil_Moisture_Value);
  Serial.println("%");
    if (Soil_Moisture_Value<25){
      timer.setInterval(1000L, pump_on);
       digitalWrite(pump_pwm,HIGH);                         //turns pump on            
     } else {
       digitalWrite(pump_pwm, LOW);                         //turns pump off 
     }
}

void setup()
{
  Serial.begin(115200);                                   //Debug console
  EspSerial.begin(ESP8266_BAUD);                          // Set ESP8266 baud rate
  Blynk.begin(auth, wifi, ssid, pass, "blynk.cloud", 80);
  pinMode(fan_pwm, OUTPUT);                               //resets axial fan actuator 
  timer.setInterval(900000L, Temperature_Control);        //temperature reading/corrective action is taken every 15mins
  pinMode(pump_pwm,OUTPUT);                               //resets water pump actuator 
  timer.setInterval(10800000L, Moisture_Control);         //soil moisture reading/corrective action is taken every 3hrs

}


void loop()
{
  Blynk.run();                                          //Continously runs Blynk connection 
  timer.run();                                          //Continously runs Blynk timer 
}
