cmake_minimum_required(VERSION 3.10)
project(multiP_basedChat)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_C_EXTENSIONS OFF)

add_definitions(-D_GNU_SOURCE)
add_executable(server server.c)

#add_executable(client_1 clients/client_1.c)
#add_executable(client_2 clients/client_2.c)
#add_executable(client_3 clients/client_3.c)
#add_executable(client_4 clients/client_4.c)
#add_executable(client_5 clients/client_5.c)

foreach(i RANGE 1 3)
    add_executable(client_${i} clients/client_${i}.c)
#    target_link_libraries(client_${i} ncurses)
endforeach()
