# @see https://trenki2.github.io/blog/2017/06/02/using-sdl2-with-cmake/
set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

# 32 and 64-bit builds
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(SDL2_LIB_DIRS "${CMAKE_CURRENT_LIST_DIR}/lib/x64")
  set(SDL2_LIBRARIES "${SDL2_LIB_DIRS}/SDL2.lib;${SDL2_LIB_DIRS}/SDL2main.lib")
else ()
  set(SDL2_LIB_DIRS "${CMAKE_CURRENT_LIST_DIR}/lib/x86")
  set(SDL2_LIBRARIES "${SDL2_LIB_DIRS}/SDL2.lib;${SDL2_LIB_DIRS}/SDL2main.lib")
endif ()

# Strip leading/trailing spaces 
string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
