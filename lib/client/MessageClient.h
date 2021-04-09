#ifndef TERMINUS_MESSAGECLIENT_H
#define TERMINUS_MESSAGECLIENT_H

#include <arpa/inet.h>
#include <netdb.h>
#include <thread>

#include "message/Buffer.h"

class MessageClient {
private:
  static const int BUFFER_SIZE = 4069;
private:
  int fSocket = -1;
  sockaddr_in fServerAddress = {};
  std::thread fReceiveThread = {};
  bool fShutDown = false;
  int fBufferSize = -1;
public:
  explicit MessageClient(int bufferSize = BUFFER_SIZE) : fBufferSize(bufferSize) {
  }

  ~MessageClient() {
    if (fSocket == -1) return;
    DWARN("disconnecting from %s", inet_ntoa(fServerAddress.sin_addr));
    if (fReceiveThread.joinable()) fReceiveThread.detach();
    close(fSocket);
  }

  bool connect(const std::string &address, int port, bool async = true) {
    fSocket = 0;

    fSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fSocket == -1) { //socket failed
      DCRITICAL("failed to create socket");
      perror("create socket failed");
      return false;
    }

    int inetSuccess = inet_aton(address.c_str(), &fServerAddress.sin_addr);

    if (!inetSuccess) {
      struct hostent *host;
      struct in_addr **addrList;
      if ((host = gethostbyname(address.c_str())) == nullptr) {
        perror("Failed to resolve hostname");
        DCRITICAL("Failed to resolve hostname");
        return false;
      }
      addrList = (struct in_addr **) host->h_addr_list;
      fServerAddress.sin_addr = *addrList[0];
    }
    fServerAddress.sin_family = AF_INET;
    fServerAddress.sin_port = htons(port);

    int connectRet = ::connect(fSocket, (struct sockaddr *) &fServerAddress, sizeof(fServerAddress));
    if (connectRet == -1) {
      perror("connect failed");
      DCRITICAL("connect failed");
      return false;
    }
    if (async) fReceiveThread = std::thread(&MessageClient::receiveTask, this);
    return true;
  }

  bool sendData(const char *msg, size_t size) const {
    size_t numBytesSent = send(fSocket, msg, size, 0);
    if (numBytesSent < 0) { // send failed
      perror("send failed");
      DCRITICAL("send failed");
      return false;
    }
    if ((uint) numBytesSent < size) { // not all bytes were sent
      DERROR("not all bytes were sent, requested size: %d, result: %d", size, numBytesSent);
      return false;
    }
    return true;
  }

  Buffer receiveData() const {
    auto buffer = new uint8_t[fBufferSize];
    auto size = recv(fSocket, buffer, fBufferSize, 0);
    Buffer data(buffer, size);
    delete[] buffer;
    return data;
  }

protected:

  virtual void onReceivedData(const Buffer &data) {
    if (data.getSize() == 0) {
      DERROR("data empty");
      return;
    }
    DINFO("data arrived! size: %zu", data.getSize());
  }

private:

  void receiveTask() {
    size_t size;
    auto buffer = new uint8_t[fBufferSize];

    while (!fShutDown) {
      size = recv(fSocket, buffer, fBufferSize, 0);

      if (size <= 0) {
        break;
      }

      Buffer callbackBuffer(buffer, size);

      onReceivedData(callbackBuffer);
    }

    delete[] buffer;
  }

};


#endif //TERMINUS_MESSAGECLIENT_H
