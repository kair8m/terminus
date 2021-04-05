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
#include <netinet/tcp.h>

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
  static const bool ENABLE_TCP_NODELAY = false;
  static const int BUF_SIZE = 4096;
  static const int KEEPALIVE_MAXCOUNT = 10;
private:
  int fServerSocket = -1;
  int fBufferSize = -1;
  int fMaxConnectQueue = -1;
  bool fTcpNoDelay = false;
  bool isRunning = false;
  bool stopRunning = false;
  std::map<const std::string, ClientParams> fClientThreadPool;
  bool fKeepAlive = false;
  int fKeepAliveInterval = -1;
  std::mutex fMutex;

public:
  explicit MessageServer(int bufferSize = BUF_SIZE, bool tcpNoDelay = ENABLE_TCP_NODELAY, int maxConnectQueue = MAX_CONNECT_QUEUE) :
    fBufferSize(bufferSize), fTcpNoDelay(tcpNoDelay), fMaxConnectQueue(maxConnectQueue) {

  }

  ~MessageServer() {
    stop();
  }

  void enableKeepAlive(int keepAliveInterval, int maxDropPackets = KEEPALIVE_MAXCOUNT) {
    fKeepAliveInterval = keepAliveInterval;
    fKeepAlive = true;
  }

  void listen(const char *host, int &port, int socketFlags = 0) {
    DWARN("starting listening %s:%d", host, port);
    return bindAndListen(host, port, socketFlags);
  }

  void stop() {
    if (!isRunning) return;
    std::lock_guard<std::mutex> lock(fMutex);
    stopRunning = true;
    close(fServerSocket);
    for (auto &item : fClientThreadPool) {
      DWARN("stopping session with client %s", item.first.c_str());
      close(item.second.socket);
    }
  }

  bool sendTo(const char *client, const Buffer &data) {
    std::lock_guard<std::mutex> lock(fMutex);
    auto item = fClientThreadPool.find(client);
    if (item == fClientThreadPool.end()) {
      DERROR("couldn't find client %s in client thread pool", client);
      return false;
    }
    if (send(item->second.socket, data.getDataPtr(), data.getSize(), 0) < 0) return false;

    return true;
  }

protected:

  virtual bool onData(const Buffer &data, const char *client) {
    if (data.getSize() == 0) {
      DERROR("data empty");
      return false;
    }
    DINFO("data arrived! size: %zu", data.getSize());
    return true;
  }

  virtual void onClientDisconnected(const char *client) {
    DERROR("client %s has disconnected");
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

      char client[INET_ADDRSTRLEN];

      inet_ntop(AF_INET, &(peer.sin_addr), client, INET_ADDRSTRLEN);

      if (fKeepAlive && !setKeepAlive(sock, fKeepAliveInterval, KEEPALIVE_MAXCOUNT)) {
        DERROR("failed to setup keepalive for client %s", client);
        close(sock);
        continue;
      }

      std::lock_guard<std::mutex> lock(fMutex);

      if (fClientThreadPool.find(client) != fClientThreadPool.end()) {
        DERROR("refusing connection due to multiple accessing!");
        close(sock);
        continue;
      }

      std::string remote = client + std::string(":") + std::to_string(peer.sin_port);

      fClientThreadPool[remote].socket = sock;
      fClientThreadPool[remote].t = std::thread([this, &remote, &sock]() {
        onNewClient(remote.c_str(), sock);
        close(sock);
        fClientThreadPool.erase(remote);
        onClientDisconnected(remote.c_str());
      });
      fClientThreadPool[remote].t.detach();
    }
  }

  void onNewClient(const char *client, int sock) {
    DINFO("new client connected: %s", client);
    int size;
    auto recvBuffer = new uint8_t[fBufferSize];
    while (true) {
      size = recv(sock, recvBuffer, fBufferSize, 0);
      if (size <= 0)
        break;
      Buffer buffer(recvBuffer, size);
      if (!onData(buffer, client))
        break;
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

  static bool setKeepAlive(int sock, int keepAliveInterval, int maxDropPackets) {
    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int))) {
      DCRITICAL("failed to set keep alive flag to socket %d", sock);
      return false;
    }

    int idle = 1;
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int))) {
      DCRITICAL("failed to set keep idle flag to socket %d", sock);
      return false;
    }

    int interval = keepAliveInterval;
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int))) {
      DCRITICAL("failed to set keep interval flag to socket %d", sock);
      return false;
    }

    int maxpkt = maxDropPackets;
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int))) {
      DCRITICAL("failed to set keep count flag to socket %d", sock);
      return false;
    }
    return true;
  }
};


#endif //TERMINUS_MESSAGESERVER_H