file(GLOB GLUTILS_SOURCES "*.cpp" "*.h")
add_library(GLUtils STATIC ${GLUTILS_SOURCES})

set_property(TARGET GLUtils PROPERTY CXX_STANDARD 20)

target_include_directories(GLUtils PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
)