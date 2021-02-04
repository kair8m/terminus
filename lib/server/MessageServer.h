#ifndef TERMINUS_MESSAGESERVER_H
#define TERMINUS_MESSAGESERVER_H

#include <iostream>
#include <memory>
#include <event.h>
#include <thread>
#include <fcntl.h>
#include <armadillo>

#include "message/Buffer.h"

class MessageServer {
private:
  const int MAX_CONNECT_QUEUE = 5;
  const bool TCP_NODELAY = false;
  const int BUF_SIZE = 4096;
public:
  MessageServer() {

  }

  void listen(const char *host, int port, int socketFlags = 0) {
    return bindAndListen(host, port, socketFlags);
  }

  virtual void onData(const Buffer &data) {

  }

private:

  void bindAndListen(const char *host, int port, int socketFlags) {
    fServerSocket = createSocket(host, port, socketFlags, TCP_NODELAY);
    if (fServerSocket == -1) {
      return;
    }

    if (port == 0) {
      sockaddr_storage addr = {0};
      socklen_t addr_len = sizeof(addr);
      if (getsockname(fServerSocket, reinterpret_cast<sockaddr *>(&addr),
                      &addr_len) == -1) {
        return;
      }
      if (addr.ss_family == AF_INET) {
        port = ntohs(reinterpret_cast<sockaddr_in *>(&addr)->sin_port);
      } else if (addr.ss_family == AF_INET6) {
        port = ntohs(reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_port);
      } else {
        return;
      }
    } else return listenInternal();
  }

  void listenInternal() {
    while (fServerSocket != -1) {
      int sock = accept(fServerSocket, nullptr, nullptr);

      if (sock == -1) {
        continue;
      }
      std::thread([&]() {
        onNewClient(sock);
        close(sock);
      });
    }
  }

  void onNewClient(int sock) {
    int size = 0;
    auto recvBuffer = new uint8_t[BUF_SIZE];
    while (true) {
      size = recv(sock,recvBuffer, BUF_SIZE, 0);
      if (size == -1)
        break;
      Buffer buffer(recvBuffer, size);
      onData(buffer);
    }
  }

  bool init(const char *host, int port) {
    fServerSocket = createSocket(host, port, 0, false);
    if (fServerSocket <= 0)
      return false;
  }

  int createSocket(const char *host, int port, int socketFlags, bool tcpNoDelay) {
    addrinfo hints = {0};
    addrinfo *result = nullptr;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = socketFlags;
    hints.ai_protocol = 0;

    auto service = std::to_string(port);

    if (getaddrinfo(host, service.c_str(), &hints, &result)) {
      return -1;
    }
    for (auto rp = result; rp; rp = rp->ai_next) {
      auto sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

      if (sock == -1)
        continue;

      if (fcntl(sock, F_SETFD, FD_CLOEXEC) == -1)
        continue;

      if (tcpNoDelay) {
        int yes = 1;
        setsockopt(sock, IPPROTO_TCP, tcpNoDelay, reinterpret_cast<char *>(&yes), sizeof(yes));
      }
      if (bindSocket(sock, *rp)) {
        freeaddrinfo(result);
        return sock;
      }

      close(sock);
    }

    freeaddrinfo(result);
    return -1;
  }

  int bindSocket(int sock, addrinfo &ai) const {
    if (::bind(sock, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen))) {
      return false;
    }
    if (::listen(sock, MAX_CONNECT_QUEUE)) {
      return false;
    }
    return true;
  }

  void listenThread() {
    while () {

    }
  }

private:
  int fServerSocket = -1;
  sockaddr_in fAddress = {};
};


#endif //TERMINUS_MESSAGESERVER_H
