add_library(hud STATIC hud.cpp)
add_library(lua STATIC
	lua-5.4.2/src/lapi.c
	lua-5.4.2/src/lauxlib.c
	lua-5.4.2/src/lbaselib.c
	lua-5.4.2/src/lcode.c
	lua-5.4.2/src/lcorolib.c
	lua-5.4.2/src/lctype.c
	lua-5.4.2/src/ldblib.c
	lua-5.4.2/src/ldebug.c
	lua-5.4.2/src/ldo.c
	lua-5.4.2/src/ldump.c
	lua-5.4.2/src/lfunc.c
	lua-5.4.2/src/lgc.c
	lua-5.4.2/src/linit.c
	lua-5.4.2/src/liolib.c
	lua-5.4.2/src/llex.c
	lua-5.4.2/src/lmathlib.c
	lua-5.4.2/src/lmem.c
	lua-5.4.2/src/loadlib.c
	lua-5.4.2/src/lobject.c
	lua-5.4.2/src/lopcodes.c
	lua-5.4.2/src/loslib.c
	lua-5.4.2/src/lparser.c
	lua-5.4.2/src/lstate.c
	lua-5.4.2/src/lstring.c
	lua-5.4.2/src/lstrlib.c
	lua-5.4.2/src/ltable.c
	lua-5.4.2/src/ltablib.c
	lua-5.4.2/src/ltm.c
	lua-5.4.2/src/lundump.c
	lua-5.4.2/src/lutf8lib.c
	lua-5.4.2/src/lvm.c
	lua-5.4.2/src/lzio.c
)

target_link_libraries(hud cpu)
