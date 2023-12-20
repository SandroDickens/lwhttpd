cmake_minimum_required(VERSION 3.11)
project(lwhttpd)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

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

add_executable(${PROJECT_NAME} main.cpp utils/utils.cpp http/response.cpp config/http_config.cpp http/mime_types.cpp)
add_executable(demo.cgi cgi_demo.cpp)

include(FetchContent)
FetchContent_Declare(log4cpp GIT_REPOSITORY https://github.com/SandroDickens/log4cpp.git GIT_TAG origin/main)
FetchContent_MakeAvailable(log4cpp)
include_directories(${log4cpp_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} log4cpp)
message(STATUS "binary dir: ${log4cpp_BINARY_DIR}")
message(STATUS "source dir: ${log4cpp_SOURCE_DIR}")

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

set(LIBRESSL_ROOT_DIR "/usr/local/libressl")
find_package(LibreSSL REQUIRED)
if (LIBRESSL_FOUND)
    message(STATUS "libressl version: ${LIBRESSL_VERSION}")
    message(STATUS "libressl include dir: ${LIBRESSL_INCLUDE_DIR}")
    message(STATUS "libressl libraries: ${LIBRESSL_LIBRARIES}")
    message(STATUS "libressl crypto library: ${LIBRESSL_CRYPTO_LIBRARY}")
    message(STATUS "libressl ssl library: ${LIBRESSL_SSL_LIBRARY}")
    message(STATUS "libressl tls library: ${LIBRESSL_TLS_LIBRARY}")
    include_directories(${LIBRESSL_INCLUDE_DIR})
    link_directories(${LIBRESSL_ROOT_DIR}/lib)
    target_link_libraries(${PROJECT_NAME} ${LIBRESSL_TLS_LIBRARY})
endif ()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach (dir ${dirs})
    message(STATUS "include directory='${dir}'")
endforeach ()
