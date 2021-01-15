#ifndef TERMINUS_MESSAGE_H
#define TERMINUS_MESSAGE_H

#include "Buffer.h"

class Message {
public:
  using Ptr = std::shared_ptr<Message>;
public:
  Message() = default;

  virtual ~Message() = default;

  const Buffer &getBuffer() const {
    return fBuffer;
  }

  virtual uint32_t getId() const = 0;

  template<class DerivedMessage>
  DerivedMessage cast() {
    static_assert(std::is_base_of<Message, DerivedMessage>::value, "DerivedMessage is not Message's child!");
    fBuffer.reset();
    return static_cast<DerivedMessage>(*this);
  }

protected:
  Buffer fBuffer;
};


#endif //TERMINUS_MESSAGE_H
