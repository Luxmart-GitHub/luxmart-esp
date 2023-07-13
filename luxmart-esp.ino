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

// Config
#define DEBUG

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

WebUserControl* webServer;

bool setupSuccessful = false;

void start_web_server() {
    
    if(WiFi.status() == WL_CONNECTED) {
        DBGLOG("Connected successfully. Starting web server");
        webServer = new PwmControllerWebUserControl();

        setupSuccessful = true;
    } else {
        DBGLOG("Setting AP");

        WiFi.softAP("ESP-WIFI-MANAGER", NULL);

        DBGLOG("Starting Web Server");
        webServer = new WifiSetupWebUserControl();
    }

    webServer->start();

}

void pwm_controller_setup() {
    EEPROM.begin(EEPROM_SIZE);

    // Load saved WiFi Data
    WifiData savedWifiData;
    EEPROM.get(0, savedWifiData);
    
    ssid = savedWifiData.ssid;
    password = savedWifiData.password;
    ip = savedWifiData.ip;
    gateway = savedWifiData.gateway;
    
    DBGLOG("Connecting to WiFi...");
    
    setupWiFi();
    start_web_server();
}

#define RESET_COUNT 3
int resetCounter = 0;

void pwm_controller_loop() {
    webServer->iterate();
    
    if(setupSuccessful) {
        if(WiFi.status() != WL_CONNECTED && resetCounter++ < RESET_COUNT) {
            setupWiFi();
        }
        if(WiFi.status() == WL_CONNECTED) resetCounter = 0;
        if(resetCounter >= RESET_COUNT) ESP.reset();
    }
}
