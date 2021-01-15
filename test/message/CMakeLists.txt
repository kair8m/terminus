file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testmessage ${SRCS})

TARGET_LINK_LIBRARIES(testmessage
  terminus
  libgtest
  libgmock
  )

add_test(NAME testmessage COMMAND testmessage)