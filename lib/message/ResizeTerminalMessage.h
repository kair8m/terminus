#ifndef TERMINUS_RESIZETERMINALMESSAGE_H
#define TERMINUS_RESIZETERMINALMESSAGE_H

#include "Message.h"

class ResizeTerminalMessage : public Message {
public:
  using Ptr = std::shared_ptr<ResizeTerminalMessage>;
  using ConstPtr = const std::shared_ptr<ResizeTerminalMessage>;
public:
  const static uint32_t id = 0xBA8A5A9;
public:
  ResizeTerminalMessage(size_t width, size_t height) :
    fWidth(width), fHeight(height) {
    fBuffer.append(id);
    fBuffer.append((uint32_t) width);
    fBuffer.append((uint32_t) height);
  }

  explicit ResizeTerminalMessage(const Message &msg) : fWidth(0), fHeight(0) {
    if (msg.getBuffer().getSize() < 12)
      return;
    auto inputId = msg.getBuffer().get<uint32_t>();
    if (id != inputId)
      return;
    fWidth = msg.getBuffer().get<uint32_t>();
    fHeight = msg.getBuffer().get<uint32_t>();
  }

  uint32_t getId() const override {
    return id;
  }

  size_t getWidth() const {
    return fWidth;
  }

  size_t getHeight() const {
    return fHeight;
  }

private:
  size_t fWidth, fHeight;

};


#endif //TERMINUS_RESIZETERMINALMESSAGE_H
