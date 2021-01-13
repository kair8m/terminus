#include "CommandData.h"
#include <map>

CommandData::CommandData() : fData() {
}

CommandData::~CommandData() = default;

uint8_t *CommandData::getPtr() const {
  return fData.data();
}

size_t CommandData::getSize() const {
  return fData.size();
}

void CommandData::appendByte(Byte element) {
  fData.emplace_back(element);
}

void CommandData::appendWord(Word element) {
  uint8_t bytes[] = {
    static_cast<uint8_t>((element >> 0) & 0xFF),
    static_cast<uint8_t>((element >> 8) & 0xFF)
  };
  appendByte(bytes[0]);
  appendByte(bytes[1]);
}

void CommandData::appendDword(Dword element) {
  Word words[] = {
    static_cast<uint16_t>((element >> 0) & 0xFFFF),
    static_cast<uint16_t>((element >> 16) & 0xFFFF)
  };
  appendWord(words[0]);
  appendWord(words[1]);
}

void CommandData::appendQword(Qword element) {
  Dword dwords[] = {
    static_cast<uint32_t>((element >> 0) & 0xFFFFFFFF),
    static_cast<uint32_t>((element >> 32) & 0xFFFFFFFF)
  };
  appendDword(dwords[0]);
  appendDword(dwords[1]);
}

void CommandData::appendByteArray(const ByteArray &element) {
  for (auto &byte : element) {
    appendByte(byte);
  }
}

void CommandData::appendWordArray(const WordArray &element) {
  for (auto &word : element) {
    appendWord(word);
  }
}

void CommandData::appendDwordArray(const DwordArray &element) {
  for (auto &dword : element) {
    appendQword(dword);
  }
}

void CommandData::appendQwordArray(const QwordArray &element) {
  for (auto &qword : element) {
    appendQword(qword);
  }
}

void CommandData::appendString(const std::string &str) {
  ByteArray byteArray;
  std::copy(str.c_str(), str.c_str()+str.length(), back_inserter(byteArray));
  return appendByteArray(byteArray);
}
