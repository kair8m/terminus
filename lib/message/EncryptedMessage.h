#ifndef TERMINUS_ENCRYPTEDMESSAGE_H
#define TERMINUS_ENCRYPTEDMESSAGE_H

#include "Message.h"
#include "crypto/CryptoInterface.h"
#include "logger/Logger.h"

class EncryptedMessage : public Message {
public:
  const static uint32_t id = 0xC7A469E3;
public:
  explicit EncryptedMessage(const Message::Ptr &msg, const std::string &key, const std::string &iv) : Message() {
    fBuffer.append(id);
    std::string data((char *) msg->getBuffer().getDataPtr(), msg->getBuffer().getSize());
    auto encryptedData = Crypto::AES256::encryptData(data, key, iv);
    fBuffer.append((uint16_t) encryptedData.size());
    fBuffer.append(encryptedData);
  }

  uint32_t getId() const override {
    return id;
  }

private:

};


#endif //TERMINUS_ENCRYPTEDMESSAGE_H
