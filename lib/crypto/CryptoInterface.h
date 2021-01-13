#ifndef TERMINUS_CRYPTOINTERFACE_H
#define TERMINUS_CRYPTOINTERFACE_H

#include <string>

namespace Crypto {
  namespace MD5 {
    bool encryptData(const std::string& input);
    std::string decryptData(const std::string& input);
  }
  namespace AES256 {
    bool encryptData(const std::string& input);
    std::string decryptData(const std::string& input);
  }
}

#endif //TERMINUS_CRYPTOINTERFACE_H
