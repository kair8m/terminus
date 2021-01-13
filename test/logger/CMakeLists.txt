file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testlogger ${SRCS})

TARGET_LINK_LIBRARIES(testlogger
  terminus
  libgtest
  libgmock
  )

add_test(NAME testlogger COMMAND testlogger)