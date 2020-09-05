#include "main.h"

LiquidCrystal_I2C lcd(0x38);
BlynkTimer timer;
DHT dht(DHTTYPE, DHTPIN);
Button buttonTempInc(INC_BUTTON);
Button buttonTempDec(DEC_BUTTON);
Button buttonHumInc(INC_BUTTON);
Button buttonHumDec(DEC_BUTTON);
Button buttonAct(ACT_BUTTON);

int temp, hum;
int tempSetpoint, humSetpoint;
bool isUpdateTempSetpoint, isUpdateHumSetpoint;
char auth[] = "MUixCwOG4evgwHBERfITVy5aV0tQAmRS";
char ssid[] = "Son Tung";
char pass[] = "sontung357";

BLYNK_CONNECTED() {

  Blynk.syncAll();
}

BLYNK_WRITE(V5) {

  temp = param.asInt();
  lcd.home();
  lcd.print(temp);
}

BLYNK_WRITE(V6) {

  hum = param.asInt();
  lcd.setCursor(0, 1);
  lcd.print(hum);
}
void sendSensor() {

  temp = (int)dht.readTemperature();
  hum = (int)dht.readHumidity();
  if (isnan(temp) || isnan(hum)) {
    temp = 0;
    hum = 0;
  }
  Blynk.virtualWrite(V5, temp);
  Blynk.virtualWrite(V6, hum);

  if (isUpdateTempSetpoint){
    Blynk.virtualWrite(V7, tempSetpoint);   // send value of setpoint's temperatue
    isUpdateTempSetpoint = false;
  }

  if (isUpdateHumSetpoint){
    Blynk.virtualWrite(V7, humSetpoint);   // send value of setpoint's temperatue
    isUpdateHumSetpoint = false;
  }
}

void setup() {

  buttonTempInc.begin();
  buttonTempDec.begin();
  buttonHumInc.begin();
  buttonHumDec.begin();
  buttonAct.begin();

  pinMode(RELAY_TEMP_1, OUTPUT);
  pinMode(RELAY_TEMP_2, OUTPUT);
  pinMode(RELAY_HUM_1, OUTPUT);
  pinMode(RELAY_HUM_2, OUTPUT);
  
  lcd.begin(16,2);
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  timer.setInterval(1000L, sendSensor);
  
}


void loop() {

  if (buttonTempInc.pressed()){
    if (tempSetpoint <= MAX_TEMP){
      tempSetpoint++;
      isUpdateTempSetpoint = true;
    }
  }

  if (buttonTempDec.pressed()) {
    if (tempSetpoint >= MIN_TEMP){
      tempSetpoint--;
      isUpdateTempSetpoint = true;
    }
  }

  if (buttonHumInc.pressed()){
    if (humSetpoint <= MAX_HUM){
      humSetpoint++;
      isUpdateHumSetpoint = true;
    }
  }

  if (buttonHumDec.pressed()){
    if (humSetpoint >= MIN_HUM){
      humSetpoint--;
      isUpdateHumSetpoint = true;
    }
  }
  
  Blynk.run();
  timer.run(); 
}