#include "main.h"

void sendSensor(void);
void sendStatus(void);
void writeTempLcd(bool type, uint8_t value);
void writeHumLcd(bool type, uint8_t value);
int mdns1(void);
void launchWeb(void);
void setupAP(void);
int testWifi(void);
String urldecode(const char *src);

Ticker secondTick;
volatile int watchdogCount = 0;
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;
WidgetLED tempLoad(V11);
WidgetLED humLoad(V12);
DHT dht(DHTPIN, DHTTYPE);
Button buttonTempInc(INC_TEMP_BUTTON);
Button buttonTempDec(DEC_TEMP_BUTTON);
Button buttonHumInc(INC_HUM_BUTTON);
Button buttonHumDec(DEC_HUM_BUTTON);
bool currentLoad1, currentLoad2;
bool lastLoad1, lastLoad2;
int temp, hum;
int tempSetpoint, humSetpoint;
bool isUpdateTempSetpoint, isUpdateHumSetpoint;
bool currentLoadTemp, currentLoadHum;
bool lastLoadTemp, lastLoadHum;
char auth[] = "iL3UlMTRHQxl5IwAVbr3Bj-DD1Pu6qHD";

WiFiServer server(80);
const char *APssid = "nodemcu"; // Name of access point
const char *APpass = "12345678";       // Pass of access point
String rsid;
String rpass;
boolean newSSID = false;

void ISRWatchdog() {
  watchdogCount++;
  DPRINTF("Watchdog counter: %d\n", watchdogCount);
  if (watchdogCount == 5) {
    DPRINTLN();
    DPRINTLN("the watch dog bites !!!!!");
    ESP.reset();
  }
}

BLYNK_CONNECTED()
{

  DPRINTLN(__FUNCTION__);
  Blynk.syncAll();
}

BLYNK_WRITE(V5)
{

  DPRINTLN(__FUNCTION__);
  temp = param.asInt();
  writeTempLcd(0, temp);
}

BLYNK_WRITE(V6)
{

  DPRINTLN(__FUNCTION__);
  hum = param.asInt();
  writeHumLcd(0, hum);
}

BLYNK_WRITE(V7)
{

  DPRINTLN(__FUNCTION__);
  tempSetpoint = param.asInt();
  writeTempLcd(1, tempSetpoint);
}

BLYNK_WRITE(V8)
{

  DPRINTLN(__FUNCTION__);
  humSetpoint = param.asInt();
  writeHumLcd(1, humSetpoint);
}

BLYNK_WRITE(V11) {

  DPRINTLN(__FUNCTION__);
  currentLoadTemp = (bool)param.asInt();
  lcd.setCursor(11,0);
  if (currentLoadTemp){
    lcd.print("X");
  }
  else {
    lcd.print("O");
  }
  lastLoadTemp = currentLoadTemp;
}

BLYNK_WRITE(V12) {

  DPRINTLN(__FUNCTION__);
  currentLoadHum = (bool)param.asInt();
  lcd.setCursor(12,0);
  if (currentLoadHum){
    lcd.print("X");
  }
  else {
    lcd.print("O");
  }
  lastLoadHum = currentLoadHum;
}

void sendStatus()
{

  if (currentLoadTemp != lastLoadTemp) {
    DPRINTLN("Temp Load change value");
    if (currentLoadTemp) {
      digitalWrite(RELAY_TEMP, HIGH);
      lcd.setCursor(11, 0);
      lcd.printf("X");
      tempLoad.on();
    }
    else {
      digitalWrite(RELAY_TEMP, LOW);
      lcd.setCursor(11, 0);
      lcd.printf("O");
      tempLoad.off();
    }
    lastLoadTemp = currentLoadTemp;
  }

  if (currentLoadHum != lastLoadHum) {
    DPRINTLN("Hum Load change value");
    if (currentLoadHum) {
      digitalWrite(RELAY_HUM, HIGH);
      lcd.setCursor(12, 0);
      lcd.printf("X");
      humLoad.on();
    }
    else {
      digitalWrite(RELAY_HUM, LOW);
      lcd.setCursor(12, 0);
      lcd.printf("O");
      humLoad.off();
    }
    lastLoadHum = currentLoadHum;
  }

  if (isUpdateTempSetpoint)
  {
    writeTempLcd(1, tempSetpoint);
    Blynk.virtualWrite(V7, tempSetpoint); // send value of setpoint's temperatue
    isUpdateTempSetpoint = false;
  }

  if (isUpdateHumSetpoint)
  {
    writeHumLcd(1, humSetpoint);
    Blynk.virtualWrite(V8, humSetpoint); // send value of setpoint's temperatue
    isUpdateHumSetpoint = false;
  }
}

