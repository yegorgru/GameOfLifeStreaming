file(GLOB SERVER_SOURCES "*.cpp" "*.h")
add_executable(ServerStreaming ${SERVER_SOURCES})

set_property(TARGET ServerStreaming PROPERTY CXX_STANDARD 20)

target_link_libraries(ServerStreaming PRIVATE Streaming)

target_include_directories(ServerStreaming PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${CMAKE_SOURCE_DIR}/GLUtils"
    "${CMAKE_SOURCE_DIR}/../Streaming"
    "${BOOST_ROOT}"
)

target_link_directories(ServerStreaming PUBLIC "${BOOST_LIB_DIR}")

if(MSVC)
    target_compile_options(ServerStreaming PRIVATE /W4)
else()
    target_compile_options(ServerStreaming PRIVATE -Wall -Wextra -Wpedantic)
endif()
