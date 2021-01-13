#include "gtest/gtest.h"
#include "crypto/CryptoInterface.h"


TEST(Md5Test, EncryptDecryptTest) {
  auto input = "md5 test";
  auto expected = "2e5f9458bcd27e3c2b5908af0b91551a";
  auto result = Crypto::MD5::encryptData(input);
  ASSERT_EQ(result, expected);
}

