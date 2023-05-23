cmake_minimum_required(VERSION 3.18)

# Project description
project(
    Formatter VERSION 0.1
    DESCRIPTION "FOXSI-4 telemetry formatter software"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(NLOHMANNJSON_ROOT /usr/local/include)
set(GOOGLETEST_ROOT /usr/local/include)

enable_testing()

# Add other sources and headers here:
set(
    SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Metronome.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TCPInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UDPInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Subsystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Commanding.cpp
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

# Build the target "forwarding_example" using source files
add_executable(forwarding_example ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/examples/forwarding/forwarding_example.cpp)
target_include_directories(
    forwarding_example
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "cli_example" using source files
add_executable(cli_example ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/examples/CLI/cli_example.cpp)
target_include_directories(
    cli_example
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "commanding_example" using source files
add_executable(commanding_example ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/examples/commanding/commanding_example.cpp)
target_include_directories(
    commanding_example
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "test_spw_crc" using source files
add_executable(test_spw_crc ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/test/spw_crc.cpp)
target_include_directories(
    test_spw_crc
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_test(test ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp)

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
    target_link_libraries(cli_example PRIVATE Threads::Threads)
    target_link_libraries(commanding_example PRIVATE Threads::Threads)
    target_link_libraries(test_spw_crc PRIVATE Threads::Threads)
    target_link_libraries(test PRIVATE Threads::Threads)
    
    # add_library(nlohmann_json INTERFACE)
    # add_library(nlohmann_json INTERFACE ${NLOHMANNJSON_ROOT})
    # target_link_libraries(
    #     lib
    #     PRIVATE
    #     nlohmann_json
    # )
    # target_link_libraries(lib PUBLIC nlohmann_json)

    target_include_directories(
        lib
        PUBLIC
        ${NLOHMANNJSON_ROOT}
    )
    target_include_directories(
        lib
        PUBLIC
        ${GOOGLETEST_ROOT}
    )

elseif(UNIX AND APPLE)
    # this usually builds fine
    find_package(nlohmann_json 3.11.2 REQUIRED)
    target_link_libraries(lib PUBLIC nlohmann_json::nlohmann_json)

    find_package(GTest REQUIRED)
    target_link_libraries(lib PUBLIC GTest::gtest)

else()
    error("Unsupported OS.")
endif()

# message(STATUS "CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

# JSON library

# target_link_libraries(lib PRIVATE nlohmann_json::nlohmann_json)

# Boost library
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.70 REQUIRED COMPONENTS filesystem program_options)
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

    # link the whole user code library to Boost and JSON
    target_link_libraries(lib PUBLIC Boost::filesystem Boost::program_options)

    # then link it all to the executables
    target_link_libraries(formatter PUBLIC Boost::filesystem lib)
    target_link_libraries(tcp_example PUBLIC Boost::filesystem lib)
    target_link_libraries(forwarding_example PUBLIC Boost::filesystem lib)
    target_link_libraries(cli_example PUBLIC Boost::filesystem lib)
    target_link_libraries(commanding_example PUBLIC Boost::filesystem lib)
    target_link_libraries(test_spw_crc PUBLIC Boost::filesystem lib)
elseif(NOT Boost_FOUND)
    error("Boost not found.")
endif()
