add_library(vicv STATIC vicv.cpp blitter.cpp)

target_link_libraries(vicv rom)
