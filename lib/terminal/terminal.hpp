#ifndef TERMINUS_TERMINAL_HPP
#define TERMINUS_TERMINAL_HPP

#include <thread>
#include <pty.h>
#include <fcntl.h>
#include <condition_variable>
#include <csignal>
#include <wait.h>

class Terminal {
public:
  using ReadHandler = std::function<void(const std::string &)>;
private:
  int fChildPid = -1;
  int fTerminalFd = -1;
  int fHeight;
  int fWidth;
  bool fReset = false;
  bool fAuthorize = false;
  ReadHandler fReadHandler = nullptr;
  const int READ_BUFFER_SIZE = 100000;
public:
  Terminal(int width, int height, bool withAuthentication) : fWidth(width), fHeight(height), fAuthorize(withAuthentication) {
    std::thread(&Terminal::readThread, this).detach();
  }

  ~Terminal() {
    fReadHandler = nullptr;
    fReset = true;
    close(fTerminalFd);
    kill(fChildPid, SIGKILL);
    int status;
    waitpid(fChildPid, &status, 0);
  }

  bool open() {
    winsize window = {};
    window.ws_col = fWidth;
    window.ws_row = fHeight;
    fChildPid = forkpty(&fTerminalFd, nullptr, nullptr, &window);
    if (fChildPid < 0)
      return false;
    if (fChildPid == 0) {
      tty();
      exit(EXIT_SUCCESS);
    }
    if (fcntl(fTerminalFd, F_SETFL, fcntl(fTerminalFd, F_GETFL) | O_NONBLOCK) < 0)
      return false;
    return true;
  }

  void subscribeDataFlow(const ReadHandler &readHandler) {
    fReadHandler = readHandler;
  }

  void write(const std::string &chars) const {
    if (fTerminalFd == -1)
      return;
    ::write(fTerminalFd, chars.c_str(), chars.size());
  }

  static std::pair<int, std::string> execute(const std::string &cmd) {
    int fd = -1;
    int childPid = forkpty(&fd, nullptr, nullptr, nullptr);
    if (childPid < 0)
      return {};
    if (childPid == 0) {
      exit(system(cmd.c_str()));
    }
    char buf[1024] = {0};
    int numBytes;
    std::string output;
    while ((numBytes = read(fd, buf, 1024))) {
      if (numBytes == -1) {
        if (errno == EAGAIN) {
          sleep(1);
          continue;
        }
        break;
      }
      output += std::string(buf, numBytes);
      memset(buf, 0, 1024);
    }

    int status;
    waitpid(childPid, &status, 0);
    close(fd);
    return std::make_pair(status, output);
  }

protected:

  virtual void onData(const std::string &data) {
    (void) data;
  }

private:

  [[noreturn]] void tty() const {
    static char termstr[] = "TERM=xterm";
    putenv(termstr);
    while (true) {
      if (fAuthorize)
        execl("/bin/login", "login", nullptr);
      else
        execl("/bin/bash", "", nullptr);
    }
  }

  void readThread() {
    while (fTerminalFd == -1)
      std::this_thread::sleep_for(std::chrono::microseconds(1));

    int recvSize = 0;
    auto recvBuf = new char[READ_BUFFER_SIZE];
    while (!fReset) {
      recvSize = read(fTerminalFd, recvBuf, READ_BUFFER_SIZE);
      if (recvSize <= 0)
        continue;
      std::string data(recvBuf, recvSize);
      if (fReadHandler)
        fReadHandler(data);
      onData(data);
    }
    delete[] recvBuf;
  }

};


#endif //TERMINUS_TERMINAL_HPP
