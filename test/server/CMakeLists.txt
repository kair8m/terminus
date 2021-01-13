file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testserver ${SRCS})

TARGET_LINK_LIBRARIES(testserver
  terminus
  libgtest
  libgmock
  )

add_test(NAME testserver COMMAND testserver)