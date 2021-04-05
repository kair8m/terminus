add_executable(terminus_server server.cpp)

# Specify here the libraries this program depends on
target_link_libraries(terminus_server terminus)

include_directories(include)

install(TARGETS terminus_server DESTINATION "/usr/local/bin")