cmake_minimum_required(VERSION 3.26.0)
project(LEDitor)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/source)
set(SOURCE
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/winapishenanigans.cpp
    ${SOURCE_DIR}/display.cpp
    ${SOURCE_DIR}/text.cpp
    ${SOURCE_DIR}/uiface.cpp
    ${SOURCE_DIR}/bitmap.cpp
    ${SOURCE_DIR}/serialize.cpp
)

add_executable(LEDitor WIN32 ${SOURCE})
set_target_properties(LEDitor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fuse-ld=lld)

target_include_directories(LEDitor PUBLIC ${CMAKE_SOURCE_DIR}/external/freetype/include)
target_link_directories(LEDitor PUBLIC ${CMAKE_SOURCE_DIR}/external/freetype/lib)

target_link_options(LEDitor PUBLIC -static -static-libstdc++ -lpthread)
target_link_libraries(LEDitor opengl32 dwmapi freetype)