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
set(spdlog_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/spdlog)
add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

# enable_testing()

# add_compile_options("$<$<CONFIG:Debug>:-Og>")

# Build library targets "lib..." to link to multiple executables
add_library(
    libcom 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Commanding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DirectMemoryAccess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DataLinkLayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LineInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UartInterface.cpp
)
add_library(
    libsubsys 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TransportLayer.cpp
)
add_library(
    libutil 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Utilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Buffers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Timing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RingBufferInterface.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Fragmenter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Circle.cpp
)
add_library(
    libgse
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Foxsimile.cpp
)

target_include_directories(
    libcom
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(
    libsubsys
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(
    libutil
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(
    libgse
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "formatter" using source files
add_executable(formatter ${CMAKE_CURRENT_SOURCE_DIR}/apps/main.cpp)
target_include_directories(
    formatter 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "shutdown" using source files
add_executable(shutdown ${CMAKE_CURRENT_SOURCE_DIR}/apps/shutdown_systems.cpp)
target_include_directories(
    shutdown
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "stop" using source files
add_executable(stop ${CMAKE_CURRENT_SOURCE_DIR}/apps/stop_systems.cpp)
target_include_directories(
    stop
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "log" using source files
add_executable(log ${CMAKE_CURRENT_SOURCE_DIR}/apps/log.cpp)
target_include_directories(
    log
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "foxsimile" using source files
add_executable(foxsimile ${CMAKE_CURRENT_SOURCE_DIR}/apps/foxsimile.cpp)
target_include_directories(
    foxsimile
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "test_spw_crc" using source files
add_executable(test_spw_crc ${CMAKE_CURRENT_SOURCE_DIR}/test/spw_crc.cpp)
target_include_directories(
    test_spw_crc
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "test_spw_ping" using source files
add_executable(test_spw_ping ${CMAKE_CURRENT_SOURCE_DIR}/test/test_spw_ping.cpp)
target_include_directories(
    test_spw_ping
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "test_spw_reply" using source files
add_executable(test_spw_reply ${CMAKE_CURRENT_SOURCE_DIR}/test/test_spw_reply.cpp)
target_include_directories(
    test_spw_reply
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "test_loop" using source files
add_executable(test_loop ${CMAKE_CURRENT_SOURCE_DIR}/test/test_loop.cpp)
target_include_directories(
    test_loop
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "test_buffer" using source files
add_executable(test_buffers ${CMAKE_CURRENT_SOURCE_DIR}/test/test_buffers.cpp)
target_include_directories(
    test_buffers
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the target "test_uart" using source files
add_executable(test_uart ${CMAKE_CURRENT_SOURCE_DIR}/test/test_uart.cpp)
target_include_directories(
    test_uart
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# add_test(test ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
add_executable(test ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp)
target_include_directories(
    test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Check which OS we're using to use correct thread library
if(UNIX AND NOT APPLE)
    # a.k.a Linux
    # Force use pthreads (for Boost/asio)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(libcom PRIVATE Threads::Threads)
    target_link_libraries(libsubsys PRIVATE Threads::Threads)
    target_link_libraries(libutil PRIVATE Threads::Threads)
    target_link_libraries(libgse PRIVATE Threads::Threads)
    target_link_libraries(formatter PRIVATE Threads::Threads)
    target_link_libraries(stop PRIVATE Threads::Threads)
    target_link_libraries(shutdown PRIVATE Threads::Threads)
    target_link_libraries(log PRIVATE Threads::Threads)
    target_link_libraries(foxsimile PRIVATE Threads::Threads)
    target_link_libraries(test_spw_crc PRIVATE Threads::Threads)
    target_link_libraries(test_spw_ping PRIVATE Threads::Threads)
    target_link_libraries(test_spw_reply PRIVATE Threads::Threads)
    target_link_libraries(test_loop PRIVATE Threads::Threads)
    target_link_libraries(test_buffers PRIVATE Threads::Threads)
    target_link_libraries(test PRIVATE Threads::Threads)
    target_link_libraries(test_uart PRIVATE Threads::Threads)

    target_include_directories(
        libcom
        PUBLIC
        ${NLOHMANNJSON_ROOT}
    )
    # target_include_directories(
    #     lib
    #     PUBLIC
    #     ${GOOGLETEST_ROOT}
    # )

elseif(UNIX AND APPLE)
    # this usually builds fine
    find_package(nlohmann_json 3.11.2 REQUIRED)
    target_link_libraries(libcom PUBLIC nlohmann_json::nlohmann_json)

    find_package(GTest REQUIRED)
    target_link_libraries(libcom PUBLIC GTest::gtest)

    find_package(Doxygen REQUIRED)

else()
    error("Unsupported OS.")
endif()

# Find and link Boost library
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.74 REQUIRED COMPONENTS program_options)
if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")

    # link the user libraries to Boost
    target_link_libraries(libcom            PUBLIC Boost::program_options)
    target_link_libraries(libsubsys         PUBLIC Boost::program_options)
    target_link_libraries(libutil           PUBLIC Boost::program_options)
    target_link_libraries(libgse            PUBLIC Boost::program_options)
   
    # then link them all to the executables
    target_link_libraries(formatter         PUBLIC libcom libsubsys libutil)
    target_link_libraries(stop              PUBLIC libcom libsubsys libutil)
    target_link_libraries(shutdown          PUBLIC libcom libsubsys libutil)
    target_link_libraries(log               PUBLIC libcom libsubsys libutil libgse)
    target_link_libraries(foxsimile         PUBLIC libcom libsubsys libutil libgse)
    target_link_libraries(test_spw_crc      PUBLIC libcom libsubsys libutil)
    target_link_libraries(test_spw_ping     PUBLIC libcom libsubsys libutil)
    target_link_libraries(test_spw_reply    PUBLIC libcom libsubsys libutil)
    target_link_libraries(test_loop         PUBLIC libcom libsubsys libutil)
    target_link_libraries(test_buffers      PUBLIC libcom libsubsys libutil)
    target_link_libraries(test              PUBLIC libcom libsubsys libutil)
    target_link_libraries(test_uart         PUBLIC libcom libsubsys libutil)
elseif(NOT Boost_FOUND)
    error("Boost not found.")
endif()

# create log/ folder if it doesn't exist.
message(STATUS "Adding log/ folder")
execute_process(
    COMMAND bash -c "mkdir ../log"
    OUTPUT_VARIABLE outvar
)

