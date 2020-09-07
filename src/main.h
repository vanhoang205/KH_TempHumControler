#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "LiquidCrystal_I2C.h"
#include <Wire.h>
#include "Button.h"

#define DHTPIN D3     
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define RELAY_TEMP A0
#define RELAY_HUM D6 
#define RELAY_LOAD_1 D7
#define RELAY_LOAD_2 D8

#define INC_TEMP_BUTTON D0
#define DEC_TEMP_BUTTON D4
#define INC_HUM_BUTTON D5
#define DEC_HUM_BUTTON 10

#define MAX_TEMP 60
#define MIN_TEMP 0
#define MAX_HUM 100
#define MIN_HUM 0

#endif