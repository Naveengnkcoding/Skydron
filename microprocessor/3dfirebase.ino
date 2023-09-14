#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "NK"
#define WIFI_PASSWORD "navkar11"

 #ifdef __cplusplus
  extern "C" {
 #endif

  uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

uint8_t temprature_sens_read();

static const int servosPins[4] = {12, 14, 5, 26};

Servo myservo[4];
// Insert Firebase project API Key
#define API_KEY "AIzaSyAiaPFkuinJ25wa8gN7kU_aFgM9tdxdeGc"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://rcf22-785e0-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
int intValue = 0;
int intL = 0;
int intR = 0;  
int intP = 0;
bool signupOK = false;
bool ready = false;

void setup(){
  Serial.begin(115200);

  myservo[1].attach(servosPins[1]);
  myservo[2].attach(servosPins[2]);
  myservo[3].attach(servosPins[3]);  // attaches the servo on pin 13 to the servo object
  myservo[4].attach(servosPins[4]);

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
  if (Firebase.signUp(&config, &auth, "", "")){
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

  if(Firebase.ready()){
    Firebase.RTDB.setString(&fbdo, "duo/f", "Setting Up...");
  }
}

void loop(){
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    
    // Write an Int number on the database path test/int
    if(ready){
    if (Firebase.RTDB.setString(&fbdo, "duo/f", "Ready to Fight")){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      ready = false;
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    }
    
    //////Gettting Int Value L
    if (Firebase.RTDB.getInt(&fbdo, "/duo/l")) {
      if (fbdo.dataType() == "int") {
        intL = fbdo.intData();
        myservo[1].write(intL);
        Serial.println(intL);
      }
    }
    else {  
      Serial.println(fbdo.errorReason());
    }

    ////////Gettig Int Value R
    if (Firebase.RTDB.getInt(&fbdo, "/duo/r")) {
      if (fbdo.dataType() == "int") {
        intR = fbdo.intData();
        Serial.println(intR);
        myservo[2].write(intL);
      }
    }
    else {  
      Serial.println(fbdo.errorReason());
    }

    
    ////////Gettig Int Value P
    if (Firebase.RTDB.getInt(&fbdo, "/duo/p")) {
      if (fbdo.dataType() == "int") {
        intP = fbdo.intData();
        if(intP > 40){
          ready = true;
        }
        myservo[3].write(intP);
        Serial.println(intP);
      }
    }
    else {  
      Serial.println(fbdo.errorReason());
    }
    //////Updating temperature
     intValue = (temprature_sens_read() - 32) / 1.8;
    Serial.print((temprature_sens_read() - 32) / 1.8);
     Serial.println(" C");
    if (Firebase.RTDB.setInt(&fbdo, "duo/a",intValue)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}
