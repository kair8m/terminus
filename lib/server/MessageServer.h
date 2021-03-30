#ifndef TERMINUS_MESSAGESERVER_H
#define TERMINUS_MESSAGESERVER_H

#include <iostream>
#include <memory>
#include <event.h>
#include <thread>
#include <fcntl.h>
#include <armadillo>
#include <condition_variable>
#include <arpa/inet.h>

#include "message/Buffer.h"

#include "logger/Logger.h"

class MessageServer {
private:
  struct ClientParams {
    int socket;
    std::thread t;
  };
private:
  static const int MAX_CONNECT_QUEUE = 5;
  static const bool TCP_NODELAY = false;
  static const int BUF_SIZE = 4096;
public:
  explicit MessageServer(int bufferSize = BUF_SIZE, bool tcpNoDelay = TCP_NODELAY, int maxConnectQueue = MAX_CONNECT_QUEUE) :
    fBufferSize(bufferSize), fTcpNoDelay(tcpNoDelay), fMaxConnectQueue(maxConnectQueue) {

  }

  ~MessageServer() {
    stop();
  }

  void listen(const char *host, int &port, int socketFlags = 0) {
    DWARN("starting listening %s:%d", host, port);
    return bindAndListen(host, port, socketFlags);
  }

  void stop() {
    if (!isRunning) return;
    stopRunning = true;
    close(fServerSocket);
    for (auto &item : fClientThreadPool) {
      DWARN("stopping session with client %s", item.first.c_str());
      close(item.second.socket);
    }
  }

protected:

  virtual void onData(const Buffer &data) {
    if (data.getSize() == 0) {
      DERROR("data empty");
      return;
    }
    DINFO("data arrived! size: %zu", data.getSize());
  }

private:

  void bindAndListen(const char *host, int &port, int socketFlags) {
    fServerSocket = createSocket(host, port, socketFlags, fTcpNoDelay);
    if (fServerSocket == -1) {
      DCRITICAL("createSocket failed");
      return;
    }

    if (port == 0) {
      sockaddr_storage addr = {0};
      socklen_t addr_len = sizeof(addr);
      if (getsockname(fServerSocket, reinterpret_cast<sockaddr *>(&addr),
                      &addr_len) == -1) {
        DCRITICAL("getsockname failed");
        return;
      }
      if (addr.ss_family == AF_INET) {
        port = ntohs(reinterpret_cast<sockaddr_in *>(&addr)->sin_port);
        DWARN("result port: %d", port);
      } else if (addr.ss_family == AF_INET6) {
        DWARN("result port: %d", port);
        port = ntohs(reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_port);
      } else {
        DCRITICAL("%s is unable to listen on port %d", host, port);
        return;
      }
    } else return listenInternal();
  }

  void listenInternal() {
    isRunning = true;
    while (fServerSocket != -1) {
      int sock = accept(fServerSocket, nullptr, nullptr);

      if (sock == -1) {
        DERROR("accept failed");
        continue;
      }

      sockaddr_in peer = {};

      socklen_t peerLen = sizeof(peer);

      getpeername(sock, (sockaddr *) &peer, &peerLen);

      char str[INET_ADDRSTRLEN];

      inet_ntop(AF_INET, &(peer.sin_addr), str, INET_ADDRSTRLEN);

      fClientThreadPool[str].socket = sock;
      fClientThreadPool[str].t = std::thread([&]() {
        onNewClient(str, sock);
        close(sock);
        fClientThreadPool.erase(str);
      });
      fClientThreadPool[str].t.detach();
    }
  }

  void onNewClient(const char* client, int sock) {
    DINFO("new client connected: %s", client);
    int size;
    auto recvBuffer = new uint8_t[fBufferSize];
    while (true) {
      size = recv(sock, recvBuffer, fBufferSize, 0);
      if (size == -1)
        break;
      Buffer buffer(recvBuffer, size);
      onData(buffer);
    }
    delete[] recvBuffer;
    DWARN("client %s disconnected", client);
  }

  bool init(const char *host, int port) {
    fServerSocket = createSocket(host, port, 0, false);
    if (fServerSocket <= 0)
      return false;
    return true;
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

      int yes = 1;

      if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&yes), sizeof(yes))) {
        DCRITICAL("failed to set reuse address socket option for socket %d", sock);
        continue;
      }

      if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<char *>(&yes), sizeof(yes))) {
        DCRITICAL("failed to set reuse port socket option for socket %d", sock);
        continue;
      }

      if (tcpNoDelay && setsockopt(sock, IPPROTO_TCP, tcpNoDelay, reinterpret_cast<char *>(&yes), sizeof(yes))) {
        DCRITICAL("failed to set tcp no delay socket option for socket %d", sock);
        continue;
      }
      if (bindSocket(sock, *rp)) {
        freeaddrinfo(result);
        return sock;
      }
      DCRITICAL("failed to bind socket %d", sock);

      close(sock);
    }

    freeaddrinfo(result);
    return -1;
  }

  int bindSocket(int sock, addrinfo &ai) const {
    if (::bind(sock, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen))) {
      return false;
    }
    if (::listen(sock, fMaxConnectQueue)) {
      return false;
    }
    return true;
  }

private:
  int fServerSocket = -1;
  int fBufferSize = -1;
  int fMaxConnectQueue = -1;
  bool fTcpNoDelay = false;
  bool isRunning = false;

  bool stopRunning = false;

  std::map<const std::string, ClientParams> fClientThreadPool;
};


#endif //TERMINUS_MESSAGESERVER_H