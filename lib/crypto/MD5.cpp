#include "CryptoInterface.h"
#include <openssl/md5.h>
#include <iomanip>

std::string Crypto::MD5::encryptData(const std::string &input) {
  unsigned char result[MD5_DIGEST_LENGTH];
  ::MD5((unsigned char *) input.c_str(), input.size(), result);

  std::ostringstream sout;
  sout << std::hex << std::setfill('0');
  for (long long c: result) {
    sout << std::setw(2) << (long long) c;
  }
  return sout.str();
}
