file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testcrypto ${SRCS})

TARGET_LINK_LIBRARIES(testcrypto
  terminus
  libgtest
  libgmock
  )

add_test(NAME testcrypto COMMAND testcrypto)