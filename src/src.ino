

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const char *ssid = "skun.ks"; // network name
const char *localdomain = "skun.ks"; // dns domain name
IPAddress apIP(192, 168, 4, 1); // ip address

DNSServer dnsServer;
Adafruit_BME280 bme;
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  cls();
  Serial.println("[app] Starting");
  setup_bme280();
  setup_ap();
  setup_dns();
  setup_spiffs();
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
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  
  Serial.print("[wifi] SSID: ");
  Serial.println(ssid);
  Serial.print("[wifi] IP: ");
  Serial.println(myIP);
}

void setup_dns() {
  Serial.println("[dns] Setting up...");
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(53, localdomain, apIP);
  Serial.print("[dns] Server exposed on: ");
  Serial.println(localdomain);
}

void setup_spiffs() { 
  Serial.println("[spiffs] Setting up...");
  if(!SPIFFS.begin()){ 
    Serial.println("[spiffs] ERROR");  
  }
  Serial.println("[spiffs] Ready");
}

void setup_web() {
  Serial.println("[www] Setting up...");
  server.on("/", handleRoot);
  server.on("/sensors/", handleSensors);
  server.on("/storage/", handleStorage);
  server.begin();
  Serial.println("[www] Server running on port 80");
}

void handleRoot() {
  const char *body = "<html><head><title>Skun.ks - handheld air monitor</title><meta charset=\"UTF-8\"></head><style type=\"text/css\"> body { font-family: \"Helvetica Neue\", Helvetica, Arial, sans-serif;} table { border: 1px solid dimgray;border-collapse: collapse;} td { border: 1px solid dimgray;} </style><body><h1>Skun.ks</h1><table><tr><td>Temperature:</td><td><span class=\"value\" id=\"temp\">#temp#</span> &#176;C</td></tr><tr><td>Humidity:</td><td><span class=\"value\" id=\"humid\">#humid#</span> %</td></tr><tr><td>Pressure:</td><td><span class=\"value\" id=\"pres\">#press#</span> hPa</td></tr></table></body><script> function reload_data () { var xhr = new XMLHttpRequest();xhr.onreadystatechange = function(){ if (xhr.readyState === 4){ var params = JSON.parse(xhr.responseText);document.getElementById('temp').innerHTML = params.temperature;document.getElementById('humid').innerHTML = params.humidity;document.getElementById('pres').innerHTML = params.pressure;} };xhr.open('GET', '/sensors/');xhr.send();} setInterval(reload_data, 1000);</script></html>";
  server.send(200, "text/html", body);
}

void handleSensors() {
  server.send(200, "application/json", String("{\"temperature\": ") + bme.readTemperature() + ", \"pressure\": " + bme.readPressure() / 100.0F + ", \"humidity\": " + bme.readHumidity() + "}");
}

void handleStorage() {
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  server.send(200, "application/json", String("{\"total_kb\": ") + fs_info.totalBytes / 1024.0F + ", \"used_kb\": " + fs_info.usedBytes / 1024.0F + ", \"free_kb\": " + (fs_info.totalBytes - fs_info.usedBytes) / 1024.0F + "}");
}
