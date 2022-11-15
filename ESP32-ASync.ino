// Import required libraries
#include <WiFi.h>
#include <AsyncTCP.h>
// https://github.com/me-no-dev/ESPAsyncWebServer
#include <ESPAsyncWebServer.h>
#include "uptime_formatter.h"
#include "pages.h"

// LCD
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// LCD END

// NTP/TIME
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -28800;
const int   daylightOffset_sec = 3600;
void printTime(){
  
  struct tm time;
   
  if(!getLocalTime(&time)){
    Serial.println("Could not obtain time info");
    return;
  }
 
  Serial.println("\n---------TIME----------");
   
  Serial.println(asctime(&time));
   
  char buffer[80];
   
  strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", &time);
  Serial.println(buffer);
 
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &time);
  Serial.println(buffer);
 
  strftime(buffer, sizeof(buffer), "%Y/%m/%d", &time);
  Serial.println(buffer);
}

String getTimestamp() {
  struct tm time;
   
  if(!getLocalTime(&time)){
    Serial.println("Could not obtain time info");
  }
  char buffer[80];
  strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", &time);
  return buffer;
}
//

// SD CARD
#include <FS.h>
#include <SPIFFS.h>
#include <SPIFFSEditor.h>
// SD CARD END

const char *ASYNCServerVer = "1.0";
int Count = 0;

// Network Info
const char* ssid = "Browncow2";
const char* password = "62619462";
//

// GET VARIABLES
const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Events source
AsyncEventSource events("/events");

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Output - GPIO 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 4</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 33</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"33\" " + outputState(33) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  if(var == "JAVASCRIPT0"){
    String JAVASCRIPT0 = "";
    JAVASCRIPT0 += "<script src=\"https://code.jquery.com/jquery-3.3.1.slim.min.js\" integrity=\"sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo\" crossorigin=\"anonymous\"></script>";
    JAVASCRIPT0 += "<script src=\"https://cdn.jsdelivr.net/npm/popper.js@1.14.3/dist/umd/popper.min.js\" integrity=\"sha384-ZMP7rVo3mIykV+2+9J3UJ46jBk0WLaUAdn689aCwoqbBJiSnjAK/l8WvCWPIPm49\" crossorigin=\"anonymous\"></script>";
    JAVASCRIPT0 += "<script src=\"https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/js/bootstrap.min.js\" integrity=\"sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy\" crossorigin=\"anonymous\"></script>";
    return JAVASCRIPT0;
  }
  if(var == "ASYNCSERVERVER"){
    String ver = ASYNCServerVer;
    return ver;
  }
  if(var == "PAGECOUNT"){
    String pageCount = String(Count);
    return pageCount;
  }
  if(var == "UPTIME"){
    String Uptime = String(uptime_formatter::getUptime());
    return Uptime;
  }
  if(var == "CPUFREQ"){
    String CpuFreq = String(ESP.getCpuFreqMHz());
    return CpuFreq;
  }
  return String();
}

String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}

