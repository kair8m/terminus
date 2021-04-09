#include "gtest/gtest.h"
#include "client/MessageClient.h"

bool gotData = false;

class TestClient : public MessageClient {
private:
  void onReceivedData(const Buffer &data) override {
    gotData = true;
  }
};

TEST(ClientTest, network) {
  TestClient messageClient;

  int pid = fork();

  if (pid == 0) {
    exit(system("nc -l 9000"));
  }

  ASSERT_TRUE(messageClient.connect("localhost", 9000));

  ASSERT_TRUE(messageClient.sendData("hello", strlen("hello")));

  kill(pid, SIGKILL);
}

