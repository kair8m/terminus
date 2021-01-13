#include "gtest/gtest.h"
#include "crypto/CryptoInterface.h"

/**
 * http://www.md5.cz/
 */

TEST(Md5Test, EncryptDecryptTest) {
  auto input = "md5 test";
  auto expected = "2e5f9458bcd27e3c2b5908af0b91551a";
  auto result = Crypto::MD5::encryptData(input);
  ASSERT_EQ(result, expected);
  input = "hello world";
  expected = "5eb63bbbe01eeed093cb22bb8f5acdc3";
  result = Crypto::MD5::encryptData(input);
  ASSERT_EQ(result, expected);
  input = "md5 hash test";
  expected = "b3d0e082600f3e5df5736eec0aefbcc7";
  result = Crypto::MD5::encryptData(input);
  ASSERT_EQ(result, expected);
  input = "function md5()";
  expected = "b1131abe22277e066731346d53b953f8";
  result = Crypto::MD5::encryptData(input);
  ASSERT_EQ(result, expected);
}

