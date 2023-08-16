cmake_minimum_required(VERSION 3.10)
project(cvAutoTrack-ResourceBuild)

set(CMAKE_CXX_STANDARD 20)
# utf-8
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

set(OpenCV_DIR "../third_party/opencv-lite-shared-world/x64/vc17/lib")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.cpp  BlockMapResource.h) 
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# copy dll to exe folder
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "../../third_party/opencv-lite-shared-world/x64/vc17/bin/opencv_world480.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
else()
add_custom_command(TARGET cvAutoTrack-ResourceBuild POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "../../third_party/opencv-lite-shared-world/x64/vc17/bin/opencv_world480.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
endif()