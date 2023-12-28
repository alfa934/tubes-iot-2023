// TO-DO: 
// - test GPS algorithm    
#include <Wire.h>
#include <TinyGPS++.h>
#include <LoRa.h>
#include <SPI.h>

/* COMMENT if not needed */
#define DEBUGGING_MODE

// LORA
#define ss 5
#define rst 14
#define dio0 2
// GPS
#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);
TinyGPSPlus gps;

// list of data
float  initLAT  = 0.0;
float  initLNG  = 0.0;
float  currLAT  = 0.0;
float  currLNG  = 0.0;
float  currSPD  = 0.0; // km/h
float  distance = 0.0; // meters

// time
unsigned long long prevGps = 0;
unsigned long long prevLora = 0;

void setup() 
{
  Serial.begin(115200); 
  
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);

  InitLora();
  
  GetInitialPosition();

}
 
void loop() 
{
   boolean newData = false;
   if (millis() >= prevGps + 500)
   {
       prevGps += 500;
       while (neogps.available())
       {
           if (gps.encode(neogps.read()))
           {
               newData = true;
           }
       }
   }
   
   if (gps.location.isValid() && newData) {
       #ifdef DEBUGGING_MODE
       Serial.print("Detected Satellites:");
       Serial.println(gps.satellites.value());
       Serial.print("Lat: ");
       Serial.println(gps.location.lat(), 4);   
       Serial.print("Lng: ");
       Serial.println(gps.location.lng(),4);
       Serial.print("Speed(km/h): ");
       Serial.println(gps.speed.kmph());
       #endif

       currLAT = gps.location.lat();
       currLNG = gps.location.lng();
       currSPD = gps.speed.kmph();
       // distance in m
       distance = (float)TinyGPSPlus::distanceBetween(currLAT, 
                                                      currLNG, 
                                                      initLAT, 
                                                      initLNG);
       
   } 
   else {
       #ifdef DEBUGGING_MODE
       Serial.println("not good");
       #endif
     
       newData = false;
   }


    if(millis() >= prevLora + 500) 
    {
        // LORA TRANSMISSION
        prevLora += 500;
        String TransmitString = String(initLAT) + ";" + String(initLNG) + ";" +
                                String(currLAT) + ";" + String(currLNG) + ";" +
                                String(currSPD) + ";" + String(distance);
                                
        #ifdef DEBUGGING_MODE
        Serial.println(TransmitString);
        #endif
        
        LoRa.beginPacket();  
        LoRa.print(TransmitString);
        LoRa.endPacket();
        
        // initLAT+=0.1;
        // initLNG+=0.2;
        // currLAT+=0.3;
        // currLNG+=0.4;
        // currSPD+=0.5;
        // distance+=0.6;
    }
}

void InitLora()
{
    LoRa.setPins(ss, rst, dio0);    
    //433E6 - Asia, 866E6 - Europe, 915E6 - North America
    while (!LoRa.begin(433E6))     
    {
        Serial.println(".");
        delay(500);
    }
    Serial.println("LoRa Initializing OK!");
}

void GetInitialPosition()
{   
    boolean newData = false;
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
        while (neogps.available())
        {
            if (gps.encode(neogps.read()))
            {
                newData = true;
            }
        }
    }
    
    if (gps.location.isValid() && newData) {
        
        #ifdef DEBUGGING_MODE
        Serial.print("Detected Satellites:");
        Serial.println(gps.satellites.value());
        Serial.print("Lat: ");
        Serial.println(gps.location.lat(), 4);   
        Serial.print("Lng: ");
        Serial.println(gps.location.lng(),4);
        Serial.print("Speed(km/h): ");
        Serial.println(gps.speed.kmph());
        #endif
        
        initLAT = gps.location.lat();
        initLNG = gps.location.lng();
    
    } 
    else {
        #ifdef DEBUGGING_MODE
        Serial.println("not good");
        #endif
        delay(1000);
    }
}
