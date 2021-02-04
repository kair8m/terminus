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
  std::string output;
  EVP_CIPHER_CTX *ctx;

  std::string key = getAlignedString(userKey, 256);
  std::string iv = getAlignedString(userIv, 256);

  int len;

  int ciphertextLen;

  /* Create and initialise the context */
  if (!(ctx = EVP_CIPHER_CTX_new()))
    return {};

  /*
   * Initialise the encryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits
   */
  if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(key.c_str()),
                              reinterpret_cast<const unsigned char *>(iv.c_str())))
    return {};

  /*
   * Provide the message to be encrypted, and obtain the encrypted output.
   * EVP_EncryptUpdate can be called multiple times if necessary
   */
  auto *cipherText = new unsigned char[BUF_SIZE];
  memset(cipherText, 0, BUF_SIZE);
  if (1 !=
      EVP_EncryptUpdate(ctx, cipherText, &len, reinterpret_cast<const unsigned char *>(input.c_str()), input.size()))
    return {};
  ciphertextLen = len;

  /*
   * Finalise the encryption. Further ciphertext bytes may be written at
   * this stage.
   */
  if (1 != EVP_EncryptFinal_ex(ctx, cipherText + len, &len))
    return {};
  ciphertextLen += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  output = std::string(reinterpret_cast<const char *>(cipherText), ciphertextLen);

  delete[] cipherText;

  return output;
}


std::string Crypto::AES256::decryptData(const std::string &input, const std::string &userKey, const std::string &userIv) {
  EVP_CIPHER_CTX *ctx;

  std::string output;

  std::string key = getAlignedString(userKey, 256);
  std::string iv = getAlignedString(userIv, 256);

  int len;

  int plaintextLen;

  /* Create and initialise the context */
  if (!(ctx = EVP_CIPHER_CTX_new()))
    return {};

  /*
   * Initialise the decryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits
   */
  if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(key.c_str()),
                              reinterpret_cast<const unsigned char *>(iv.c_str())))
    return {};

  /*
   * Provide the message to be decrypted, and obtain the plaintext output.
   * EVP_DecryptUpdate can be called multiple times if necessary.
   */
  auto *plainText = new unsigned char[BUF_SIZE];
  if (1 !=
      EVP_DecryptUpdate(ctx, plainText, &len, reinterpret_cast<const unsigned char *>(input.c_str()), input.size()))
    return {};
  plaintextLen = len;

  /*
   * Finalise the decryption. Further plaintext bytes may be written at
   * this stage.
   */
  if (1 != EVP_DecryptFinal_ex(ctx, plainText + len, &len))
    return {};
  plaintextLen += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  output = std::string(reinterpret_cast<const char *>(plainText), plaintextLen);

  delete[] plainText;

  return output;
}
