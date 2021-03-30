add_executable(terminus_client client.cpp)

# Specify here the libraries this program depends on
target_link_libraries(terminus_client terminus)

install(TARGETS terminus_client DESTINATION "/usr/local/bin")