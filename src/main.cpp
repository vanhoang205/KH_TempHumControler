#include "main.h"

void sendSensor(void);
void sendSetpoint(void);
void writeTempLcd(bool type, uint8_t value);
void writeHumLcd(bool type, uint8_t value);
int mdns1(void);
void launchWeb(void);
void setupAP(void);
int testWifi(void);

String urldecode(const char *src);
LiquidCrystal_I2C lcd(0x27, 16, 2);
PCF8574 pcf8574(0x20);
SHT3X sht30(0x44);
BlynkTimer timer;
Button buttonTempInc(INC_TEMP_BUTTON);
Button buttonTempDec(DEC_TEMP_BUTTON);
Button buttonHumInc(INC_HUM_BUTTON);
Button buttonHumDec(DEC_HUM_BUTTON);
bool currentLoad1, currentLoad2;
bool lastLoad1, lastLoad2;
int temp, hum;
int tempSetpoint, humSetpoint;
bool isUpdateTempSetpoint, isUpdateHumSetpoint;
char auth[] = "MUixCwOG4evgwHBERfITVy5aV0tQAmRS";

WiFiServer server(80);
const char *APssid = "nodemcu"; // Name of access point
const char *APpass = "12345678";       // Pass of access point
String rsid;
String rpass;
boolean newSSID = false;

BLYNK_CONNECTED()
{

  Serial.println(__FUNCTION__);
  Blynk.syncAll();
}

BLYNK_WRITE(V5)
{

  Serial.println(__FUNCTION__);
  temp = param.asInt();
  writeTempLcd(0, temp);
}

BLYNK_WRITE(V6)
{

  Serial.println(__FUNCTION__);
  hum = param.asInt();
  writeHumLcd(0, hum);
}

BLYNK_WRITE(V7)
{

  Serial.println(__FUNCTION__);
  tempSetpoint = param.asInt();
  writeTempLcd(1, tempSetpoint);
}

BLYNK_WRITE(V8)
{

  Serial.println(__FUNCTION__);
  humSetpoint = param.asInt();
  writeHumLcd(1, humSetpoint);
}

void sendSetpoint()
{

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

  Serial.println(__FUNCTION__);

  if (sht30.get() == 0) {
    temp = (uint8_t)sht30.cTemp;
    hum = (uint8_t)sht30.humidity;
  }
  else {
    temp = 0;
    hum = 0;
  }
  writeTempLcd(0, temp);
  writeHumLcd(0, hum);
  Serial.printf("temp: %d - hum: %d\n", temp, hum);

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

  Serial.begin(57600);
  delay(10);

  Serial.println("start main() function");

  Serial.println("Start Init Relay");
  pcf8574.pinMode(RELAY_TEMP, OUTPUT);
  pcf8574.pinMode(RELAY_HUM, OUTPUT);
  pcf8574.pinMode(RELAY_LOAD_1, OUTPUT);
  pcf8574.pinMode(RELAY_LOAD_2, OUTPUT);
  pcf8574.begin();
  Serial.println("Finish Init Relay");

  Serial.println("Start Init Button");
  buttonTempInc.begin();
  buttonTempDec.begin();
  buttonHumInc.begin();
  buttonHumDec.begin();
  Serial.println("Finish Init Button");

  Serial.println("Start Init lcd");
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Waiting ...");
  Serial.println("Finish Init lcd");

  Serial.println("Start Connect Blink");
  if (testWifi()) /*--- if the stored SSID and password connected successfully, exit setup ---*/
  {
    Blynk.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str());
  }
  else /*--- otherwise, set up an access point to input SSID and password  ---*/
  {
    Serial.println("");
    Serial.println("Connect timed out, opening AP");
    lcd.clear();
    lcd.home();
    lcd.print("Connecting");
    lcd.setCursor(0, 1);
    lcd.print("Your wifi ...");
    setupAP();
  }
  timer.setInterval(1000L, sendSensor);
  timer.setInterval(200, sendSetpoint);
  Serial.println("Finish Connect Blink");

  Serial.println("Finish main()");
  Serial.println("--------------------------------");
  Serial.println("--------------------------------");
  Serial.println("GO TO LOOP()");

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
  currentLoad1 = pcf8574.digitalRead(RELAY_LOAD_1);
  currentLoad2 = pcf8574.digitalRead(RELAY_LOAD_2);
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
}

