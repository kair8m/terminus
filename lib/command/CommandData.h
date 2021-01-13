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
private:
  enum class StoredType {
    Byte,
    Word,
    Dword,
    Qword,
    ByteArray,
    WordArray,
    DwordArray,
    QwordArray,
    String,
  };
public:
  CommandData();

  ~CommandData();

  size_t getSize() const;

  uint8_t *getPtr() const;

  template<typename T>
  void append(T data) {
    switch (sizeof(data)) {
      case sizeof(Byte):
        fDataFields.emplace_back(StoredType::Byte);
        return appendByte(data);
      case sizeof(Word):
        fDataFields.emplace_back(StoredType::Word);
        return appendWord(data);
      case sizeof(Dword):
        fDataFields.emplace_back(StoredType::Dword);
        return appendDword(data);
      case sizeof(Qword):
        fDataFields.emplace_back(StoredType::Qword);
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
    fDataFields.emplace_back(StoredType::String);
    return appendString(chars);
  }

  void append(const uint8_t* bytes, size_t numBytes) {
    ByteArray byteArray;
    byteArray.insert(byteArray.end(), &bytes[0], &bytes[numBytes]);
    fDataFields.emplace_back(StoredType::WordArray);
    return appendByteArray(byteArray);
  }

  void append(const uint16_t* words, size_t numWords) {
    WordArray wordArray;
    wordArray.insert(wordArray.end(), &words[0], &words[numWords]);
    fDataFields.emplace_back(StoredType::WordArray);
    return appendWordArray(wordArray);
  }

  void append(const uint32_t* words, size_t numDwords) {
    DwordArray dwordArray;
    dwordArray.insert(dwordArray.end(), &words[0], &words[numDwords]);
    fDataFields.emplace_back(StoredType::DwordArray);
    return appendDwordArray(dwordArray);
  }

  void append(const uint64_t* words, size_t numQwords) {
    QwordArray qwordArray;
    qwordArray.insert(qwordArray.end(), &words[0], &words[numQwords]);
    fDataFields.emplace_back(StoredType::QwordArray);
    return appendQwordArray(qwordArray);
  }

  void clear() {
    fDataFields.clear();
    fData.clear();
  }

private:

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
  std::vector<StoredType> fDataFields;
};


#endif //TERMINUS_COMMANDDATA_H
