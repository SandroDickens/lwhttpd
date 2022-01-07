cmake_minimum_required(VERSION 3.0)
project(lwhttpd)

set(CMAKE_C_STANDARD 20)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wold-style-cast -Wextra -Wshadow -Wpointer-arith -Wwrite-strings \
	-Woverloaded-virtual \
	-Wno-pessimizing-move \
	-Wno-uninitialized -Wno-unused-parameter -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable \
	-Wno-unused-value -march=native -pipe")

include_directories(${PROJECT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The default is the Release version" FORCE)
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	message(STATUS "Debug version...")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
else()
	message(STATUS "Release version...")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

if(CMAKE_HOST_WIN32)
	message(FATAL_ERROR "Windows is not support!")
	include_directories("D:/OpenCode/boost/include")
	link_directories("D:/OpenCode/boost/lib")
elseif(CMAKE_HOST_UNIX)
	message(STATUS "Host is Unix/Linux")
	include_directories("/usr/local/boost/include")
	link_directories("/usr/local/boost/lib")
else()
	message(FATAL_ERROR "Unsupported OS")
endif()

add_executable(${PROJECT_NAME} main.cpp misce.cpp response.cpp config/http_config.cpp config/http_config.h mimetype/MIMETypes.cpp mimetype/MIMETypes.h)
add_executable(demo.cgi cgi_demo.cpp)

target_link_libraries(${PROJECT_NAME} pthread boost_json)