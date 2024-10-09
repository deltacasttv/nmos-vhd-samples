cmake_minimum_required(VERSION 3.19)

if(TARGET nmos-cpp::nmos-cpp)
   link_libraries(nmos-cpp::nmos-cpp)
endif()

add_subdirectory(receiver)
