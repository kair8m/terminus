#ifndef TERMINUS_COMMAND_H
#define TERMINUS_COMMAND_H

#include "CommandData.h"

class Command {
public:
  inline std::shared_ptr<CommandData> &getData() const { return fComand; }

  Command() : fCommandData(nullptr) {}

  virtual ~Command() = delete;

protected:
  template<class T>
  inline void appendCommandData(const T &data) { return fCommandData->append(data); }

private:
  std::shared_ptr<CommandData> fCommandData;
};


#endif //TERMINUS_COMMAND_H
