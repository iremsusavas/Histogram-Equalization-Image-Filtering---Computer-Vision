cmake_minimum_required(VERSION 2.8)

project(Labaratory3)

find_package(OpenCV REQUIRED)

include_directories(include ${OpenCV_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp")

add_executable( ${PROJECT_NAME} ${SOURCES} )
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )
file(COPY data DESTINATION ${CMAKE_BINARY_DIR})