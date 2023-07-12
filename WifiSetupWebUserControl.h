#ifndef WIFI_SETUP_WEB_USER_CONTROL
#define WIFI_SETUP_WEB_USER_CONTROL

#include "WebUserControl.h"

class WifiSetupWebUserControl : public WebUserControl {

public:

    WifiSetupWebUserControl() : WebUserControl(
        endpoints,
        methods,
        callbacks,
        2
    ) {}

private:
    static void getHomeCallback(AsyncWebServerRequest* request);
    static void postHomeCallback(AsyncWebServerRequest* request);
    
    static const char* endpoints[];
    static const WebRequestMethod methods[];
    static const ArRequestHandlerFunction callbacks[];

    static const char* PARAM_INPUT_1;
    static constexpr const char* PARAM_INPUT_2 = "pass";
    static constexpr const char* PARAM_INPUT_3 = "ip";
    static constexpr const char* PARAM_INPUT_4 = "gateway";
};

const char* WifiSetupWebUserControl::endpoints[2] = {"/", "/"};
const WebRequestMethod WifiSetupWebUserControl::methods[2] = {HTTP_GET, HTTP_POST};
const ArRequestHandlerFunction WifiSetupWebUserControl::callbacks[] = {getHomeCallback, postHomeCallback};


const char* WifiSetupWebUserControl::PARAM_INPUT_1 = "ssid";

void WifiSetupWebUserControl::getHomeCallback(AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", wifimanager_html, wifimanager_html_len);
};

void WifiSetupWebUserControl::postHomeCallback(AsyncWebServerRequest* request) {
    int params = request->params();
    WifiData wifiData;
    for(int i = 0; i < params; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
            // HTTP POST ssid value
            if (p->name() == PARAM_INPUT_1) {
                ssid = p->value().c_str();
                strcpy(wifiData.ssid, p->value().c_str());
            }
            // HTTP POST pass value
            if (p->name() == PARAM_INPUT_2) {
                password = p->value().c_str();
                strcpy(wifiData.password, p->value().c_str());
            }
            // HTTP POST ip value
            if (p->name() == PARAM_INPUT_3) {
                ip = p->value().c_str();
                strcpy(wifiData.ip, p->value().c_str());
            }
            // HTTP POST gateway value
            if (p->name() == PARAM_INPUT_4) {
                gateway = p->value().c_str();
                strcpy(wifiData.gateway,p->value().c_str());
            }
        }
    }
    EEPROM.put(0, wifiData);
    EEPROM.commit();
    request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
    delay(3000);
    ESP.restart();
};

#endif