void sendSensor()
{

  DPRINTLN(__FUNCTION__);

  temp = (uint8_t)dht.readTemperature();
  hum = (uint8_t)dht.readHumidity();
  if (isnan(temp) || isnan(hum))
  {
    temp = 0;
    hum = 0;
  }
  writeTempLcd(0, temp);
  writeHumLcd(0, hum);
  DPRINTF("temp: %d - hum: %d\n", temp, hum);

  Blynk.virtualWrite(V5, temp);
  Blynk.virtualWrite(V6, hum);
}

void writeTempLcd(bool type, uint8_t value)
{

  if (type)
  {
    lcd.setCursor(4, 1);
  }
  else
  {
    lcd.setCursor(4, 0);
  }
  if ((value / 10) == 0)
  {
    lcd.print("0");
  }
  lcd.printf("%d", value);
}

void writeHumLcd(bool type, uint8_t value)
{

  if (type)
  {
    lcd.setCursor(7, 1);
  }
  else
  {
    lcd.setCursor(7, 0);
  }
  if ((value / 100) == 0)
  {
    lcd.print("0");
    if ((value / 10) == 0)
    {
      lcd.print("0");
    }
  }
  lcd.printf("%d", value);
}

void setup()
{

#ifdef DEBUG
  Serial.begin(57600);
  delay(10);
#endif

  DPRINTLN("start main() function");

  DPRINTLN("Start Init Relay");
  // pinMode(RELAY_TEMP_1, OUTPUT);
  analogWrite(A0, 255);
  pinMode(RELAY_TEMP, OUTPUT);
  pinMode(RELAY_HUM, OUTPUT);
  pinMode(RELAY_LOAD_1, OUTPUT);
  pinMode(RELAY_LOAD_2, OUTPUT);
  DPRINTLN("Finish Init Relay");

  DPRINTLN("Start Init Button");
  buttonTempInc.begin();
  buttonTempDec.begin();
  buttonHumInc.begin();
  buttonHumDec.begin();
  DPRINTLN("Finish Init Button");

  DPRINTLN("Start Init lcd");
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Waiting ...");
  DPRINTLN("Finish Init lcd");

  DPRINTLN("Start Init DHT22");
  dht.begin();
  DPRINTLN("Finish Init DHT22");


  DPRINTLN("Start Connect Blink");
  if (testWifi()) /*--- if the stored SSID and password connected successfully, exit setup ---*/
  {
    Blynk.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str(), IPAddress(192,168,1,50), 8080);
  }
  else /*--- otherwise, set up an access point to input SSID and password  ---*/
  {
    DPRINTLN("");
    DPRINTLN("Connect timed out, opening AP");
    lcd.clear();
    lcd.home();
    lcd.print("Connect nodemcu");
    lcd.setCursor(0, 1);
    lcd.print("192.168.4.1");
    setupAP();
  }
  timer.setInterval(1000L, sendSensor);
  timer.setInterval(200L, sendStatus);
  DPRINTLN("Finish Connect Blink");

  DPRINTLN("Finish main()");
  DPRINTLN("--------------------------------");
  DPRINTLN("--------------------------------");
  DPRINTLN("GO TO LOOP()");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printstr("RL:T");
  lcd.setCursor(6, 0);
  lcd.print("H");
  lcd.setCursor(0, 1);
  lcd.printstr("SP:T");
  lcd.setCursor(6, 1);
  lcd.print("H");
  lcd.setCursor(10, 0);
  lcd.print("-");
  lcd.setCursor(10, 1);
  lcd.print("L1234");
  currentLoad1 = digitalRead(RELAY_LOAD_1);
  currentLoad2 = digitalRead(RELAY_LOAD_2);
  currentLoad1 = lastLoad1;
  currentLoad2 = lastLoad2;

  if (currentLoad1)
  {
    lcd.setCursor(13, 0);
    lcd.print("X");
  }
  else
  {
    lcd.setCursor(13, 0);
    lcd.print("O");
  }

  if (currentLoad2)
  {
    lcd.setCursor(14, 0);
    lcd.print("X");
  }
  else
  {
    lcd.setCursor(14, 0);
    lcd.print("O");
  }

  secondTick.attach(1, ISRWatchdog);
}

