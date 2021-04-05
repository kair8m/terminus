#ifndef TERMINUS_CONNECTMESSAGE_H
#define TERMINUS_CONNECTMESSAGE_H

#include "Message.h"

#include <utility>

enum class ConnectionType : uint32_t {
  TypeSlave = 0xBA186B22,
  TypeMaster = 0x8DAE13DF
};

class ConnectOptions {
public:
  const static uint16_t defaultKeepAliveInterval = 5;
public:
  explicit ConnectOptions(ConnectionType connectionType, std::string clientId, bool useKeepAlive = false,
                          uint16_t keepAliveInterval = defaultKeepAliveInterval) :
    fConnectionType(connectionType), fUseKeepAlive(useKeepAlive), fKeepAliveInterval(keepAliveInterval), fClientId(std::move(clientId)) {
  }

  ConnectionType &getConnectionType() const {
    return fConnectionType;
  }

  void setConnectionType(ConnectionType type) {
    fConnectionType = type;
  }

  bool &keepAliveUsed() const {
    return fUseKeepAlive;
  }

  const std::string &getClientId() const {
    return fClientId;
  }

  uint16_t &keepAliveInterval() const {
    return fKeepAliveInterval;
  }

  void useKeepAlive(uint16_t keepAliveInterval) {
    fUseKeepAlive = true;
    fKeepAliveInterval = keepAliveInterval;
  }

private:
  mutable bool fUseKeepAlive;
  mutable uint16_t fKeepAliveInterval;
  mutable ConnectionType fConnectionType;
  std::string fClientId;
};

class ConnectMessage : public Message {
public:
  using Ptr = std::shared_ptr<ConnectMessage>;
public:
  const static uint32_t id = 0x6E4DC60B;
public:
  explicit ConnectMessage(const ConnectOptions &connectOptions) :
    Message(), fConnectOptions(std::make_shared<ConnectOptions>(connectOptions)) {
    fBuffer.append(id);
    fBuffer.append((uint32_t) fConnectOptions->getConnectionType());
    fBuffer.append((uint32_t) fConnectOptions->getClientId().length());
    fBuffer.append(fConnectOptions->getClientId());
    fBuffer.append((uint8_t) fConnectOptions->keepAliveUsed());
    fBuffer.append((uint16_t) fConnectOptions->keepAliveInterval());
  }

  explicit ConnectMessage(const Message &msg) {
    if (msg.getBuffer().getSize() < 13)
      return;
    auto inputId = msg.getBuffer().get<uint32_t>();
    if (id != inputId)
      return;
    auto connectionType = msg.getBuffer().get<uint32_t>();
    auto clientIdLen = msg.getBuffer().get<uint32_t>();
    auto charVector = msg.getBuffer().get<char>(clientIdLen);
    fConnectOptions = std::make_shared<ConnectOptions>(
      static_cast<ConnectionType>(connectionType),
      std::string(charVector.begin(), charVector.end()));
    if (msg.getBuffer().get<uint8_t>())
      fConnectOptions->useKeepAlive(msg.getBuffer().get<uint16_t>());
  }

  uint32_t getId() const override {
    return id;
  }

  const ConnectOptions &getConnectOptions() const {
    return *fConnectOptions;
  }

private:
  mutable std::shared_ptr<ConnectOptions> fConnectOptions;
};

#endif //TERMINUS_CONNECTCOMMAND_H