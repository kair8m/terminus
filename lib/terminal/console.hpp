#ifndef TERMINUS_CONSOLE_HPP
#define TERMINUS_CONSOLE_HPP

#include <pty.h>
#include <fcntl.h>
#include <thread>
#include <functional>
#include <armadillo>

class Console {
public:
  using InputHandler = std::function<void(const std::string &)>;
private:
  const int RECV_BUF_SIZE = 4096;
private:
  termios fSave = {};
  termios fWindow = {};
  InputHandler fInputHandler = nullptr;
  bool active = true;
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

  bool setup() {
    if (tcgetattr(STDIN_FILENO, &fWindow) == -1)
      return false;
    fSave = fWindow;
    //fWindow.c_lflag &= ~(ECHO | ICANON);
    fWindow.c_iflag &= ~(ICRNL | IXON);
    //fWindow.c_oflag &= ~(OPOST);
    fWindow.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &fWindow) == -1)
      return false;
    std::thread(&Console::recvThread, this).detach();
    std::thread(&Console::windowHandlerThread, this).detach();
    return true;
  }

  void setupInputHandler(const InputHandler &inputHandler) {
    fInputHandler = inputHandler;
  }

  void display(const std::string &data) {
    (void) fSave;
    ::write(STDOUT_FILENO, data.c_str(), data.size());
  }

  void write(const std::string &data) {
    (void) fSave;
    ::write(STDIN_FILENO, data.c_str(), data.size());
  }

private:

  void recvThread() {
    auto buf = new char[RECV_BUF_SIZE];
    int recvSize = 0;
    while (active) {
      recvSize = ::read(STDIN_FILENO, buf, RECV_BUF_SIZE);
      if (recvSize <= 0)
        continue;
      std::string data(buf, recvSize);
      onInputData(data);
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
        onWindowChange(size.ws_col, size.ws_row);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

protected:

  virtual void onInputData(const std::string &data) {
    (void) data;
  }

  virtual void onWindowChange(int width, int height) {
    (void) width;
    (void) height;
  }
};

#endif //TERMINUS_CONSOLE_HPP
