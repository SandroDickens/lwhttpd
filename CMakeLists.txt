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

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The default is the Release version" FORCE)
endif ()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "Debug version...")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
else ()
    message(STATUS "Release version...")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif ()


if (NOT (CMAKE_HOST_UNIX))
    message(FATAL_ERROR "Unsupported OS")
endif ()

add_executable(${PROJECT_NAME} main.cpp utils/utils.cpp http/response.cpp logger/logger.cpp config/http_config.cpp http/mime_types.cpp)
add_executable(demo.cgi cgi_demo.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
if (Threads_FOUND)
    if (CMAKE_USE_PTHREADS_INIT)
        target_link_libraries(${PROJECT_NAME} Threads::Threads)
    else ()
        message(FATAL_ERROR "pthread is required")
    endif ()
else ()
    message(FATAL_ERROR "pthread not found!")
endif ()

#set(Boost_VERBOSE ON)
find_package(Boost 1.75 REQUIRED COMPONENTS json)
if (Boost_FOUND)
    # message(STATUS "Boost_LIB_VERSION = ${Boost_VERSION}")
    # message(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
    # message(STATUS "Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}")
    # message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif ()

