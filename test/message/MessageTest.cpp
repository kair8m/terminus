#include "gtest/gtest.h"
#include "message/MessageParser.h"
#include "logger/Logger.h"

TEST(MessageTest, ParserTest) {
  //init variables
  Message::Ptr parseResult;
  EncryptedMessage::Ptr encryptedMessage;

  // init message parser
  std::string key = "1ZNDH6P00ABZJN";
  std::string iv = "dji-alpha";
  MessageParser messageParser(key, iv);

  //test resize terminal message
  auto resizeTerminalMessage = MessageFactory::create<ResizeTerminalMessage>(10, 10);
  ASSERT_EQ(resizeTerminalMessage->getHeight(), 10);
  ASSERT_EQ(resizeTerminalMessage->getWidth(), 10);
  parseResult = messageParser.parse(resizeTerminalMessage->getBuffer().getDataPtr(),
                                    resizeTerminalMessage->getBuffer().getSize());
  ASSERT_TRUE(parseResult != nullptr);
  ASSERT_EQ(parseResult->getId(), resizeTerminalMessage->getId());
  ASSERT_EQ(parseResult->cast<ResizeTerminalMessage>().getHeight(), resizeTerminalMessage->getHeight());
  ASSERT_EQ(parseResult->cast<ResizeTerminalMessage>().getWidth(), resizeTerminalMessage->getWidth());
  ASSERT_TRUE(messageParser.parse(nullptr, 0) == nullptr);
  encryptedMessage = MessageFactory::create<EncryptedMessage>(resizeTerminalMessage, key, iv);
  parseResult = messageParser.parse(encryptedMessage->getBuffer().getDataPtr(),
                                    encryptedMessage->getBuffer().getSize());
  ASSERT_TRUE(parseResult != nullptr);
  ASSERT_EQ(parseResult->getId(), resizeTerminalMessage->getId());
  ASSERT_EQ(parseResult->cast<ResizeTerminalMessage>().getHeight(), resizeTerminalMessage->getHeight());
  ASSERT_EQ(parseResult->cast<ResizeTerminalMessage>().getWidth(), resizeTerminalMessage->getWidth());

  key = "1ZNDH8500BM3KW";
  iv = "dji-gamma";
  messageParser.setKey(key);
  messageParser.setIv(iv);

  //test response message
  nlohmann::json testData;
  auto testToken = "testToken";
  testData["Token"] = testToken;
  auto responseMessage = MessageFactory::create<ResponseMessage>(ResponseCode::ResponseOk, testData);
  ASSERT_EQ(responseMessage->getMetaData()["Token"], testToken);
  ASSERT_EQ(responseMessage->getResponseCode(), ResponseCode::ResponseOk);
  parseResult = messageParser.parse(responseMessage->getBuffer().getDataPtr(),
                                    responseMessage->getBuffer().getSize());
  ASSERT_TRUE(parseResult != nullptr);
  ASSERT_EQ(parseResult->cast<ResponseMessage>().getMetaData()["Token"], testToken);
  ASSERT_EQ(parseResult->cast<ResponseMessage>().getResponseCode(), ResponseCode::ResponseOk);

  encryptedMessage = MessageFactory::create<EncryptedMessage>(responseMessage, key, iv);
  parseResult = messageParser.parse(encryptedMessage->getBuffer().getDataPtr(),
                                    encryptedMessage->getBuffer().getSize());
  ASSERT_TRUE(parseResult != nullptr);
  ASSERT_EQ(parseResult->cast<ResponseMessage>().getMetaData()["Token"], testToken);
  ASSERT_EQ(parseResult->cast<ResponseMessage>().getResponseCode(), ResponseCode::ResponseOk);


  key = "1ZNDH7H00A52Z8";
  iv = "dji-delta";
  messageParser.setKey(key);
  messageParser.setIv(iv);

  //test putchar message
  std::string testChars = "12345789";
  auto putCharMessage = MessageFactory::create<PutCharMessage>(testChars);
  ASSERT_EQ(putCharMessage->getChars(), testChars);
  parseResult = messageParser.parse(putCharMessage->getBuffer().getDataPtr(),
                                    putCharMessage->getBuffer().getSize());
  ASSERT_TRUE(parseResult != nullptr);
  ASSERT_EQ(parseResult->cast<PutCharMessage>().getChars(), testChars);

  encryptedMessage = MessageFactory::create<EncryptedMessage>(putCharMessage, key, iv);
  parseResult = messageParser.parse(encryptedMessage->getBuffer().getDataPtr(),
                                    encryptedMessage->getBuffer().getSize());
  ASSERT_TRUE(parseResult != nullptr);
  ASSERT_EQ(parseResult->cast<PutCharMessage>().getChars(), testChars);

  key = "1ZNDH9B00CD247";
  iv = "dji-epsilon";
  messageParser.setKey(key);
  messageParser.setIv(iv);

  //test connect message
  auto clientId = "test";
  ConnectOptions connectOptions(ConnectionType::TypeSlave, clientId, true, 10);
  auto connectMessage = MessageFactory::create<ConnectMessage>(connectOptions);
  ASSERT_EQ(connectMessage->getConnectOptions().keepAliveInterval(), 10);
  ASSERT_EQ(connectMessage->getConnectOptions().keepAliveUsed(), true);
  ASSERT_EQ(connectMessage->getConnectOptions().getConnectionType(), ConnectionType::TypeSlave);
  ASSERT_EQ(connectMessage->getConnectOptions().getClientId(), clientId);
  parseResult = messageParser.parse(connectMessage->getBuffer().getDataPtr(),
                                    connectMessage->getBuffer().getSize());
  ASSERT_TRUE(parseResult != nullptr);
  ASSERT_EQ(parseResult->cast<ConnectMessage>().getConnectOptions().keepAliveInterval(), 10);
  ASSERT_EQ(parseResult->cast<ConnectMessage>().getConnectOptions().keepAliveUsed(), true);
  ASSERT_EQ(parseResult->cast<ConnectMessage>().getConnectOptions().getConnectionType(), ConnectionType::TypeSlave);
  ASSERT_EQ(parseResult->cast<ConnectMessage>().getConnectOptions().getClientId(), clientId);
  encryptedMessage = MessageFactory::create<EncryptedMessage>(connectMessage, key, iv);
  parseResult = messageParser.parse(encryptedMessage->getBuffer().getDataPtr(),
                                    encryptedMessage->getBuffer().getSize());
  ASSERT_TRUE(parseResult != nullptr);
  ASSERT_EQ(parseResult->cast<ConnectMessage>().getConnectOptions().keepAliveInterval(), 10);
  ASSERT_EQ(parseResult->cast<ConnectMessage>().getConnectOptions().keepAliveUsed(), true);
  ASSERT_EQ(parseResult->cast<ConnectMessage>().getConnectOptions().getConnectionType(), ConnectionType::TypeSlave);
}


