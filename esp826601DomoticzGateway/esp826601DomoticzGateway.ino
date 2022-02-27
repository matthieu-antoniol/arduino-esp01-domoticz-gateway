#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

//Developper inputs
String ssid = "EnterHereYourWifiSSID";
String password = "EnterHereYourWifiPassword";
const String domoticz_host = "192.168.1.1"; //Enter here your domoticz host
const uint16_t domoticz_port = 8080; //Enter here your domoticz port (secure port if you use_secure_protocole)
const bool use_secured_protocole = false;
int max_request_size_allowed = 1024; //Limit in bytes of the size of the input json sent on serial

//General variables
WiFiClient wifiClient;


bool tx_message_received = false;
String request_message = "";
String json_first_level_key;
String sensor_idx;
String sensor_type;
String sensor_key;
String sensor_measure;
//Version, date and compatible sensor for this script. Be sure to modify those value if you make this script evolve
String esp8266_01_domoticz_gateway_version = "1.0";
String compatible_sensors = "humidity/temperature/temperature-humidity/moisture/air-quality";
String date_of_implementation = "2022-01-28";
//Domoticz compatibility information
String domoticz_version_for_api = "2021-01";
String domoticz_git_hash = "8457c5b7e";

void setup()
{
  WiFi.begin(ssid,password);
  Serial.begin(115200);
  
  delay(100);
  Serial.println("\r\n\r\n");
  Serial.println("ESP8266-01 Domoticz Gateway v"+esp8266_01_domoticz_gateway_version+" implemented on "+ date_of_implementation );
  Serial.println("This is an open source version of a domoticz compatible gateway for ESP8266-01.");
  Serial.println("With this program you can send information about the following domoticz sensors:");
  Serial.println(compatible_sensors + "\r");
  Serial.println("Be sure to verify if your domoticz version API is coded the same way as Domoticz version");
  Serial.println("Domoticz: version `" + domoticz_version_for_api + "` Hash: `" + domoticz_git_hash+"`");
  Serial.println("If you need any information don't hesitate to contact me at: matthieu.antoniol.code@gmail.com");
  Serial.println("Sincerely your, Matthieu Antoniol");
  Serial.println("\r\n");
  Serial.println("Initializing Wifi connection");
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\r\n");
  Serial.println("Congratulation you are connected to : " + ssid);
  Serial.print("Gateway IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("You can now send your request through the Gateway");
}

void sendDomoticzMeasureRequest(String sensor_idx, String measure, String type, bool use_secured_protocole=false){
    //Create http client and preformed the request
    HTTPClient httpClient;
    String request_string = (use_secured_protocole) ? "https://" : "http://";
    request_string += domoticz_host + ":" + domoticz_port + "/json.htm?";
    //Value used for humidity, temperature and DHT
    String n_value="";
    String s_value="";

    if(type=="humidity"){
        /* type: "humidity"
        *  example measure : "45;2"
        *  Humidity:int in percentage;humidity status: 0=Normal(30-45)  1=Comfortable(45-50) 2=Dry(<30) 3=Wet(>50)
        */
        int first_delimiter_index=measure.indexOf(';'); 
        String humidity=measure.substring(0, first_delimiter_index);
        String humidity_status=measure.substring(first_delimiter_index);
        n_value="&nvalue="+measure;
        s_value="&svalue="+humidity_status;
    }else if(type=="temperature"||type== "temperature-humidity"){
        /* type: "temperature"
        *  example measure : "25.2"
        *  float representing the temperature in celsius degree
        */
        /* type: "temperature-humidity"
        *  example measure : "25.2;52.0;1"
        *  Temperature float in celsius; humidity float representing a percentage;humidity status: 0=Normal(30-45)  1=Comfortable(45-50) 2=Dry(<30) 3=Wet(>50)
        */
        n_value="&nvalue=0";
        s_value="&svalue="+measure;
    }else if(type=="moisture"||type=="air-quality"){
        /* type: "moisture"
        *  example measure : "18"
        *  00 - 09 = saturated, 10 - 19 = adequately wet, 20 - 59 = irrigation advice, 60 - 99 = irrigation, 100-200 = Dangerously dry, 
        */
        /* type: "air-quality"
        *  example measure : "850"
        *  CO2-concentration in PPM  
        */
        n_value="&nvalue="+measure;
    }else {
        // type not recognized, exiting the whole function
        return;
    }
    request_string+="type=command&param=udevice&idx="+sensor_idx+n_value+s_value;
    
    //@debug Serial.println("[ESP]"); Serial.print(request_string);
    httpClient.begin(wifiClient,request_string);
    httpClient.addHeader("Content-Type", "text/plain");
    int httpClientCode= httpClient.POST("Message");
    String payload = httpClient.getString();
    httpClient.end();
}

void decodeRequestAndSendSensorsValue(DynamicJsonDocument request_json){
    JsonObject json_request_object;
    json_request_object = request_json.as<JsonObject>();
    for (JsonPair iterator : json_request_object) {
        json_first_level_key = iterator.key().c_str();

        if(json_first_level_key == "sensors"){
            JsonObject sensor_json_object = iterator.value().as<JsonObject>();
            //@debug Serial.println("  \r\n ----------- \r\n  ");
            for (JsonPair sensor_iterator : sensor_json_object) {
                sensor_type = "";
                sensor_measure = "";
                sensor_idx = sensor_iterator.key().c_str(); // is a JsonString.
                //@debug Serial.print("sensor ");Serial.println(sensor_idx); Serial.println("\r\n");
                JsonObject sensor = sensor_iterator.value().as<JsonObject>();
                for (JsonPair sensor_keys_iterator : sensor) {
                    sensor_key = sensor_keys_iterator.key().c_str(); // is a JsonString.
                    if(sensor_key=="type"){
                        sensor_type = sensor_keys_iterator.value().as<String>();
                    }else if(sensor_key=="value"){
                        sensor_measure = sensor_keys_iterator.value().as<String>();
                    }else {
                        // Do nothing because the key is not recognized
                    }
                    //@debug Serial.println();
                }
                
                if(sensor_idx !="" && sensor_measure !=""&& sensor_type != ""){
                    sendDomoticzMeasureRequest(sensor_idx, sensor_measure, sensor_type, use_secured_protocole);
                    //@debug Serial.println("Value to send to domoticz: id " + sensor_idx + "|type: " + sensor_type + "|value: " + sensor_measure);
                    //@debug Serial.println("\r\n");
                }
            }
        }
    }
}
void loop()
{   
    /*   here find the type of json format that can be received by this ESP8266-01 domoticz emmiter:
        {
            "type":"request",
            "sensors": {
                "134":{
                    "type":"air-quality",
                    "value":500
                },
                "125":{
                    "type":"moisture",
                    "value":70
                }
            }
        }
        Please make sure that the request is stripped of any useless spaces and return carriage.
        {"type":"request","sensors":{"134":{"type":"air-quality","value":500},"125":{"type":"moisture","value":70}}}
    
    */
    
  DynamicJsonDocument json_request(max_request_size_allowed);
  DynamicJsonDocument sensor(max_request_size_allowed);
  // Reading the response
  while(tx_message_received == false) { // blocking but that's ok
    if(Serial.available()) {
      request_message = Serial.readString();
      tx_message_received = true;
      //This uses TX to send back debug messages
      //@debug Serial.println("[ESP]Message received : "); Serial.print(request_message);
    }
  }
  // Attempt to deserialize the JSON-formatted message
  DeserializationError deserialization_error = deserializeJson(json_request,request_message);
  if(!deserialization_error) {
    decodeRequestAndSendSensorsValue(json_request);
  }  else {
    return;
  }
  tx_message_received = false;
  request_message="";
}
