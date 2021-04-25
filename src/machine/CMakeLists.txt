add_library(machine STATIC machine.cpp)

target_link_libraries(machine blit cia cpu mmu sids timer vicv)
