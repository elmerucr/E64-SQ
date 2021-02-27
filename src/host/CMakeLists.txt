find_package(sdl2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_library(host STATIC host.cpp settings.cpp sdl2.cpp stats.cpp video.cpp)

target_link_libraries(host ${SDL2_LIBRARIES})
