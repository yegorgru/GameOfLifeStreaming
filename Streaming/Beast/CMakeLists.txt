file(GLOB STREAMING_BEAST_SOURCES "*.cpp" "*.h")
add_library(StreamingBeast STATIC ${STREAMING_BEAST_SOURCES})

set_property(TARGET StreamingBeast PROPERTY CXX_STANDARD 20)

target_include_directories(StreamingBeast PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${CMAKE_CURRENT_SOURCE_DIR}/../../GLUtils"
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
    "${BOOST_ROOT}"
)

target_link_directories(StreamingBeast PUBLIC "${BOOST_LIB_DIR}")