void loop()
{
  if (buttonTempInc.pressed())
  {
    if (tempSetpoint < MAX_TEMP)
    {
      tempSetpoint++;
      isUpdateTempSetpoint = true;
      DPRINTLN("buttonTempInc Pressed");
    }
  }

  if (buttonTempDec.pressed())
  {
    if (tempSetpoint > MIN_TEMP)
    {
      tempSetpoint--;
      isUpdateTempSetpoint = true;
      DPRINTLN("buttonTempDec Pressed");
    }
  }

  if (buttonHumInc.pressed())
  {
    if (humSetpoint < MAX_HUM)
    {
      humSetpoint++;
      isUpdateHumSetpoint = true;
      DPRINTLN("buttonHumInc Pressed");
    }
  }

  if (buttonHumDec.pressed())
  {
    if (humSetpoint > MIN_HUM)
    {
      humSetpoint--;
      isUpdateHumSetpoint = true;
      DPRINTLN("buttonHumDec Pressed");
    }
  }

  if (temp > tempSetpoint)
  {
    currentLoadTemp = true;
  }
  else
  {
    currentLoadTemp = false;
  }

  if (hum > humSetpoint)
  {
    currentLoadHum = true;
  }
  else
  {
    currentLoadHum = false;
  }
  
  currentLoad1 = digitalRead(RELAY_LOAD_1);
  if (currentLoad1 != lastLoad1){
    lastLoad1 = currentLoad1;
    lcd.setCursor(13, 0);
    if (currentLoad1 == 1){
      lcd.print("O");
    }
    else {
      lcd.print("X");
    }
  } 


  currentLoad2 = digitalRead(RELAY_LOAD_2);
  if (currentLoad2 != lastLoad2){
    lastLoad2 = currentLoad2;
    lcd.setCursor(14, 0);
    if (currentLoad2 == 1){
      lcd.print("O");
    }
    else {
      lcd.print("X");
    }
  } 

  Blynk.run();
  timer.run();

  lcd.setCursor(15, 0);
  if (!Blynk.connected()) {
    lcd.print("D");
  }
  else {
    lcd.setCursor(15, 0);
    lcd.print(" ");
  }

  lcd.setCursor(15, 1);
  if (WiFi.status() != WL_CONNECTED) {
    lcd.print("D");
  }
  else {
    lcd.print(" ");
  }

  watchdogCount = 0;
}

/*--- auto connect with stored wifi ---*/
int testWifi(void)
{
  int c = 0;
  DPRINTLN("Waiting for Wifi to connect");
  while (c < 20)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      DPRINTLN("WiFi connected.");
      DPRINTLN(WiFi.localIP());
      return (1);
    }
    delay(500);
    DPRINT(WiFi.status());
    c++;
  }
  return (0);
} //end testwifi

void setupAP(void)
{
  WiFi.mode(WIFI_STA); //mode STA
  WiFi.disconnect();   //disconnect to scan wifi
  delay(100);

  DPRINTLN("");
  delay(100);
  WiFi.softAP(APssid, APpass); //change to AP mode with AP ssid and APpass
  DPRINTLN("softAP");
  DPRINTLN("");
  launchWeb(); //?
}

