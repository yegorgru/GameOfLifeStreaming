file(GLOB CLIENT_SOURCES "*.cpp" "*.h")
add_executable(ClientStreaming ${CLIENT_SOURCES})

set_property(TARGET ClientStreaming PROPERTY CXX_STANDARD 20)

target_link_directories(ClientStreaming PUBLIC "${RAYLIB_LIB_DIR}")

target_link_libraries(ClientStreaming PRIVATE 
    raylib
    Streaming 
)

if(WIN32)
    target_link_libraries(ClientStreaming PRIVATE winmm)
endif()

target_include_directories(ClientStreaming PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${CMAKE_SOURCE_DIR}/GLUtils"
    "${CMAKE_SOURCE_DIR}/../Streaming"
    "${RAYLIB_INCLUDE_DIR}"
    "${BOOST_ROOT}"
)

target_link_directories(ClientStreaming PUBLIC "${BOOST_LIB_DIR}")

if(MSVC)
    target_compile_options(ClientStreaming PRIVATE /W4)
else()
    target_compile_options(ClientStreaming PRIVATE -Wall -Wextra -Wpedantic)
endif()
