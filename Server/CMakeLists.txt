file(GLOB SERVER_SOURCES "*.cpp" "*.h")
add_executable(Server ${SERVER_SOURCES})

set_property(TARGET Server PROPERTY CXX_STANDARD 20)

target_link_libraries(Server PRIVATE Streaming GLUtils)

target_include_directories(Server PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${CMAKE_SOURCE_DIR}/GLUtils"
    "${CMAKE_SOURCE_DIR}/../Streaming"
)

if(MSVC)
    target_compile_options(Server PRIVATE /W4)
else()
    target_compile_options(Server PRIVATE -Wall -Wextra -Wpedantic)
endif()
