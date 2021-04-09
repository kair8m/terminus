#ifndef TERMINUS_MESSAGESERVER_H
#define TERMINUS_MESSAGESERVER_H

#include <iostream>
#include <memory>
#include <event.h>
#include <thread>
#include <fcntl.h>
#include <armadillo>
#include <condition_variable>
#include <utility>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include <message/Buffer.h>
#include <logger/Logger.h>
#include <message/MessageParser.h>

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
  std::map<std::string, int> fMasterSocketPool;
  std::map<std::string, int> fSlaveSocketPool;
  bool fKeepAlive = false;
  int fKeepAliveInterval = -1;
  std::mutex fMutex;
  std::shared_ptr<MessageParser> fMessageParser = nullptr;
  std::string fServerLogin;
  std::string fServerPassword;

public:
  explicit MessageServer(std::string login, std::string password, int bufferSize = BUF_SIZE,
                         bool tcpNoDelay = ENABLE_TCP_NODELAY, int maxConnectQueue = MAX_CONNECT_QUEUE) :
    fBufferSize(bufferSize), fTcpNoDelay(tcpNoDelay), fMaxConnectQueue(maxConnectQueue),
    fServerLogin(std::move(login)), fServerPassword(std::move(password)) {
    fMessageParser = std::make_shared<MessageParser>(fServerLogin, fServerPassword);
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
        clientHandler(remote.c_str(), sock);
        close(sock);
        fClientThreadPool.erase(remote);
      });
      fClientThreadPool[remote].t.detach();
    }
  }

  void clientHandler(const char *client, int sock) {
    DINFO("new client connected: %s", client);
    size_t size;
    int redirectSocket = -1;
    std::string clientId;
    auto recvBuffer = new uint8_t[fBufferSize];
    std::shared_ptr<ConnectionType> connectionType = nullptr;
    while (true) {
      size = recv(sock, recvBuffer, fBufferSize, 0);
      if (size <= 0) break;

      auto parseResult = fMessageParser->parse(recvBuffer, size);
      if (!parseResult) {
        DERROR("failed to parse incoming message");
        break;
      }

      if (parseResult->getId() == ConnectMessage::id) {
        clientId = parseResult->cast<ConnectMessage>().getConnectOptions().getClientId();
        if (!connectMessageHandler(client, sock, parseResult, connectionType)) break;
        continue;
      }

      if (redirectSocket < 0 && connectionType != nullptr) {
        redirectSocket = getRedirectSocket(clientId, *connectionType);
      }

      if (redirectSocket < 0) continue;

      send(redirectSocket, recvBuffer, size, 0);

    }
    delete[] recvBuffer;
    DWARN("client %s disconnected", client);
    if (connectionType == nullptr) return;

    DWARN("erasing id %s from session socket pool", clientId.c_str());
    switch (*connectionType) {
      case ConnectionType::TypeSlave:
        fSlaveSocketPool.erase(clientId);
        break;
      case ConnectionType::TypeMaster:
        fMasterSocketPool.erase(clientId);
        break;
    }
  }

  int getRedirectSocket(const std::string &clientId, ConnectionType connectionType) {
    std::map<std::string, int> *map;
    switch (connectionType) {
      case ConnectionType::TypeSlave: {
        map = &fMasterSocketPool;
        break;
      }
      case ConnectionType::TypeMaster: {
        map = &fSlaveSocketPool;
        break;
      }
    }
    auto item = map->find(clientId);
    if (item == map->end()) return -1;
    return item->second;
  }

  bool connectMessageHandler(const std::string &client, int clientSock, const std::shared_ptr<Message> &parseResult, std::shared_ptr<ConnectionType> &connectionType) {
    auto connectMessage = parseResult->cast<ConnectMessage>();
    auto clientId = connectMessage.getConnectOptions().getClientId();
    switch (connectMessage.getConnectOptions().getConnectionType()) {
      case ConnectionType::TypeSlave:
        if (fSlaveSocketPool.find(clientId) != fSlaveSocketPool.end()) {
          DERROR("client %s requested slave connection for %s, but there is slave already");
          return false;
        }
        DINFO("client %s registered as slave, id %s", client.c_str(), clientId.c_str());
        fSlaveSocketPool[clientId] = clientSock;
        break;
      case ConnectionType::TypeMaster:
        if (fMasterSocketPool.find(clientId) != fMasterSocketPool.end()) {
          DERROR("client %s requested master connection for %s, but there is master already");
          return false;
        }
        DINFO("client %s registered as master, id %s", client.c_str(), clientId.c_str());
        fMasterSocketPool[clientId] = clientSock;
        break;
    }
    connectionType = std::make_shared<ConnectionType>(connectMessage.getConnectOptions().getConnectionType());
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