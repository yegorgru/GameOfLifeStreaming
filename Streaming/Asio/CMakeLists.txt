file(GLOB STREAMING_ASIO_SOURCES "*.cpp" "*.h")
add_library(StreamingAsio STATIC ${STREAMING_ASIO_SOURCES})

set_property(TARGET StreamingAsio PROPERTY CXX_STANDARD 20)

target_include_directories(StreamingAsio PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${CMAKE_CURRENT_SOURCE_DIR}/../../GLUtils"
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
    "${BOOST_ROOT}"
)

target_link_directories(StreamingAsio PUBLIC "${BOOST_LIB_DIR}")