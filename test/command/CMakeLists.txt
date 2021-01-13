file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testcommand ${SRCS})

TARGET_LINK_LIBRARIES(testcommand
  terminus
  libgtest
  libgmock
  )

add_test(NAME testcommand COMMAND testcommand)