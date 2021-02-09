

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "DSM501.h"
#include <Ticker.h>
#include <EEPROM.h>

#define DSM501_PM2_5 D5 // pin 4 - Vout1 - over 2.5 um
#define DSM501_PM1_0 D6 // pin 2 - Vout2 - over 1 um
#define PIN_BATTERY_VOLTAGE A0

#define SAMPLE_TIME_DSM510 30
#define SAMPLE_TIME_BME280 2
#define SAMPLE_TIME_VOLTAGE 1

const char *ssid = "skunks"; // network name
const char *hostname = "skunks"; // dns domain name
IPAddress apIP(192, 168, 4, 1); // ip address

DNSServer dnsServer;
Adafruit_BME280 bme;
DSM501 dsm501;
ESP8266WebServer server(80);
Ticker timer_voltage;
Ticker timer_bme280;

int pm25 = 0;
float battery_voltage = 0;
float temperature = 0;
float humidity = 0;
float pressure = 0;

void setup() {
  Serial.begin(115200);
  cls();
  Serial.println("[app] Starting");
  setup_eeprom(512);
  setup_voltage();
  setup_bme280();
  setup_dsm501a();
  setup_ap();
  setup_dns();
  setup_web();
  Serial.println("[app] Skunks ready");
}

void write_string_to_eeprom(int pos, String str) {
  for (int i = 0; i < str.length(); ++i)
    {
      EEPROM.write(i + pos, str[i]);
    }
  EEPROM.write(str.length(), 0xFF);
  EEPROM.commit();
  Serial.println(String("[eeprom] Wrote ") + str.length() + " bytes from pos: " + pos);
}

String read_string_from_eeprom(int pos) {
  String str;
  int pos_act = pos;
  uint8_t next_char = 0;
  while (true) {
    next_char = EEPROM.read(pos_act);
    if (next_char != 0xFF){
    str += char(next_char);
    pos_act++;
  }
    else {
      Serial.println(String("[eeprom] Read ") + str.length() + " bytes from pos: " + pos + ": " + str);
  return str;
}
  }
}

void loop() {
  server.handleClient();
  dnsServer.processNextRequest();
  read_dsm501a();
}

void cls() {
  Serial.write(27);
  Serial.print("[2J");
  Serial.write(27);
  Serial.print("[H");
}

void setup_eeprom(int size) {
  Serial.println("[eeprom] Initializing ...");
  EEPROM.begin(size);
  delay(50); // without a delay eeprom r/w may result in an error
  Serial.println(String("[eeprom] Initialized size: ") + size);
}

void setup_bme280() {
  Serial.println("[bme280] Setting up...");
  bool status;
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("[bme280] ERROR: device not found!");
    while (1); // stopping the program
  }
  Serial.println("[bme280] Setting repeated measures");
  timer_bme280.attach(SAMPLE_TIME_BME280, read_bme280);
  Serial.println("[bme280] Ready");
}

void setup_dsm501a() {
  Serial.println("[dsm501a] Setting up...");
  dsm501.begin(DSM501_PM1_0, DSM501_PM2_5, SAMPLE_TIME_DSM510);
  Serial.println("[dsm501a] Ready");
}

void setup_voltage() {
  Serial.println("[volt] Setting up...");
  timer_voltage.attach(SAMPLE_TIME_VOLTAGE, read_voltage);
  Serial.println("[volt] Ready");
}

void read_dsm501a(){
  if (dsm501.update())
  { 
    pm25 = dsm501.getConcentration();
    Serial.println(String("[dsm501a] Read: PM1: ") + dsm501.getParticleCount(0) + ", PM2.5: " + dsm501.getParticleCount(1) + ", concentration: " + dsm501.getConcentration());
  } 
}

void read_bme280(){
  temperature = bme.readTemperature();
  pressure = bme.readPressure() / 100.0F;
  humidity = bme.readHumidity();
  Serial.println(String("[bme280] Read: temperature: ") + temperature  + "*C, humidity: " + humidity + "%, pressure: " + pressure + " hPa");
}

void read_voltage() {
  float calculated_voltage = analogRead(PIN_BATTERY_VOLTAGE) / 1024.0F * 3.3;
  battery_voltage = calculated_voltage;
  Serial.println(String("[volt] voltage: ") + battery_voltage + " V");
}

void setup_ap() {
  Serial.println("[wifi] Setting up...");
  WiFi.hostname(hostname);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  
  Serial.print("[wifi] SSID: ");
  Serial.println(ssid);
  Serial.print("[wifi] IP: ");
  Serial.println(myIP);
}

void setup_dns() {
  Serial.println("[dns] Serring up DNS...");
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(53, hostname, apIP);
  Serial.println(String("[dns] create entry for: ") + hostname);
}

void setup_web() {
  Serial.println("[www] Setting up endpoints...");
  server.on("/", handleRoot);
  server.on("/index.html", handleRoot);
  server.on("/style.css", handleCSS);
  server.on("/favicon.png", handleFavicon);
  server.on("/skunk.svg", handleSVGIcon);
  server.on("/cache.manifest", handleCacheM);
  server.on("/sensors/", handleSensors);
  server.begin();
  Serial.println("[www] Server running on port 80");
}

const char index_html[] PROGMEM = { 
  #include "index.html.h" 
  };
void handleRoot() { server.send(200, "text/html", index_html, sizeof(index_html)); }

const char style_css[] PROGMEM = { 
  #include "style.css.h" 
  };
void handleCSS() { server.send(200, "text/css", style_css, sizeof(style_css)); }

const char favicon_png[] PROGMEM = { 
  #include "favicon.png.h" 
  };
void handleFavicon() { server.send(200, "image/png", favicon_png, sizeof(favicon_png)); }

const char skunk_svg[] PROGMEM = { 
  #include "skunk.svg.h" 
  };
void handleSVGIcon() { server.send(200, "image/svg+xml", skunk_svg, sizeof(skunk_svg)); }

const char cache_manifest[] PROGMEM = { 
  #include "cache.manifest.h" 
  };
void handleCacheM() { server.send(200, "text/cache-manifest", cache_manifest, sizeof(cache_manifest)); }

void handleSensors() {
  server.send(
    200, "application/json", 
    String("{\"temperature\": ") + temperature + 
    ", \"pressure\": " + pressure + 
    ", \"humidity\": " + humidity + 
    ", \"uptime\": " + millis() + 
    ", \"voltage\": " + battery_voltage +
    ", \"pm25\": " + pm25 +
    "}");
}