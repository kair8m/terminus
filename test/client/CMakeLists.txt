file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testclient ${SRCS})

TARGET_LINK_LIBRARIES(testclient
  terminus
  libgtest
  libgmock
  )

add_test(NAME testclient COMMAND testclient)