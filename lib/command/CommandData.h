#ifndef TERMINUS_COMMANDDATA_H
#define TERMINUS_COMMANDDATA_H

#include <memory>
#include <vector>
#include <cstdlib>
#include <cstring>

class CommandData {
private:
  using Byte = uint8_t;
  using Word = uint16_t;
  using Dword = uint32_t;
  using Qword = uint64_t;
  using ByteArray = std::vector<Byte>;
  using WordArray = std::vector<Word>;
  using DwordArray = std::vector<Dword>;
  using QwordArray = std::vector<Qword>;
public:
  CommandData();

  ~CommandData();

  size_t getSize() const;

  uint8_t *getDataPtr() const;

  template<typename T>
  T get() const {
    // check if data is empty
    if (fData.empty())
      return {};
    // check if requested element exceeds data buffer size
    if (sizeof(T) + fDataIterator > fData.end())
      return {};
    auto it = fDataIterator;
    fDataIterator += sizeof(T);
    T ret = {};
    auto ratio = sizeof(T) / sizeof(Byte);
    for (int i = 0, pos = 0; i < sizeof(T) / sizeof(Byte); i++, pos += sizeof(Byte) * 8) {
      ret |= (T) *it++ << pos;
    }
    return ret;
  }

  template<typename T>
  std::vector<T> get(uint16_t size) const {
    // check if data is empty
    if (fData.empty())
      return {};
    // check if requested element exceeds data buffer size
    if (sizeof(T) * size + fDataIterator > fData.end())
      return {};
    std::vector<T> ret = {};
    auto it = fDataIterator;
    fDataIterator += size;
    for (int i = 0; i < size; i++) {
      ret.emplace_back(*it++);
    }
    return ret;
  }

  template<typename T>
  void append(T data) {
    switch (sizeof(data)) {
      case sizeof(Byte):
        return appendByte(data);
      case sizeof(Word):
        return appendWord(data);
      case sizeof(Dword):
        return appendDword(data);
      case sizeof(Qword):
        return appendQword(data);
      default:
        return;
    }
  }

  void append(const char *str, int len = -1) {
    if (len < 0)
      len = strlen(str);
    return appendString(std::string(str, len));
  }

  void append(const std::string &chars) {
    return appendString(chars);
  }

  void append(const uint8_t *bytes, size_t numBytes) {
    ByteArray byteArray;
    byteArray.insert(byteArray.end(), &bytes[0], &bytes[numBytes]);
    return appendByteArray(byteArray);
  }

  void append(const uint16_t *words, size_t numWords) {
    WordArray wordArray;
    wordArray.insert(wordArray.end(), &words[0], &words[numWords]);
    return appendWordArray(wordArray);
  }

  void append(const uint32_t *words, size_t numDwords) {
    DwordArray dwordArray;
    dwordArray.insert(dwordArray.end(), &words[0], &words[numDwords]);
    return appendDwordArray(dwordArray);
  }

  void append(const uint64_t *words, size_t numQwords) {
    QwordArray qwordArray;
    qwordArray.insert(qwordArray.end(), &words[0], &words[numQwords]);
    return appendQwordArray(qwordArray);
  }

  void clear() {
    fData.clear();
  }

private:

  template<typename ReturnType, typename InputType>
  std::vector<ReturnType> splitElement(InputType in) {
    std::vector<ReturnType> retVector = {};
    if (sizeof(ReturnType) > sizeof(InputType))
      return {};
    auto ratio = sizeof(InputType) / sizeof(ReturnType);
    for (int i = 0, pos = 0; i < ratio; i++, pos += sizeof(ReturnType)) {
      ReturnType element = in >> pos;
      retVector.emplace_back(element);
    }
  }

  void appendByte(Byte element);

  void appendWord(Word element);

  void appendDword(Dword element);

  void appendQword(Qword element);

  void appendByteArray(const ByteArray &array);

  void appendWordArray(const WordArray &array);

  void appendDwordArray(const DwordArray &array);

  void appendQwordArray(const QwordArray &array);

  void appendString(const std::string &str);

private:
  mutable ByteArray fData;
  mutable ByteArray::iterator fDataIterator;
};


#endif //TERMINUS_COMMANDDATA_H
