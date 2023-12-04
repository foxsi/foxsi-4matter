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

add_compile_options("$<$<CONFIG:Debug>:-Og>")

# Build library targets "lib..." to link to multiple executables
add_library(
    libcom 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Commanding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Systems.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DirectMemoryAccess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DataLinkLayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LineInterface.cpp
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

add_executable(shutdown ${CMAKE_CURRENT_SOURCE_DIR}/apps/shutdown_systems.cpp)
target_include_directories(
    shutdown
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(log ${CMAKE_CURRENT_SOURCE_DIR}/apps/log.cpp)
target_include_directories(
    log
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(mock ${CMAKE_CURRENT_SOURCE_DIR}/apps/mock.cpp)
target_include_directories(
    mock
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# # Build the target "forwarding_example" using source files
# add_executable(forwarding_example ${CMAKE_CURRENT_SOURCE_DIR}/examples/forwarding/forwarding_example.cpp)
# target_include_directories(
#     forwarding_example
#     PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

# # Build the target "cli_example" using source files
# add_executable(cli_example ${CMAKE_CURRENT_SOURCE_DIR}/examples/CLI/cli_example.cpp)
# target_include_directories(
#     cli_example
#     PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

# # Build the target "commanding_example" using source files
# add_executable(commanding_example ${CMAKE_CURRENT_SOURCE_DIR}/examples/commanding/commanding_example.cpp)
# target_include_directories(
#     commanding_example
#     PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

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

# add_test(test ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
add_executable(test ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp)
target_include_directories(
    test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Check which OS we're using to use correct thread library
if(UNIX AND NOT APPLE)
    # Force use pthreads (for Boost/asio)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(libcom PRIVATE Threads::Threads)
    target_link_libraries(libsubsys PRIVATE Threads::Threads)
    target_link_libraries(libutil PRIVATE Threads::Threads)
    target_link_libraries(libgse PRIVATE Threads::Threads)
    target_link_libraries(formatter PRIVATE Threads::Threads)
    target_link_libraries(shutdown PRIVATE Threads::Threads)
    target_link_libraries(log PRIVATE Threads::Threads)
    target_link_libraries(mock PRIVATE Threads::Threads)
    # target_link_libraries(forwarding_example PRIVATE Threads::Threads)
    # target_link_libraries(cli_example PRIVATE Threads::Threads)
    # target_link_libraries(commanding_example PRIVATE Threads::Threads)
    target_link_libraries(test_spw_crc PRIVATE Threads::Threads)
    target_link_libraries(test_spw_ping PRIVATE Threads::Threads)
    target_link_libraries(test_loop PRIVATE Threads::Threads)
    target_link_libraries(test_buffers PRIVATE Threads::Threads)
    target_link_libraries(test PRIVATE Threads::Threads)

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

    # link the user libraries to Boost
    target_link_libraries(libcom PUBLIC Boost::filesystem Boost::program_options)
    target_link_libraries(libsubsys PUBLIC Boost::filesystem Boost::program_options)
    target_link_libraries(libutil PUBLIC Boost::filesystem Boost::program_options)
    target_link_libraries(libgse PUBLIC Boost::filesystem Boost::program_options)
    # target_link_libraries(libutil PRIVATE spdlog::spdlog_header_only)
   
    # then link them all to the executables
    target_link_libraries(formatter PUBLIC Boost::filesystem libcom libsubsys libutil)
    target_link_libraries(shutdown PUBLIC Boost::filesystem libcom libsubsys libutil)
    target_link_libraries(log PUBLIC Boost::filesystem libcom libsubsys libutil libgse)
    target_link_libraries(mock PUBLIC Boost::filesystem libcom libsubsys libutil libgse)
    # target_link_libraries(forwarding_example PUBLIC Boost::filesystem libsubsys libcom libutil)
    # target_link_libraries(cli_example PUBLIC Boost::filesystem libcom libsubsys libutil)
    # target_link_libraries(commanding_example PUBLIC Boost::filesystem libcom libsubsys libutil)
    target_link_libraries(test_spw_crc PUBLIC Boost::filesystem libcom libsubsys libutil)
    target_link_libraries(test_spw_ping PUBLIC Boost::filesystem libcom libsubsys libutil)
    target_link_libraries(test_loop PUBLIC Boost::filesystem libcom libsubsys libutil)
    target_link_libraries(test_buffers PUBLIC Boost::filesystem libcom libsubsys libutil)
    target_link_libraries(test PUBLIC Boost::filesystem libcom libsubsys libutil)
elseif(NOT Boost_FOUND)
    error("Boost not found.")
endif()

message(STATUS "Adding log/ folder")
execute_process(
    COMMAND bash -c "mkdir ../log"
    OUTPUT_VARIABLE outvar
)

