cmake_minimum_required(VERSION 3.15)
project(get_item_json)

set(CMAKE_CXX_STANDARD 17)

add_executable(get_item_json main.cpp)

if(WIN32) # Install dlls in the same directory as the executable on Windows
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()
include_directories("../../third_party")


# cpr
include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400) 
FetchContent_MakeAvailable(cpr)
# spdlog
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git
                            GIT_TAG v1.12.0) 
FetchContent_MakeAvailable(spdlog)


# cpr
target_link_libraries(get_item_json PRIVATE cpr::cpr)
# spdlog
target_link_libraries(get_item_json PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