void loop()
{

  if (buttonTempInc.pressed())
  {
    if (tempSetpoint < MAX_TEMP)
    {
      tempSetpoint++;
      isUpdateTempSetpoint = true;
      Serial.println("buttonTempInc Pressed");
    }
  }

  if (buttonTempDec.pressed())
  {
    if (tempSetpoint > MIN_TEMP)
    {
      tempSetpoint--;
      isUpdateTempSetpoint = true;
      Serial.println("buttonTempDec Pressed");
    }
  }

  if (buttonHumInc.pressed())
  {
    if (humSetpoint < MAX_HUM)
    {
      humSetpoint++;
      isUpdateHumSetpoint = true;
      Serial.println("buttonHumInc Pressed");
    }
  }

  if (buttonHumDec.pressed())
  {
    if (humSetpoint > MIN_HUM)
    {
      humSetpoint--;
      isUpdateHumSetpoint = true;
      Serial.println("buttonHumDec Pressed");
    }
  }

  if (temp > tempSetpoint)
  {
    pcf8574.digitalWrite(RELAY_TEMP, HIGH);
    lcd.setCursor(11, 0);
    lcd.printf("X");
  }
  else
  {
    pcf8574.digitalWrite(RELAY_TEMP, LOW);
    lcd.setCursor(11, 0);
    lcd.printf("O");
  }

  if (hum > humSetpoint)
  {
    pcf8574.digitalWrite(RELAY_HUM, HIGH);
    lcd.setCursor(12, 0);
    lcd.printf("X");
  }
  else
  {
    pcf8574.digitalWrite(RELAY_HUM, LOW);
    lcd.setCursor(12, 0);
    lcd.printf("O");
  }
  
  currentLoad1 = pcf8574.digitalRead(RELAY_LOAD_1);
  if (currentLoad1 != lastLoad1){
    lastLoad1 = currentLoad1;
    lcd.setCursor(13, 0);
    if (currentLoad1 == 1){
      lcd.print("X");
    }
    else {
      lcd.print("O");
    }
  } 


  currentLoad2 = pcf8574.digitalRead(RELAY_LOAD_2);
  if (currentLoad2 != lastLoad2){
    lastLoad2 = currentLoad2;
    lcd.setCursor(14, 0);
    if (currentLoad2 == 1){
      lcd.print("X");
    }
    else {
      lcd.print("O");
    }
  } 

  Blynk.run();
  timer.run();

  if (!Blynk.connected()) {
    lcd.setCursor(15, 0);
    lcd.print("D");
    lcd.setCursor(15, 1);
    lcd.print("D");
  }
  else {
    lcd.setCursor(15, 0);
    lcd.print(" ");
    lcd.setCursor(15, 1);
    lcd.print(" ");
  }
}

/*--- auto connect with stored wifi ---*/
int testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while (c < 20)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WiFi connected.");
      Serial.println(WiFi.localIP());
      return (1);
    }
    delay(500);
    Serial.print(WiFi.status());
    c++;
  }
  return (0);
} //end testwifi

void setupAP(void)
{
  WiFi.mode(WIFI_STA); //mode STA
  WiFi.disconnect();   //disconnect to scan wifi
  delay(100);

  Serial.println("");
  delay(100);
  WiFi.softAP(APssid, APpass); //change to AP mode with AP ssid and APpass
  Serial.println("softAP");
  Serial.println("");
  launchWeb(); //?
}

void launchWeb()
{
  Serial.println("");
  Serial.println(WiFi.softAPIP());
  server.begin(); // Start the server
  Serial.println("Server started");
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

      Serial.println("Connecting to local Wifi");
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
        Serial.println("");
        Serial.println("New SSID or Password failed. Reconnect to server, and try again.");
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
  Serial.println("");
  Serial.println("New client");

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
    Serial.print("Invalid request: ");
    Serial.println(req);
    return (20);
  }
  req = req.substring(addr_start + 1, addr_end);
  Serial.print("Request: ");
  Serial.println(req);
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
    Serial.println("Sending 200");
  }
  else if (req.startsWith("/a?ssid="))
  {
    newSSID = true;
    String qsid;                                                  //WiFi SSID
    qsid = urldecode(req.substring(8, req.indexOf('&')).c_str()); //correct coding for spaces as "+"
    Serial.println(qsid);
    rsid = qsid;

    String qpass;                                                                         //Wifi Password
    qpass = urldecode(req.substring(req.indexOf('&') + 6, req.lastIndexOf(' ')).c_str()); //correct for coding spaces as "+"
    Serial.println(qpass);
    rpass = qpass;

    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Temperature & Humidity Controller";
    s += "<p> New SSID and Password accepted</html>\r\n\r\n";
  }
  else
  {
    s = "HTTP/1.1 404 Not Found\r\n\r\n";
    Serial.println("Sending 404");
  }
  client.print(s);
  Serial.println("Done with client");
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
