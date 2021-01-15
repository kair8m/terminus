#ifndef TERMINUS_MESSAGEFACTORY_H
#define TERMINUS_MESSAGEFACTORY_H

#include "Message.h"

class MessageFactory {
public:
  template<class MessageType, class ... Types>
  static std::shared_ptr<MessageType> create(Types ... args) {
    static_assert(std::is_base_of<Message, MessageType>::value, "MessageType is not Message's child!");
    return std::make_shared<MessageType>(std::forward<Types>(args) ...);
  }
};

#endif //TERMINUS_MESSAGEFACTORY_H
