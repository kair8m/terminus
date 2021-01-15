#ifndef TERMINUS_RESPONSEMESSAGE_H
#define TERMINUS_RESPONSEMESSAGE_H

#include "Message.h"

#include <utility>
#include <nlohmann/json.hpp>

enum class ResponseCode : uint32_t {
  ResponseOk = 0x4DC280B5,
  ResponseErr = 0xBFDAC919,
};

class ResponseMessage : public Message {
public:
  using Ptr = std::shared_ptr<ResponseMessage>;
public:
  const static uint32_t id = 0x5B7CF880;
public:
  explicit ResponseMessage(ResponseCode code, const nlohmann::json &metaData = {}) : fCode(code),
                                                                                     fMetaData(metaData) {
    fBuffer.append(id);
    fBuffer.append((uint32_t) code);
    if (!metaData.is_structured())
      return;
    auto size = (uint16_t) metaData.dump().size();
    fBuffer.append(size);
    std::string chars = metaData.dump();
    fBuffer.append(chars);
  }

  explicit ResponseMessage(const Message &msg) : fCode(ResponseCode::ResponseErr), fMetaData({}) {
    if (msg.getBuffer().getSize() < 8)
      return;
    auto inputId = msg.getBuffer().get<uint32_t>();
    if (inputId != id)
      return;
    auto code = msg.getBuffer().get<uint32_t>();
    fCode = static_cast<ResponseCode>(code);
    if (msg.getBuffer().getSize() > 4) {
      auto size = msg.getBuffer().get<uint16_t>();
      auto dataVector = msg.getBuffer().get<char>(size);
      fMetaData = nlohmann::json::parse(std::string(dataVector.begin(), dataVector.end()));
    }
  }

  uint32_t getId() const override {
    return id;
  }

  const ResponseCode &getResponseCode() const {
    return fCode;
  }

  const nlohmann::json &getMetaData() const {
    return fMetaData;
  }

private:
  ResponseCode fCode;
  nlohmann::json fMetaData;
};


#endif //TERMINUS_RESPONSEMESSAGE_H
