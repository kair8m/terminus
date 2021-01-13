#include "gtest/gtest.h"
#include "crypto/CryptoInterface.h"


TEST(AesTest, EncryptDecryptTest) {
  auto input = "aes256 encryt test";
  std::string key = "123124125";
  std::string iv = "123124252345";
  auto encrypted = Crypto::AES256::encryptData(input, key, iv);
  auto result = Crypto::AES256::decryptData(encrypted, key, iv);
  ASSERT_EQ(result, input);
  key = "asdfijapsodifjp";
  iv = "asjdfhaisuhpin";
  encrypted = Crypto::AES256::encryptData(input, key, iv);
  result = Crypto::AES256::decryptData(encrypted, key, iv);
  ASSERT_EQ(result, input);
  key = "asojfigsopnhodkfpgih";
  iv = "as65d4fa684fd6asd1f5as9d5f19";
  encrypted = Crypto::AES256::encryptData(input, key, iv);
  result = Crypto::AES256::decryptData(encrypted, key, iv);
  ASSERT_EQ(result, input);
  key = "as9df4a98d4fa6sd1fa62s1d9f4asdf";
  iv = "as98d7as95d1a6s2d1a6s5d4as8d9a8sd4";
  encrypted = Crypto::AES256::encryptData(input, key, iv);
  result = Crypto::AES256::decryptData(encrypted, key, iv);
  ASSERT_EQ(result, input);
  key = "as9df4a98d4fa6sd1fa62s1d9f4asdfas9df4a98d4fa6sd1fa62s1d9f4asdfas9df4a98d4fa6sd1fa62s1d9f4asdf";
  iv = "as98d7as95d1a6s2d1a6s5d4as8d9a8sd4as9df4a98d4fa6sd1fa62s1d9f4asdfas9df4a98d4fa6sd1fa62s1d9f4asdf";
  encrypted = Crypto::AES256::encryptData(input, key, iv);
  result = Crypto::AES256::decryptData(encrypted, key, iv);
  ASSERT_EQ(result, input);
}
