#ifndef TERMINUS_MESSAGEPARSER_H
#define TERMINUS_MESSAGEPARSER_H

#include <utility>

#include "PutCharMessage.h"
#include "ResizeTerminalMessage.h"
#include "ResponseMessage.h"
#include "ConnectMessage.h"
#include "EncryptedMessage.h"
#include "MessageFactory.h"

class MessageParser {
public:
  using Ptr = std::shared_ptr<MessageParser>;
private:
  std::string fKey, fIv;
public:
  explicit MessageParser(std::string key, std::string iv) : fKey(std::move(key)), fIv(std::move(iv)) {
  }

  void setKey(const std::string &key) {
    fKey = key;
  }

  void setIv(const std::string &iv) {
    fIv = iv;
  }

  const std::string &getKey() const {
    return fKey;
  }

  const std::string &getIv(const std::string &iv) const {
    return fIv;
  }

  std::shared_ptr<Message> parse(const uint8_t *data, size_t len) const {
    Buffer buffer(data, len);
    auto id = buffer.get<uint32_t>();
    switch (id) {
      case ConnectMessage::id: {
        auto connectionType = buffer.get<uint32_t>();
        auto clientIdLen = buffer.get<uint32_t>();
        auto charVector = buffer.get<char>(clientIdLen);
        bool keepAliveUsed = buffer.get<uint8_t>();
        uint16_t keepAliveInterval = buffer.get<uint8_t>();
        auto connectOpts = ConnectOptions(static_cast<ConnectionType>(connectionType),
                                          std::string(charVector.begin(), charVector.end()),
                                          keepAliveUsed, keepAliveInterval);
        return MessageFactory::create<ConnectMessage>(connectOpts);
      }
      case PutCharMessage::id: {
        auto size = buffer.get<uint32_t>();
        auto charVector = buffer.get<char>(size);
        std::string chars(charVector.begin(), charVector.end());
        return MessageFactory::create<PutCharMessage>(chars);
      }
      case ResizeTerminalMessage::id: {
        auto width = buffer.get<uint32_t>();
        auto height = buffer.get<uint32_t>();
        return MessageFactory::create<ResizeTerminalMessage>(width, height);
      }
      case ResponseMessage::id: {
        auto code = buffer.get<uint32_t>();
        auto size = buffer.get<uint16_t>();
        auto charVector = buffer.get<char>(size);
        std::string chars(charVector.begin(), charVector.end());
        nlohmann::json metaData = nlohmann::json::parse(chars);
        return MessageFactory::create<ResponseMessage>(static_cast<ResponseCode>(code), metaData);
      }
      case EncryptedMessage::id: {
        auto size = buffer.get<uint16_t>();
        auto charVector = buffer.get<char>(size);
        std::string chars = Crypto::AES256::decryptData(charVector.data(), fKey, fIv);
        return parse((const uint8_t *) (chars.data()), chars.size());
      }
      default:
        return nullptr;
    }
  }
};


#endif //TERMINUS_COMMANDPARSER_H
