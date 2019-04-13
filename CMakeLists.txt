cmake_minimum_required(VERSION 3.7)

project(email)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES email.cpp)
add_library(${PROJECT_NAME} ${SOURCE_FILES})

option(ENABLE_ETEST "enable etest, default=OFF")
if(ENABLE_ETEST)
	add_executable(demo main.cpp)
  target_link_libraries(demo ${PROJECT_NAME})
endif()
