cmake_minimum_required (VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(GameOfLifeStreaming VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(EXISTS "${CMAKE_SOURCE_DIR}/config.cmake")
    include("${CMAKE_SOURCE_DIR}/config.cmake")
else()
    message(WARNING "Config file not found. Please create config.cmake for custom settings.")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(USE_ASIO "Use Boost.Asio implementation" OFF)
option(USE_BEAST "Use Boost.Beast implementation" ON)
option(USE_POCO "Use POCO implementation" OFF)

add_subdirectory(GLUtils)
add_subdirectory(Streaming)
add_subdirectory(Server)
add_subdirectory(Client)
