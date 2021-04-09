#ifndef TERMINUS_CONSOLE_HPP
#define TERMINUS_CONSOLE_HPP

#include <pty.h>
#include <fcntl.h>
#include <thread>
#include <functional>
#include <armadillo>
#include <condition_variable>

#include <message/Buffer.h>

class Console {
public:
  using InputHandler = std::function<void(const std::string &)>;
  using WindowSizeHandler = std::function<void(int, int)>;
private:
  const int RECV_BUF_SIZE = 4096;
private:
  termios fSave = {};
  termios fWindow = {};
  InputHandler fInputHandler = nullptr;
  WindowSizeHandler fWindowSizeHandler = nullptr;
  bool active = true;
  std::condition_variable fInputFlag;
public:
  Console() {
    tcgetattr(STDIN_FILENO, &fSave);
    tcgetattr(STDIN_FILENO, &fWindow);
  }

  ~Console() {
    active = true;
    fInputHandler = nullptr;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &fSave);
  }

  bool setup(bool async = true) {
    if (tcgetattr(STDIN_FILENO, &fWindow) == -1)
      return false;
    fSave = fWindow;
    //fWindow.c_lflag &= ~(ECHO | ICANON);
    fWindow.c_iflag &= ~(ICRNL | IXON);
    //fWindow.c_oflag &= ~(OPOST);
    fWindow.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &fWindow) == -1)
      return false;
    std::thread(&Console::windowHandlerThread, this).detach();
    if (!async) return true;
    std::thread(&Console::recvThread, this).detach();
    return true;
  }

  void setupInputHandler(const InputHandler &inputHandler) {
    fInputHandler = inputHandler;
  }

  template<class Base>
  void setupInputHandler(void(Base::*handler)(const std::string &), const Base *obj) {
    fInputHandler = std::bind(handler, obj, std::placeholders::_1);
  }

  template<class Base>
  void setupInputHandler(void(Base::*handler)(int, int), const Base *obj) {
    fWindowSizeHandler = std::bind(handler, obj, std::placeholders::_1);
  }

  void display(const std::string &data) {
    (void) fSave;
    ::write(STDOUT_FILENO, data.c_str(), data.size());
  }

  void write(const std::string &data) {
    (void) fSave;
    ::write(STDIN_FILENO, data.c_str(), data.size());
  }

  Buffer read() const {
    auto buf = new uint8_t[RECV_BUF_SIZE];
    auto recvSize = ::read(STDIN_FILENO, buf, RECV_BUF_SIZE);
    if (recvSize <= 0) {
      delete[] buf;
      return {};
    }
    Buffer data(buf, recvSize);
    delete[] buf;
    return data;
  }

  static winsize getCurrentWindowSize() {
    winsize size = {};
    ioctl(STDIN_FILENO, TIOCGWINSZ, &size);
    return size;
  }

private:

  void recvThread() {
    auto buf = new char[RECV_BUF_SIZE];
    size_t recvSize = 0;
    while (active) {
      recvSize = ::read(STDIN_FILENO, buf, RECV_BUF_SIZE);
      if (recvSize <= 0)
        continue;
      std::string data(buf, recvSize);
      if (fInputHandler)
        fInputHandler(data);
    }
    delete[] buf;
  }

  void windowHandlerThread() {
    winsize size = {};
    uint16_t oldRows = 0, oldCols = 0;
    while (active) {
      ioctl(STDIN_FILENO, TIOCGWINSZ, &size);
      if (oldRows != size.ws_row || oldCols != size.ws_col) {
        oldRows = size.ws_row;
        oldCols = size.ws_col;
        if (fWindowSizeHandler) fWindowSizeHandler(size.ws_col, size.ws_row);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
};

#endif //TERMINUS_CONSOLE_HPP
