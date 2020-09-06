#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include "DHT.h"
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "Button.h"

#define DHTPIN 2     
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define RELAY_TEMP_1 13
#define RELAY_TEMP_2 10
#define RELAY_HUM_1 9
#define RELAY_HUM_2 14

#define INC_BUTTON 1
#define DEC_BUTTON 2
#define ACT_BUTTON 3

#define MAX_TEMP 60
#define MIN_TEMP 0
#define MAX_HUM 100
#define MIN_HUM 0

#endif