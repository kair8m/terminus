#ifndef TERMINUS_PUTCHARCOMMAND_H
#define TERMINUS_PUTCHARCOMMAND_H

#include "Message.h"

class PutCharMessage : public Message {
public:
  using Ptr = std::shared_ptr<PutCharMessage>;
public:
  const static uint32_t id = 0xB0E0A971;
public:
  explicit PutCharMessage(const std::string &chars) : Message(), fChars(chars) {
    fBuffer.append(id);
    fBuffer.append((uint32_t) chars.size());
    fBuffer.append(chars);
  }

  explicit PutCharMessage(const Message &msg) {
    if (msg.getBuffer().getSize() < 9)
      return;
    auto inputId = msg.getBuffer().get<uint32_t>();
    if (id != inputId)
      return;
    auto size = msg.getBuffer().get<uint32_t>();
    auto charVector = msg.getBuffer().get<char>(size);
    fChars = std::string(charVector.begin(), charVector.end());
  }

  uint32_t getId() const override {
    return id;
  }

  std::string &getChars() const {
    return fChars;
  }

private:
  mutable std::string fChars;
};

#endif //TERMINUS_PUTCHARCOMMAND_H
