#ifndef TERMINUS_CRYPTOINTERFACE_H
#define TERMINUS_CRYPTOINTERFACE_H

#include <string>

namespace Crypto {
  namespace MD5 {
    std::string encryptData(const std::string &input);
  }
  /**
   * @brief encrypt and decrypt data using AES256 algorithm
   * @note iv is optional
   * @warning undefined behaviour if key and iv are equal
   */
  namespace AES256 {
    std::string encryptData(const std::string &input, const std::string &key, const std::string &iv);

    std::string decryptData(const std::string &input, const std::string &key, const std::string &iv);
  }
}

#endif //TERMINUS_CRYPTOINTERFACE_H
