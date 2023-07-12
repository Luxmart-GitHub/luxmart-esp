#ifndef WEB_USER_CONTROL_H
#define WEB_USER_CONTROL_H

class WebUserControl : public UserControl {
public:

  WebUserControl(
  const char* endpoints[], 
  const WebRequestMethod methods[], 
  const ArRequestHandlerFunction callbacks[],
  int length
  ) : server(80) {
  
    for(int i = 0; i < length; i++)
      server.on(endpoints[i], methods[i], callbacks[i]);
  }
  void iterate() {}
  void start() { server.begin(); }

  virtual ~WebUserControl() {}
private:
  AsyncWebServer server;
};

#endif
