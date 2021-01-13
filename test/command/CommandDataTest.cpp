#include "gtest/gtest.h"
#include "command/CommandData.h"

TEST(CommandDataTest, FieldTest) {
  CommandData data;
  uint8_t byte = 1;
  uint16_t word = 2;
  uint32_t dword = 3;
  uint64_t qword = 4;
  size_t dataSize = sizeof(byte) + sizeof(word) + sizeof(dword) + sizeof(qword);
  data.append(byte);
  data.append(word);
  data.append(dword);
  data.append(qword);
  auto text = "asdiaosidj";
  data.append(text);
  auto result = data.getSize();
  auto expected = dataSize + strlen(text);
  ASSERT_EQ(result, expected);
}

