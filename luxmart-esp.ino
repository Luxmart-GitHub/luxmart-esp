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

//Variables to save values from HTML form
String ssid;
String password;
String ip;
String gateway;

struct WifiData {
  char ssid[33];
  char password[64];
  char ip[16];
  char gateway[16];
};

#include "api/pwm_controller_commands.h"
#include "common/pwm_controller_esp.h"
#include "common/pwm_controller_user.h"
#include "html.h"

#include "PwmControllerWebUserControl.h"
#include "WifiSetupWebUserControl.h"

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
  WiFi.begin(ssid.c_str(), password.c_str());
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

int readValueFromUART(int timeout) {
  Serial1.write('r');
  Serial.println("Requesting current value...");
  
  int ch;
  int startTime = millis();
  int time = 0;

  ESP.wdtDisable();
  do {
    ch = Serial1.read();

    time = millis();
  }
  while(ch != 'w' && time < startTime + timeout);
  ch = Serial1.read();

  Serial.println(ch);

  ESP.wdtEnable(1000);
    
  if(time >= startTime + timeout)
    ch = -1;

  Serial.println("Read value!");

  return ch;
}

String processor(const String& var) {
  if(var == "VALUE") {
    int value = readValueFromUART(2000);
    return value >= 0 ? String(value) : "COULD NOT READ VALUE";
  }
  return String();
}

void pwm_controller_setup() {
    
    // Load saved WiFi Data
    WifiData savedWifiData;
    EEPROM.get(0, savedWifiData);
    
    ssid = savedWifiData.ssid;
    password = savedWifiData.password;
    ip = savedWifiData.ip;
    gateway = savedWifiData.gateway;
    
    // Start Web Server
    WebUserControl* webServer;
    if(setupWiFi()) {
        webServer = new PwmControllerWebUserControl();
    } else {
        webServer = new WifiSetupWebUserControl();
    }

    webServer->start();
}

void pwm_controller_loop() {}
