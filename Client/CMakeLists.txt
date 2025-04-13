file(GLOB CLIENT_SOURCES "*.cpp" "*.h")
add_executable(Client ${CLIENT_SOURCES})

set_property(TARGET Client PROPERTY CXX_STANDARD 20)

target_link_directories(Client PUBLIC "${RAYLIB_LIB_DIR}")

target_link_libraries(Client PRIVATE 
    raylib
    Streaming 
    GLUtils
)

if(WIN32)
    target_link_libraries(Client PRIVATE winmm)
endif()

target_include_directories(Client PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${CMAKE_SOURCE_DIR}/GLUtils"
    "${CMAKE_SOURCE_DIR}/../Streaming"
    "${RAYLIB_INCLUDE_DIR}"
)

if(MSVC)
    target_compile_options(Client PRIVATE /W4)
else()
    target_compile_options(Client PRIVATE -Wall -Wextra -Wpedantic)
endif()
