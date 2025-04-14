file(GLOB STREAMING_POCO_SOURCES "*.cpp" "*.h")
add_library(StreamingPoco STATIC ${STREAMING_POCO_SOURCES})

set_property(TARGET StreamingPoco PROPERTY CXX_STANDARD 20)

find_package(Poco REQUIRED COMPONENTS Net Util Foundation)

target_link_libraries(StreamingPoco PUBLIC Poco::Net Poco::Util Poco::Foundation GLUtils)

target_include_directories(StreamingPoco PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${CMAKE_CURRENT_SOURCE_DIR}/../../GLUtils"
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
)