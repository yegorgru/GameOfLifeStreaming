file(GLOB STREAMING_SOURCES "*.cpp" "*.h")
add_library(Streaming STATIC ${STREAMING_SOURCES})

set_property(TARGET Streaming PROPERTY CXX_STANDARD 20)

if(USE_ASIO)
    message(STATUS "Using Boost.Asio implementation")
elseif(USE_BEAST)
    message(STATUS "Using Boost.Beast implementation")
elseif(USE_POCO)
    message(STATUS "Using POCO implementation")
else()
    message(FATAL_ERROR "No networking implementation selected. Please enable one of: USE_ASIO, USE_BEAST, or USE_POCO")
endif()

target_include_directories(Streaming PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}../GLUtils"
)

target_link_libraries(Streaming PUBLIC GLUtils)

if(USE_ASIO)
    add_subdirectory(Asio)
    target_compile_definitions(Streaming PUBLIC USE_ASIO)
    target_link_libraries(Streaming PRIVATE StreamingAsio)
elseif(USE_BEAST)
    add_subdirectory(Beast)
    target_compile_definitions(Streaming PUBLIC USE_BEAST)
    target_link_libraries(Streaming PRIVATE StreamingBeast)
elseif(USE_POCO)
    add_subdirectory(Poco)
    target_compile_definitions(Streaming PUBLIC USE_POCO)
    target_link_libraries(Streaming PRIVATE StreamingPoco)
endif()
