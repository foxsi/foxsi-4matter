cmake_minimum_required(VERSION 3.18)

# Project description
project(
    Formatter VERSION 0.1
    DESCRIPTION "FOXSI-4 telemetry formatter software"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Add other sources and headers here:
set(
    SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Metronome.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Utilities.cpp)

# Build the target "formatter" using source files
add_executable(formatter ${SOURCES})
target_include_directories(
    formatter 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Force use pthreads (for Boost/asio)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(formatter PRIVATE Threads::Threads)

# message(STATUS "CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

# Boost library
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.50 REQUIRED COMPONENTS filesystem)
if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    
    # Check if Boost version newer than CMake, or if CMake is too old:
    if(NOT TARGET Boost::filesystem)
        add_library(Boost::filesystem IMPORTED INTERFACE)
        set_property(TARGET Boost::filesystem PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
        set_property(TARGET Boost::filesystem PROPERTY
            INTERFACE_LINK_LIBRARIES ${Boost_LIBRARIES})
    endif()

    target_link_libraries(formatter PUBLIC Boost::filesystem)
elseif(NOT Boost_FOUND)
    error("Boost not found.")
endif()
