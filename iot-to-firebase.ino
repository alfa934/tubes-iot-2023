// TO-DO: parse data from Tx esp32
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include <LoRa.h>
#include <SPI.h>
 
#define ss 5
#define rst 14
#define dio0 2

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "*******************"
#define WIFI_PASSWORD "*******************"

// Insert Firebase project API Key
// alfa
//#define API_KEY "*******************"
// evander
#define API_KEY "*******************"

// Insert RTDB URLefine the RTDB URL */
// alfa
//#define DATABASE_URL "*******************"
// evander
#define DATABASE_URL "*******************" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long recvLoraPrevMillis = 0;
bool signupOK = false;

/* LIST OF DATA */
float  initLAT  = 0.0;
float  initLNG  = 0.0;
float  currLAT  = 0.0;
float  currLNG  = 0.0;
float  currSPD  = 0.0;
float  distance = 0.0;
String currSTAT = "false";

void setup()
{
    Serial.begin(115200);
//    InitLora();
    InitFirebase();
}

void loop() {
  // From Tx ESP32
//  LoraOperation();
  // To Firebase  
  FirebaseOperation();
}

void InitLora()
{   ///////// LORA SETUP /////////
    LoRa.setPins(ss, rst, dio0);
    
    //433E6 - Asia, 866E6 - Europe, 915E6 - North America
    while (!LoRa.begin(433E6))     
    {
        Serial.println(".");
        delay(500);
    }
    
    Serial.println("LoRa Initializing OK!");
}

void InitFirebase()
{   ///////// WIFI AND FIREBASE SETUP ///////////
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    
    while (WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    
    /* Assign the api key (required) */
    config.api_key = API_KEY;
    
    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;
    
    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("ok");
        signupOK = true;
    }
    else{
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }
    
    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void LoraOperation()
{
  String inString = "";    // string to hold input
  if(millis() - recvLoraPrevMillis > 1000 || recvLoraPrevMillis == 0) 
  {
    recvLoraPrevMillis = millis();
    
    int packetSize = LoRa.parsePacket();
    
    if (packetSize) { 
        // read packet    
        while (LoRa.available()) {
          int inChar = LoRa.read();
          inString += (char)inChar;
          currLAT = inString.toInt();       
        }
        inString = "";     
        LoRa.packetRssi();    
    }
    
  }
}

void FirebaseOperation()
{
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    String path = "data/";
    // send constant data
    SendFloatData(initLAT, path+"initLAT");
    SendFloatData(initLNG, path+"initLNG");
    
    // send updateable data
    SendFloatData(currLAT, path+"currLAT");
    SendFloatData(currLNG, path+"currLNG");
    SendFloatData(currSPD, path+"currSPD");
    SendFloatData(distance, path+"distance");

    // then calculate the status of the vehicle
    if(distance > 10.0 || currSPD > 10.0) {
    // if distance or speed is above tolerance, then danger danger xD
        currSTAT = "true";
    } else {
        currSTAT = "false";
    }
    SendStringData(currSTAT, path+"status");

    initLAT+=0.1;
    initLNG+=0.2;
    currLAT+=0.3;
    currLNG+=0.4;
    currSPD+=0.5;
    distance+=0.6;
  }
}

void SendFloatData(float inputData, String nodeName)
{   // Write an Float number on the database path nodeName
    if (Firebase.RTDB.setFloat(&fbdo, nodeName, inputData)) 
    {
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
    }
}

void SendIntData(int inputData, String nodeName)
{   // Write an Int number on the database path nodeName
    if (Firebase.RTDB.setInt(&fbdo, nodeName, inputData))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
}

void SendStringData(String inputData, String nodeName)
{   // Write an String on the database path nodeName
    if (Firebase.RTDB.setString(&fbdo, nodeName, inputData))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
}
