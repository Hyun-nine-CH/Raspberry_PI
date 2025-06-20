cmake_minimum_required(VERSION 3.10)
project(multiP_basedChat)

set(CMAKE_C_STANDARD 99)

include_directories(header)

add_executable(server server/server.c)
add_executable(client_1 clients/client_1.c)
# add_executable(client_2 clients/client_2.c)
# add_executable(client_3 clients/client_3.c)
# add_executable(client_4 clients/client_4.c)
# add_executable(client_5 clients/client_5.c)
