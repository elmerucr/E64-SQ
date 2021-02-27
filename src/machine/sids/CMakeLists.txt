add_subdirectory(resid-0.16/)

add_library(sids STATIC sids.cpp)

target_link_libraries(sids resid)
