# Presentation

## Description
Public libraries and resources to make homemade sensors.

For the moment only a fully functionnal JSON API allows you to send data from an arduino to domoticz through an ESP8266-01.



## Prerequisite for ESP8266-01
You need to be able to program the ESP8266-01 and the arduino independently. For this I bought a key and customized it to make sending a code to ESP8266-01 Easy.
On the arduino part, all the usual, not much to say a simple cable and the arduino IDE and you are good to go.

## Disclaimer

The ESP8266-01 doesn't work as a slave in this he will receive its code before being plugged in to the arduino.
In my case I used a simple serial communication to send information from arduino to ESP8266-01.
In the end the ESP8266-01 is just an API that received information on its RX pins and send it to domoticz through wifi.

Another point of interest is that, the fake RX and fake TX you choose to define on arduino should be linked respectively to TX and RX on ESP8266-01
So that they can communicate on this channel.

# How does it work

- You have to collect and measure data from your arduino as you see fits. But the code to send it to the ESP8266-01 should be the same as in the examples.
- The Gateway code has been made to handle several measures so you can have more than one sensor measures sent in one go.

- Be sure that you have uploaded the gateway code to your ESP8266-01.
If you didn't find your sensor please adapt and improve the gateway to handle your own data.
You can find the API documentation on https://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s

- Plug the ESP8266-01 as in the example that follow. I didn't use any resistance in the build for now and it work for me.
(in the example I put a moisture sensor). 
![image](https://user-images.githubusercontent.com/8017433/155862520-491d2c1b-82d9-4f3d-a41a-23d3d3b83253.png)

- Upload the code of a sensor that can send a JSON on its serial (inspire yourself from the examples).

- If you respect the documentation you shall see the data in domoticz.


