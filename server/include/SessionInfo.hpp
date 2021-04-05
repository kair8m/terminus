#ifndef TERMINUS_SESSIONINFO_HPP
#define TERMINUS_SESSIONINFO_HPP

#include <string>
#include <mutex>
#include <map>

class SessionInfo {
private:
  const int SESSION_ID_LEN = 14;
private:
  struct SessionInfoInternal {
    std::string masterId;
    std::string slaveId;
    std::string sessionId;
  };
private:
  mutable std::mutex fMutex;
  SessionInfoInternal fSessionInfo;
public:
  SessionInfo(const std::string &masterId, const std::string &slaveId) {
    fSessionInfo.masterId = masterId;
    fSessionInfo.slaveId = slaveId;
    fSessionInfo.sessionId = getRandomSessionId(SESSION_ID_LEN);
  }

  const std::string &getMaster() const {
    std::lock_guard<std::mutex> lock(fMutex);
    return fSessionInfo.masterId;
  }

  const std::string &getSlave() const {
    std::lock_guard<std::mutex> lock(fMutex);
    return fSessionInfo.slaveId;
  }

  const std::string &getSessionId() {
    std::lock_guard<std::mutex> lock(fMutex);
    return fSessionInfo.sessionId;
  }

private:
  static std::string getRandomSessionId(int length) {
    std::string randomStr = {};
    if (randomStr.empty()) {
      auto randchar = []() -> char {
        const char charset[] =
          "0123456789"
          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
          "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
      };
      std::string str(length, 0);
      std::generate_n(str.begin(), length, randchar);
      randomStr = str;
    }
    return randomStr;
  }
};

#endif //TERMINUS_SESSIONINFO_HPP