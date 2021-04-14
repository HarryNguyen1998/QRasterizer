set(SDL2_IMG_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

# 32 and 64-bit builds
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(SDL2_IMG_LIB_DIRS "${CMAKE_CURRENT_LIST_DIR}/lib/x64")
  set(SDL2_IMG_LIBRARIES "${SDL2_IMG_LIB_DIRS}/SDL2_image.lib")
else ()
  set(SDL2_IMG_LIB_DIRS "${CMAKE_CURRENT_LIST_DIR}/lib/x86")
  set(SDL2_IMG_LIBRARIES "${SDL2_IMG_LIB_DIRS}/SDL2_image.lib")
endif ()

# Strip leading/trailing spaces 
string(STRIP "${SDL2_IMG_LIBRARIES}" SDL2_IMG_LIBRARIES)