void launchWeb()
{
  DPRINTLN("");
  DPRINTLN(WiFi.softAPIP());
  server.begin(); // Start the server
  DPRINTLN("Server started");
  int b = 20;
  int c = 0;
  while (b == 20)
  {
    b = mdns1(); //mdns1(function: web interface, read local IP SSID, Pass)
    /*--- If a new SSID and Password were sent, close the AP, and connect to local WIFI ---*/
    if (newSSID == true)
    {
      newSSID = false;
      /*--- convert SSID and Password string to char ---*/
      char ssid[rsid.length()];
      rsid.toCharArray(ssid, rsid.length());
      char pass[rpass.length()];
      rpass.toCharArray(pass, rpass.length());

      DPRINTLN("Connecting to local Wifi");
      delay(500);
      WiFi.softAPdisconnect(true); //disconnet APmode
      delay(500);
      WiFi.mode(WIFI_STA);    //change to STA mode
      WiFi.begin(ssid,pass);                            //connect to local wifi
      delay(1000);
      ///set -> check -> set
      if (testWifi()) //test connect
      {
        Blynk.begin(auth, ssid, pass);
        return;
      }
      else //if wrong ssid or pass
      {
        DPRINTLN("");
        DPRINTLN("New SSID or Password failed. Reconnect to server, and try again.");
        setupAP();
        return;
      }
    }
  }
}

int mdns1()
{
  // Check for any mDNS queries and send responses
  // Check if a client has connected                    //server mode
  WiFiClient client = server.available(); //check client
  if (!client)
  {
    return (20);
  }
  DPRINTLN("");
  DPRINTLN("New client");

  // Wait for data from client to become available
  while (client.connected() && !client.available())
  {
    delay(1);
  }

  // Read the first line of HTTP request
  String req = client.readStringUntil('\r');

  // First line of HTTP request looks like "GET /path HTTP/1.1"
  // Retrieve the "/path" part by finding the spaces
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1)
  {
    DPRINT("Invalid request: ");
    DPRINTLN(req);
    return (20);
  }
  req = req.substring(addr_start + 1, addr_end);
  DPRINT("Request: ");
  DPRINTLN(req);
  client.flush();
  String s;
  if (req == "/")
  {
    IPAddress ip = WiFi.softAPIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>NodeMCU - Designed by VanHoang<br/>";
    s += "IP: ";
    s += ipStr;
    s += "<p>";
    //s += st;
    s += "<form method='get' action='a'>";
    s += "<table><tr><td>SSID:</td><td><input name='ssid' length=32></td> </tr><tr><td>PASS:</td><td><input name='pass' length=64></td> </tr></table>";
    s += "<input type='submit' style='left: 160px; position: relative;'></form>";
    s += "</html>\r\n\r\n";
    DPRINTLN("Sending 200");
  }
  else if (req.startsWith("/a?ssid="))
  {
    newSSID = true;
    String qsid;                                                  //WiFi SSID
    qsid = urldecode(req.substring(8, req.indexOf('&')).c_str()); //correct coding for spaces as "+"
    DPRINTLN(qsid);
    rsid = qsid;

    String qpass;                                                                         //Wifi Password
    qpass = urldecode(req.substring(req.indexOf('&') + 6, req.lastIndexOf(' ')).c_str()); //correct for coding spaces as "+"
    DPRINTLN(qpass);
    rpass = qpass;

    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Temperature & Humidity Controller";
    s += "<p> New SSID and Password accepted</html>\r\n\r\n";
  }
  else
  {
    s = "HTTP/1.1 404 Not Found\r\n\r\n";
    DPRINTLN("Sending 404");
  }
  client.print(s);
  DPRINTLN("Done with client");
  return (20);
}

String urldecode(const char *src)
{
  String decoded = "";
  char a, b;
  while (*src)
  {
    if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b)))
    {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      decoded += char(16 * a + b);
      src += 3;
    }
    else if (*src == '+')
    {
      decoded += ' ';
      *src++;
    }
    else
    {
      decoded += *src;
      *src++;
    }
  }
  decoded += '\0';
  return decoded;
}
