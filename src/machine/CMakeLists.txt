add_subdirectory(cia/)
add_subdirectory(cpu/)
add_subdirectory(mmu/)
add_subdirectory(sids/)
add_subdirectory(timer/)
add_subdirectory(vicv/)

add_library(machine STATIC machine.cpp)

target_link_libraries(machine cia mmu sids timer vicv)
