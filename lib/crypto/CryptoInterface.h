#ifndef TERMINUS_CRYPTOINTERFACE_H
#define TERMINUS_CRYPTOINTERFACE_H

#include <string>

namespace Crypto {
  const static char magicKey[] = "40bbca6b3421e7966ce00949ed8fccd58e5e36ce94d04fe8eeeb1f3706c4df30"; // MD5('KairMuldashev')
  const static char magicIv[] = "539c0972470ff80a630a68e29d404c75"; // MD5('Terminus')
  namespace MD5 {
    std::string encryptData(const std::string &input);
  }
  /**
   * @brief encrypt and decrypt data using AES256 algorithm
   * @note iv is optional
   * @warning undefined behaviour if key and iv are equal
   */
  class AES256 {
  public:
    static std::string encryptData(const std::string &input, const std::string &key, const std::string &iv);

    static std::string decryptData(const std::string &input, const std::string &key, const std::string &iv);

  private:
    static int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext);

    static int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext);
  };
}

#endif //TERMINUS_CRYPTOINTERFACE_H