String logRequest(String(timestamp), String(remoteIP), String(version), String(method), String(url), String(host), String(contentType), String(contentLength), String(multipart)) {

 String logString = timestamp + "," + remoteIP + "," + "HTTP/1." + version + "," + method + "," + url + "," + host + "," + contentType + "," + contentLength + "," + multipart;
 Serial.println(String(logString));
 return logString;
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  pinMode(4, OUTPUT);                                                                                                                                                                                                                                          
  digitalWrite(4, LOW);
  pinMode(33, OUTPUT);
  digitalWrite(33, LOW);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // End Time
  
  // Fav Icon
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "image/x-icon", favicon_ico_gz, favicon_ico_gz_len);
    Serial.println("Served : /favicon.ico - Count: "+String(Count));
    String timestamp = String(getTimestamp());
    logRequest(String(timestamp), String(request->client()->remoteIP().toString()), String(request->version()), String(request->methodToString()), String(request->url()), String(request->host()), String(request->contentType()), String(request->contentLength()), String(request->multipart()));
    Count++;
  });
  /////
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
    Serial.println("Served : / - Count: "+String(Count));
    String timestamp = String(getTimestamp());
    //Serial.println(request->client()->remoteIP());
    logRequest(String(timestamp), String(request->client()->remoteIP().toString()), String(request->version()), String(request->methodToString()), String(request->url()), String(request->host()), String(request->contentType()), String(request->contentLength()), String(request->multipart()));
    Count++; 
  });

  // Route for buttons page
  server.on("/buttons", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", buttons_html, processor);
    Serial.println("Served : /buttons - Count: "+String(Count));
    String timestamp = String(getTimestamp());
    logRequest(String(timestamp), String(request->client()->remoteIP().toString()), String(request->version()), String(request->methodToString()), String(request->url()), String(request->host()), String(request->contentType()), String(request->contentLength()), String(request->multipart()));
    Count++;
  });

  // pages.h test
  // Just tests that pages.h was compiled in properly
  server.on("/pages_test", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", pages_test, processor);
    Serial.println("Served : /pages_test - Count: "+String(Count));
    String timestamp = String(getTimestamp());
    logRequest(String(timestamp), String(request->client()->remoteIP().toString()), String(request->version()), String(request->methodToString()), String(request->url()), String(request->host()), String(request->contentType()), String(request->contentLength()), String(request->multipart()));
    Count++;
  });

  // Wifi Scan
  // FIXME (11/3/22) : Usually fails the first request with empty JSON array
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
  String json = "[";
  int n = WiFi.scanComplete();
  if(n == -2){
    WiFi.scanNetworks(true);
  } else if(n){
    for (int i = 0; i < n; ++i){
      if(i) json += ",";
      json += "{";
      json += "\"rssi\":"+String(WiFi.RSSI(i));
      json += ",\"ssid\":\""+WiFi.SSID(i)+"\"";
      json += ",\"bssid\":\""+WiFi.BSSIDstr(i)+"\"";
      json += ",\"channel\":"+String(WiFi.channel(i));
      json += ",\"secure\":"+String(WiFi.encryptionType(i));
      //json += ",\"hidden\":"+String(WiFi.isHidden(i)?"true":"false");
      json += "}";
    }
    WiFi.scanDelete();
    if(WiFi.scanComplete() == -2){
      WiFi.scanNetworks(true);
    }
  }
  json += "]";
  request->send(200, "application/json", json);
  json = String();
  Serial.println("Served : /scan - Count: "+String(Count));
  String timestamp = String(getTimestamp());
  logRequest(String(timestamp), String(request->client()->remoteIP().toString()), String(request->version()), String(request->methodToString()), String(request->url()), String(request->host()), String(request->contentType()), String(request->contentLength()), String(request->multipart()));
  Count++;
});

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    Serial.print("GPIO: ");
    Serial.print(inputMessage1);
    Serial.print(" - Set to: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
    Serial.println("Served : /update - Count: "+String(Count));
    String timestamp = String(getTimestamp());
    logRequest(String(timestamp), String(request->client()->remoteIP().toString()), String(request->version()), String(request->methodToString()), String(request->url()), String(request->host()), String(request->contentType()), String(request->contentLength()), String(request->multipart()));
    Count++;
  });

/*
 * Not Found Handler
 */

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", notFound_html, processor);
    Serial.println("Served : NOT FOUND/404 - Count: "+String(Count));
    String timestamp = String(getTimestamp());
    logRequest(String(timestamp), String(request->client()->remoteIP().toString()), String(request->version()), String(request->methodToString()), String(request->url()), String(request->host()), String(request->contentType()), String(request->contentLength()), String(request->multipart()));
    Count++;
  });
///////////////////////////

  // Start server
  server.begin();
}

void loop() {
// LCD
// LCD -- INIT FLASH VERSION/STARTING
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  } else { Serial.println("SSD1306 allocated"); }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("\nInit ASYNCWeb: v" + String(ASYNCServerVer));
  display.println("IP: " + WiFi.localIP().toString());
  display.println("Page Count: " + String(Count));
  display.display(); 
// LCD END
}
