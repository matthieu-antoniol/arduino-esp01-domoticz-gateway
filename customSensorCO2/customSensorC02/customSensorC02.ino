/*
   MQ135 gas sensor coupled with moisture sensor
   Author: Matthieu Antoniol
   Date of creation : 28-01-2022
*/

/*
   TO use in domoticzs please replace all @domoticzSensorIDX@ in the id of your sensor.
   Please use the boolean calibrationAirSensor@domoticzSensorIDX@ to get the value of R0.
   Then modify MQ135.h script to match the value.

*/

// The load resistance on the board
#define RLOAD 10.0 //Please match the value in the MQ135.h script equivalent to the resistance of the board. in KOhm
#include "MQ135.h" //Resulting from calibration.

#include <SoftwareSerial.h>
#include <ArduinoJson.h>

//ESP8266-01 parameters
#define RX 2 //Pin to plug to esp-01 TX
#define TX 3 //Pin to plug to esp-01 RX 

//Global sensor hub parameters
String espMessage = "";
bool messageReady = false;
String requestString = "";
int timeBetweenMeasures = 30000; //5 minutes = 30000ms
bool measureReadyToSend = false;

SoftwareSerial esp8266(RX, TX);


//Input Moisture AirSensor@domoticzSensorIDX@
int analogicPinAirSensor@domoticzSensorIDX@ = A0;
String typeAirSensor@domoticzSensorIDX@ = "air-quality";
String nameAirSensor@domoticzSensorIDX@ = "Air quality AirSensor@domoticzSensorIDX@";
String idxAirSensor@domoticzSensorIDX@ = "@domoticzSensorIDX@";
int valueAirSensor@domoticzSensorIDX@;
bool calibrationAirSensor@domoticzSensorIDX@ = false;

MQ135 gasAirSensor@domoticzSensorIDX@ = MQ135(analogicPinAirSensor@domoticzSensorIDX@);

void setup() {
  //Global initialization
  Serial.begin(9600); //Launch communication with computer
  esp8266.begin(112500); //Allow communication

  pinMode(analogicPinAirSensor@domoticzSensorIDX@, INPUT);
}

void loop() {
  DynamicJsonDocument requestToSend(1024);
  //Build a json request string
  requestString = "{\"type\":\"request\"}";
  DeserializationError error = deserializeJson(requestToSend, requestString);

  JsonObject sensorList = requestToSend.createNestedObject("sensors");
  measureReadyToSend = false;
  if (!calibrationAirSensor@domoticzSensorIDX@) {
    JsonObject AirQuality@domoticzSensorIDX@ = sensorList.createNestedObject("@domoticzSensorIDX@");
    float outputValueAirSensor@domoticzSensorIDX@ = gasAirSensor@domoticzSensorIDX@.getPPM();
    AirQuality@domoticzSensorIDX@["type"] = typeAirSensor@domoticzSensorIDX@;
    AirQuality@domoticzSensorIDX@["value"] = String(outputValueAirSensor@domoticzSensorIDX@, 1);

    measureReadyToSend = true;
  } else {
    float resistanceR0 = gasAirSensor@domoticzSensorIDX@.getRZero();
    Serial.print("Value of resistance R0: ");
    Serial.println(resistanceR0);
  }
  
  if (measureReadyToSend) {
    //Send document to ESP 8266
    serializeJson(requestToSend,esp8266);
  }
  delay(timeBetweenMeasures);
}
