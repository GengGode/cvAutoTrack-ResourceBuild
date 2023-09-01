cmake_minimum_required(VERSION 3.10)
project(cvAutoTrack-ResourceBuild)


if(WIN32) # Install dlls in the same directory as the executable on Windows
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

add_subdirectory(src)
add_subdirectory(src/get_item_json)