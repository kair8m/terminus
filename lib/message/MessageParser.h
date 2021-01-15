#ifndef TERMINUS_COMMANDPARSER_H
#define TERMINUS_COMMANDPARSER_H

#include "PutCharMessage.h"
#include "ResizeTerminalMessage.h"
#include "ResponseMessage.h"
#include "ConnectMessage.h"
#include "MessageFactory.h"

class MessageParser {
public:
  using Ptr = std::shared_ptr<MessageParser>;
public:
  static std::shared_ptr<Message> parse(const uint8_t *data, size_t len) {
    Buffer buffer(data, len);
    auto id = buffer.get<uint32_t>();
    switch (id) {
      case ConnectMessage::id: {
        auto connectionType = buffer.get<uint32_t>();
        bool keepAliveUsed = buffer.get<uint8_t>();
        uint16_t keepAliveInterval = buffer.get<uint8_t>();
        auto connectOpts = ConnectOptions(static_cast<ConnectionType>(connectionType), keepAliveUsed, keepAliveInterval);
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
      default:
        return nullptr;
    }
  }
};


#endif //TERMINUS_COMMANDPARSER_H
