add_executable(terminus_client client.cpp)
add_executable(terminus_server server.cpp)

# Specify here the libraries this program depends on
target_link_libraries(terminus_client terminus)
target_link_libraries(terminus_server terminus)

install(TARGETS terminus_client DESTINATION bin)
install(TARGETS terminus_server DESTINATION bin)