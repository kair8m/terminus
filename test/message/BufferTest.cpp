#include "gtest/gtest.h"
#include "message/Buffer.h"

TEST(BufferTest, FieldTest) {
  Buffer data;
  uint8_t byte = 0x0A;
  uint16_t word = 0xABCD;
  uint32_t dword = 0xABCDABCD;
  uint64_t qword = 0xABCDABCDABCDABCD;
  size_t dataSize = sizeof(byte) + sizeof(word) + sizeof(dword) + sizeof(qword);
  data.append(byte);
  data.append(word);
  data.append(dword);
  data.append(qword);
  auto text = "hello world";
  data.append(text);
  auto result = data.getSize();
  auto expected = dataSize + strlen(text);
  ASSERT_EQ(result, expected);
  ASSERT_EQ(data.get<uint8_t>(), byte);
  ASSERT_EQ(data.get<uint16_t>(), word);
  ASSERT_EQ(data.get<uint32_t>(), dword);
  ASSERT_EQ(data.get<uint64_t>(), qword);
  auto testEmpty = data.get<char>(0);
  ASSERT_TRUE(testEmpty.empty());
  testEmpty = data.get<char>(strlen(text) + 1);
  ASSERT_TRUE(testEmpty.empty());
  auto resultVector = data.get<char>(strlen(text));
  auto resultString = std::string(resultVector.begin(), resultVector.end());
  ASSERT_EQ(resultString, text);
  testEmpty = data.get<char>(strlen(text));
  ASSERT_TRUE(testEmpty.empty());
  testEmpty = data.get<char>(1);
  ASSERT_TRUE(testEmpty.empty());
}

