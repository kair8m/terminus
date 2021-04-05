#include "CryptoInterface.h"
#include <openssl/aes.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <cstring>

static const int BUF_SIZE = 4096;

static std::string getAlignedString(const std::string &str, const size_t &desiredLen) {
  std::string output;
  output.assign(desiredLen, ' ');
  for (int i = 0; i < desiredLen; i++) {
    if (i <= str.size())
      output[i] = str[i];
    else
      output[i] = i;
  }
  return output;
}

std::string Crypto::AES256::encryptData(const std::string &input, const std::string &userKey, const std::string &userIv) {
  auto key = getAlignedString(userKey, 32);
  auto iv = getAlignedString(userIv, 16);
  auto size = input.length() * 10;
  auto buffer = new char [size];
  auto len = encrypt((uint8_t *)input.c_str(), input.length(), (uint8_t *)key.c_str(), (uint8_t *)iv.c_str(), (uint8_t*) buffer);
  if (len < 0) {
    delete[] buffer;
    return {};
  }
  std::string output(buffer, len);
  delete[] buffer;
  return output;
}


std::string Crypto::AES256::decryptData(const std::string &input, const std::string &userKey, const std::string &userIv) {
  auto key = getAlignedString(userKey, 32);
  auto iv = getAlignedString(userIv, 16);
  auto size = input.length() * 10;
  auto buffer = new char [size];
  auto len = decrypt((uint8_t *)input.c_str(), input.length(), (uint8_t *)key.c_str(), (uint8_t *)iv.c_str(), (uint8_t*) buffer);
  if (len < 0) {
    delete[] buffer;
    return {};
  }
  std::string output(buffer, len);
  delete[] buffer;
  return output;
}

int Crypto::AES256::encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext) {
  EVP_CIPHER_CTX *ctx;
  int len;
  int ciphertext_len;
  if (!(ctx = EVP_CIPHER_CTX_new()))
    return -1;

  if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv))
    return -1;

  if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    return -1;
  ciphertext_len = len;

  if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
    return -1;
  ciphertext_len += len;

  EVP_CIPHER_CTX_free(ctx);

  return ciphertext_len;
}

int Crypto::AES256::decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
  EVP_CIPHER_CTX *ctx;

  int len;

  int plaintext_len;

  if (!(ctx = EVP_CIPHER_CTX_new()))
    return -1;

  if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv))
    return -1;

  if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    return -1;
  plaintext_len = len;

  if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
    return -1;
  plaintext_len += len;

  EVP_CIPHER_CTX_free(ctx);

  return plaintext_len;
}
