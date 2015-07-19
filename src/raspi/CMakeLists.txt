cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(beehive-sensing)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

#target_compile_features(foobar PRIVATE cxx_regex)

find_package(Qt5Bluetooth)
find_package(Qt5Network)
find_package(Qt5SerialPort)
find_package(Threads REQUIRED)

add_executable(beehive_reader MAC_device_parser.cpp bluetooth_manager.cpp database_manager.cpp
	scheduler.cpp serial_communication.cpp bluetooth_manager.h database_manager.h
	scheduler.h serial_communication.h main.cpp)

target_link_libraries(beehive_reader Qt5::Bluetooth Qt5::Network Qt5::SerialPort ${CMAKE_THREAD_LIBS_INIT})

