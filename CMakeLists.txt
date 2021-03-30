cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3")

project(E64)

find_package(sdl2 REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    src/
    src/host/
    src/kernel/
    src/kernel/lua-5.4.2/src/
    src/components/
    src/components/blitter/
    src/components/cia/
    src/components/cpu/
    src/components/mmu/
    src/components/sids/
    src/components/sids/resid-0.16/
    src/components/timer/
    src/components/vicv/
    src/machine/
    src/rom/
)

add_subdirectory(src/)

add_executable(E64 src/main.cpp)

target_link_libraries(E64 host kernel lua machine rom ${SDL2_LIBRARIES})
