file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testterminal ${SRCS})

TARGET_LINK_LIBRARIES(testterminal
  terminus
  libgtest
  libgmock
  )

add_test(NAME testterminal COMMAND testterminal)