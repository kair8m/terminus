set(libterminus_PATCH 1)
set(libterminus_MAJOR 0)
set(libterminus_MINOR 1)
set(libterminus_VERSION ${libterminus_MAJOR}.${libterminus_MINOR}.${libterminus_PATCH})

set(libterminus_COMMAND_SOURCES
  command/Command.h
  command/CommandFactory.h
  command/CommandFactory.cpp
  )

set(libterminus_CRYPTO_SOURCES
  crypto/CryptoInterface.h
  crypto/AES256.cpp
  crypto/MD5.cpp
  )

set(libterminus_LOGGER_SOURCES
  logger/Logger.h
  logger/Logger.cpp
  )

set(libterminus_CLIENT_SOURCES
  client/Master.h
  client/Master.cpp
  client/Slave.h
  client/Slave.cpp
  )

set(libterminus_SERVER_SOURCES
  server/Bridge.h
  server/Bridge.cpp
  )

# Declare the library
add_library(terminus SHARED
  ${libterminus_COMMAND_SOURCES}
  ${libterminus_CRYPTO_SOURCES}
  ${libterminus_LOGGER_SOURCES}
  )

# Specify here the include directories exported
# by this library
target_include_directories(terminus PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

set_target_properties(terminus PROPERTIES VERSION ${libterminus_VERSION} SOVERSION ${libterminus_MAJOR})