#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "LiquidCrystal_I2C.h"
#include <Wire.h>
#include "Button.h"
#include "PCF8574.h"
#include "ESP8266_SHT3X.h"

#define RELAY_TEMP          P2
#define RELAY_HUM           P4 
#define RELAY_LOAD_1        P1
#define RELAY_LOAD_2        P3

#define INC_TEMP_BUTTON     12
#define DEC_TEMP_BUTTON     13
#define INC_HUM_BUTTON      14
#define DEC_HUM_BUTTON      16

#define MAX_TEMP 60
#define MIN_TEMP 0
#define MAX_HUM 100
#define MIN_HUM 0

#endif