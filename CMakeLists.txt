cmake_minimum_required(VERSION 3.18)

# Project description
project(
    Formatter VERSION 0.1
    DESCRIPTION "FOXSI-4 telemetry formatter software"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Add other sources and headers here:
set(
    SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Metronome.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TCPInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UDPInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Subsystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LineInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Utilities.cpp
)

# Build the target "formatter" using source files
add_executable(formatter ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/apps/main.cpp)
target_include_directories(
    formatter 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "tcp_example" using source files
add_executable(tcp_example ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/examples/TCP/tcp_example.cpp)
target_include_directories(
    tcp_example
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(forwarding_example ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/examples/forwarding/forwarding_example.cpp)
target_include_directories(
    forwarding_example
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build library target "lib" to link to multiple executables
add_library(lib ${SOURCES})
target_include_directories(
    lib
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Check which OS we're using to use correct thread library
if(UNIX AND NOT APPLE)
    # Force use pthreads (for Boost/asio)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(lib PRIVATE Threads::Threads)
    target_link_libraries(formatter PRIVATE Threads::Threads)
    target_link_libraries(tcp_example PRIVATE Threads::Threads)
    target_link_libraries(forwarding_example PRIVATE Threads::Threads)
elseif(UNIX AND APPLE)
    # this usually builds fine
else()
    error("Unsupported OS.")
endif()

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

    target_link_libraries(lib PUBLIC Boost::filesystem)
    target_link_libraries(formatter PUBLIC Boost::filesystem lib)
    target_link_libraries(tcp_example PUBLIC Boost::filesystem lib)
    target_link_libraries(forwarding_example PUBLIC Boost::filesystem lib)
elseif(NOT Boost_FOUND)
    error("Boost not found.")
endif()
