#include "Buffer.h"
#include <map>

Buffer::Buffer() : fData() {
  fDataIterator = fData.end();
}

Buffer::Buffer(const uint8_t *data, size_t len) : Buffer() {
  fData = std::vector<Byte>(data, data + len);
  fDataIterator = fData.begin();
}

Buffer::~Buffer() = default;

const uint8_t *Buffer::getDataPtr() const {
  return fData.data();
}

size_t Buffer::getSize() const {
  return fData.size();
}

void Buffer::appendByte(Byte element) {
  fData.emplace_back(element);
  fDataIterator = fData.begin();
}

void Buffer::appendWord(Word element) {
  uint8_t bytes[] = {
    static_cast<uint8_t>((element >> 0) & 0xFF),
    static_cast<uint8_t>((element >> 8) & 0xFF)
  };
  appendByte(bytes[0]);
  appendByte(bytes[1]);
}

void Buffer::appendDword(Dword element) {
  Word words[] = {
    static_cast<uint16_t>((element >> 0) & 0xFFFF),
    static_cast<uint16_t>((element >> 16) & 0xFFFF)
  };
  appendWord(words[0]);
  appendWord(words[1]);
}

void Buffer::appendQword(Qword element) {
  Dword dwords[] = {
    static_cast<uint32_t>((element >> 0) & 0xFFFFFFFF),
    static_cast<uint32_t>((element >> 32) & 0xFFFFFFFF)
  };
  appendDword(dwords[0]);
  appendDword(dwords[1]);
}

void Buffer::appendByteArray(const ByteArray &element) {
  for (auto &byte : element) {
    appendByte(byte);
  }
}

void Buffer::appendWordArray(const WordArray &element) {
  for (auto &word : element) {
    appendWord(word);
  }
}

void Buffer::appendDwordArray(const DwordArray &element) {
  for (auto &dword : element) {
    appendQword(dword);
  }
}

void Buffer::appendQwordArray(const QwordArray &element) {
  for (auto &qword : element) {
    appendQword(qword);
  }
}

void Buffer::appendString(const std::string &str) {
  ByteArray byteArray;
  std::copy(str.c_str(), str.c_str() + str.length(), back_inserter(byteArray));
  return appendByteArray(byteArray);
}
