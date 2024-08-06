cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/submodule/cmake")

include(DetectOs)
include(SetupEnvironment)

project(tasp-microservice LANGUAGES CXX)

include(SetupCompileOptions)
include(SetupHardening)

include_directories(include)

file(GLOB_RECURSE SOURCES
  ./src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

include(Version)

include(Dependency)

find_package(Threads REQUIRED)

tasp_check_modules(tasp-common)

pkg_check_modules(LIBEVENT REQUIRED libevent)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)


target_link_libraries(${PROJECT_NAME}
    PUBLIC
        jsoncpp
        ${TASP-COMMON_LDFLAGS}
    PRIVATE
        Threads::Threads
        event
)

include(SetupInstall)
