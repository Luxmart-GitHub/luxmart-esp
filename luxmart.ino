/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-wi-fi-manager-asyncwebserver/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <stdlib.h>
#include <EEPROM.h>

#define EEPROM_SIZE 130

#include "html.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";


//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

// Initialize WiFi
bool initWiFi() {
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());


  if (!WiFi.config(localIP, localGateway, subnet)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect.");
      return false;
    }
    yield();
  }

  Serial.println(WiFi.localIP());
  return true;
}

void handleSetValuePost(AsyncWebServerRequest* request) {
  int numberOfParams = request->params();
  for(int i = 0; i < numberOfParams; i++) {
    AsyncWebParameter* param = request->getParam(i);
    if(param->isPost()) {
      int value = atoi(param->value().c_str());
      Serial.print("Sending value: ");
      Serial.println(value);
      // Send signal to UART

      Serial1.write('0');
      for(int i = 0; i < value; i++)
        Serial1.write('w');
    }
  }

  

  String htmlString = (char*)index_html;
  request->send(200, "text/html", htmlString);
}

struct WifiData {
  char ssid[33];
  char password[64];
  char ip[16];
  char gateway[16];
};


void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);

  WifiData readWifiData;

  EEPROM.get(0, readWifiData);
  
  // Load values saved in SPIFFS
  ssid = readWifiData.ssid;
  pass = readWifiData.password;
  ip = readWifiData.ip;
  gateway = readWifiData.gateway;
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  if(initWiFi()) {  

    Serial1.begin(9600);

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      String htmlString = (char*)index_html;
      request->send(200, "text/html", htmlString);
    });

    server.on("/", HTTP_POST, handleSetValuePost);

    server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
      WifiData wifiData;
      wifiData.ssid[0] = '\0';
      wifiData.password[0] = '\0';
      wifiData.ip[0] = '\0';
      wifiData.gateway[0] = '\0';
      EEPROM.put(0, wifiData);
      EEPROM.commit();
      request->send(200, "text/plain", "Wifi Settings Reset. ESP will restart.");
      ESP.restart();
    });

    server.begin();
  }
  else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      String htmlString = (char*)wifimanager_html;
      request->send(200, "text/html", htmlString);
    });
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      WifiData wifiData;
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            strcpy(wifiData.ssid, p->value().c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            strcpy(wifiData.password, p->value().c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            strcpy(wifiData.ip, p->value().c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            strcpy(wifiData.gateway,p->value().c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      Serial.print(wifiData.ssid);
      EEPROM.put(0, wifiData);
      EEPROM.commit();
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart();
    });
    server.begin();
  }
}

void loop() {
} 
