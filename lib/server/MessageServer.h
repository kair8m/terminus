#ifndef TERMINUS_MESSAGESERVER_H
#define TERMINUS_MESSAGESERVER_H

#include <iostream>
#include <memory>

#include "message/Buffer.h">

class MessageServer {
public:
  explicit MessageServer(const std::string &serverUrl) {

  }


  MessageServer(const uint8_t *ip, uint16_t port) {

  }

  virtual void onData(const Buffer &data) = 0;

  virtual void onNewClient() {}

private:

};


#endif //TERMINUS_MESSAGESERVER_H
