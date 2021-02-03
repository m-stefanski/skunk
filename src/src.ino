

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <ESP8266WiFi.h>
#include <ESP8266TimerInterrupt.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const int ADVERTISE_MDNS = 5;
const int DUST_ANALOG = A0; 
const int DUST_LED = D5;

const char *ssid = "skunks"; // network name
const char *hostname = "skunks"; // dns domain name
IPAddress apIP(192, 168, 4, 1); // ip address

DNSServer dnsServer;
Adafruit_BME280 bme;
ESP8266WebServer server(80);
ESP8266Timer timer;

void setup() {
  Serial.begin(115200);
  cls();
  Serial.println("[app] Starting");
  setup_bme280();
  setup_dustmeter();
  setup_ap();
  setup_dns();
  setup_web();
  Serial.println("[app] Skunks ready");
}

void loop() {
  server.handleClient();
  dnsServer.processNextRequest();
}

void cls() {
  Serial.write(27);
  Serial.print("[2J");
  Serial.write(27);
  Serial.print("[H");
}

void setup_dustmeter() {
  pinMode(DUST_LED, OUTPUT);
}

void setup_bme280() {
  Serial.println("[bme280] Setting up...");
  bool status;
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("[bme280] ERROR: device not found!");
    while (1); // stopping the program
  }
  Serial.println("[bme280] Ready");
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

float read_dustmeter() {
  float voltsMeasured = 0;
  float calcVoltage = 0;
  float dustDensitySum = 0;
  float dustDensity = 0;

  for(int i=0; i<10; i++){
    digitalWrite(DUST_LED, LOW);
    delayMicroseconds(280);
    voltsMeasured = analogRead(DUST_ANALOG);
    delayMicroseconds(40);
    digitalWrite(DUST_LED, HIGH); 
    delayMicroseconds(9680);
    // voltage given as 0-1023 as a fraction of 3.3 V
    calcVoltage = voltsMeasured / 1024 * 3.3;
    // 0.5V for each 100 ug/m3
    dustDensity = calcVoltage / 0.5 * 100;
    Serial.println(String("[dust] Measure ") + i + " - raw Analog: " + voltsMeasured + ", dust: " + dustDensity);
    dustDensitySum += dustDensity;
  }

  dustDensity = dustDensitySum / 10.0F;
  Serial.println(String("[dust] Calculated dust: ") + dustDensity);
  return dustDensity;
}

void handleSensors() {
  server.send(200, "application/json", String("{\"temperature\": ") + bme.readTemperature() + ", \"pressure\": " + bme.readPressure() / 100.0F + ", \"humidity\": " + bme.readHumidity() + ", \"uptime\": " + millis() + + ", \"dust_pm25\": " + read_dustmeter() + "}");
}

