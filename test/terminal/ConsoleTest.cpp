#include "gtest/gtest.h"
#include "terminal/console.hpp"

class TestApplication : public Console {
public:
  TestApplication() = default;

private:
  void onInputData(const std::string &data) override {
    display(data);
  }

  void onWindowChange(int width, int height) override {
    display("size changed, width " + std::to_string(width) + " height " + std::to_string(width));
  }
};


TEST(ConsoleTest, InputTest) {
//  TestApplication testApplication;
//  ASSERT_TRUE(testApplication.setup());
//  testApplication.write("hello");
//  while(true) {
//    std::this_thread::sleep_for(std::chrono::milliseconds(1));
//  }
}