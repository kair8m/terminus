#include "gtest/gtest.h"
#include "message/MessageParser.h"

TEST(MessageTest, ParserTest) {
  auto resizeTerminalMessage = MessageFactory::create<ResizeTerminalMessage>(10, 10);
  ASSERT_EQ(resizeTerminalMessage->getHeight(), 10);
  ASSERT_EQ(resizeTerminalMessage->getWidth(), 10);
  auto parseResult = MessageParser::parse(resizeTerminalMessage->getBuffer().getDataPtr(),
                                          resizeTerminalMessage->getBuffer().getSize());
  ASSERT_TRUE(parseResult != nullptr);
  ASSERT_EQ(parseResult->getId(), resizeTerminalMessage->getId());
  ASSERT_EQ(parseResult->cast<ResizeTerminalMessage>().getHeight(), resizeTerminalMessage->getHeight());
  ASSERT_EQ(parseResult->cast<ResizeTerminalMessage>().getWidth(), resizeTerminalMessage->getWidth());
  ASSERT_TRUE(MessageParser::parse(nullptr, 0) == nullptr);

  ConnectOptions connectOptions(ConnectionType::TypeSlave, true, 10);
  auto connectMessage = MessageFactory::create<ConnectMessage>(connectOptions);
  ASSERT_EQ(connectMessage->getConnectOptions().keepAliveInterval(), 10);
  ASSERT_EQ(connectMessage->getConnectOptions().keepAliveUsed(), true);
  ASSERT_EQ(connectMessage->getConnectOptions().getConnectionType(), ConnectionType::TypeSlave);
  parseResult = MessageParser::parse(connectMessage->getBuffer().getDataPtr(),
                                     connectMessage->getBuffer().getSize());
  ASSERT_TRUE(parseResult != nullptr);

  ASSERT_EQ(parseResult->cast<ConnectMessage>().getConnectOptions().keepAliveInterval(), 10);
  ASSERT_EQ(parseResult->cast<ConnectMessage>().getConnectOptions().keepAliveUsed(), true);
  ASSERT_EQ(parseResult->cast<ConnectMessage>().getConnectOptions().getConnectionType(), ConnectionType::TypeSlave);

  nlohmann::json testData;
  auto testToken = "testToken";
  testData["Token"]  = testToken;
  auto responseMessage = MessageFactory::create<ResponseMessage>(ResponseCode::ResponseOk, testData);
  ASSERT_EQ(responseMessage->getMetaData()["Token"], testToken);
  ASSERT_EQ(responseMessage->getResponseCode(), ResponseCode::ResponseOk);
  parseResult = MessageParser::parse(responseMessage->getBuffer().getDataPtr(),
                                     responseMessage->getBuffer().getSize());
  ASSERT_TRUE(parseResult != nullptr);
  ASSERT_EQ(parseResult->cast<ResponseMessage>().getMetaData()["Token"], testToken);
  ASSERT_EQ(parseResult->cast<ResponseMessage>().getResponseCode(), ResponseCode::ResponseOk);

  std::string testChars = "12345789";
  auto putCharMessage = MessageFactory::create<PutCharMessage>(testChars);
  ASSERT_EQ(putCharMessage->getChars(), testChars);
  parseResult = MessageParser::parse(putCharMessage->getBuffer().getDataPtr(),
                                     putCharMessage->getBuffer().getSize());
  ASSERT_EQ(parseResult->cast<PutCharMessage>().getChars(), testChars);
}


