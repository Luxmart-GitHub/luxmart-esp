#ifndef PWM_CONTROLLER_WEB_USER_CONTROL_H
#define PWM_CONTROLLER_WEB_USER_CONTROL_H

#include "WebUserControl.h"

class PwmControllerWebUserControl : public WebUserControl {

public:

    PwmControllerWebUserControl() : WebUserControl(
        endpoints,
        methods,
        callbacks,
        4
    ) {
        lastTimeValueRead = millis();
    }

    void iterate();

private:

    static void getHomeCallback(AsyncWebServerRequest* request);
    static void getValueCallback(AsyncWebServerRequest* request);
    static void postHomeCallback(AsyncWebServerRequest* request);
    static void postResetCallback(AsyncWebServerRequest* request);

    static const char* endpoints[]; 
    static const WebRequestMethod methods[];
    static const ArRequestHandlerFunction callbacks[];

    static String templateProcessor(const String&);

    static char value;
    static bool valueValid;
    static unsigned long lastTimeValueRead; 
    static const int valueRequestDelay = 100;
};

char PwmControllerWebUserControl::value = 0;
bool PwmControllerWebUserControl::valueValid = 0;
unsigned long PwmControllerWebUserControl::lastTimeValueRead = 0;

const char* PwmControllerWebUserControl::endpoints[] = {"/", "/value", "/", "/reset"};
const WebRequestMethod PwmControllerWebUserControl::methods[] = {HTTP_GET, HTTP_GET, HTTP_POST, HTTP_POST};
const ArRequestHandlerFunction PwmControllerWebUserControl::callbacks[] = {getHomeCallback, getValueCallback, postHomeCallback, postResetCallback};

void PwmControllerWebUserControl::iterate() {
    if(millis() >= lastTimeValueRead + valueRequestDelay) {
        ReturnedValue err = executeCommand(CommandGetValue, nullptr, &value);
        if(err) {
            value = -1;
            valueValid = false;
        } else {
            valueValid = true;
        }
        
        DBGLOG("Received value:");
        DBGLOG(value);

        lastTimeValueRead = millis();
    }
}

String PwmControllerWebUserControl::templateProcessor(const String& var) {
  if(var == "VALUE") {
    return valueValid ? String(value) : "?";
  }
  return String();
}

void PwmControllerWebUserControl::getHomeCallback(AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, index_html_len, templateProcessor);
};

void PwmControllerWebUserControl::getValueCallback(AsyncWebServerRequest* request) {
    request->send(200, "text/plain", String(value));
};

void PwmControllerWebUserControl::postHomeCallback(AsyncWebServerRequest* request) {
    DBGLOG("Received Post Request");

    int numberOfParams = request->params();
    for(int i = 0; i < numberOfParams; i++) {
        AsyncWebParameter* param = request->getParam(i);
        if(param->isPost()) {
            value = atoi(param->value().c_str());
            
            webServer->executeCommand(CommandSetValue, &value, nullptr);

            DBGLOG("Set value to");
            DBGLOG(value);
        }
    }
    
    request->send_P(200, "text/html", index_html, index_html_len, templateProcessor);
};

void PwmControllerWebUserControl::postResetCallback(AsyncWebServerRequest* request) {
    WifiData wifiData;
    wifiData.ssid[0] = '\0';
    wifiData.password[0] = '\0';
    wifiData.ip[0] = '\0';
    wifiData.gateway[0] = '\0';
    EEPROM.put(0, wifiData);
    EEPROM.commit();
    request->send(200, "text/plain", "Wifi Settings Reset. ESP will restart.");
    ESP.restart();
};

#endif
