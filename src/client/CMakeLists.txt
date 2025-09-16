message("Creating rum_client")

add_definitions(-DRUM_CLIENT)

file(GLOB_RECURSE src_files RELATIVE ${CMAKE_CURRENT_LIST_DIR}
     *.cpp *.hpp *.c *.h *.inl *.rc)

set(src_files ${src_files} ${SDK_PATH}/iniparser/iniparser.c)
set(src_files ${src_files} ${SDK_PATH}/md5/md5.cpp)
set(src_files ${src_files} ${SDK_PATH}/glad/src/glad.c)

include_directories(${SDK_PATH}/imgui)
file(GLOB imgui_files ${SDK_PATH}/imgui
     ${SDK_PATH}/imgui/*.h
     ${SDK_PATH}/imgui/*.cpp)

# GLFW PRE --------------------------------------------------------------------

if(USE_GLFW)
  message(STATUS "Including GLFW")
  add_definitions(-DUSE_GLFW)
  add_definitions(-DFREEIMAGE_LIB)
  include_directories(${SDK_PATH}/directx7/include)
  include_directories(${SDK_PATH}/FreeImage)
  include_directories(${SDK_PATH}/glad/include)
  include_directories(${SDK_PATH}/glfw/include)
  if(${CMAKE_CL_64})
    link_directories(${SDK_PATH}/FreeImage/lib/x64)
    link_directories(${SDK_PATH}/glfw/lib/x64)
    link_directories(${SDK_PATH}/libpng/lib/x64)
  endif()

  file(GLOB imgui_files ${SDK_PATH}/imgui/backends
       ${SDK_PATH}/imgui/backends/*.h
       ${SDK_PATH}/imgui/backends/*.cpp)
endif()

# FMOD PRE --------------------------------------------------------------------

if(USE_FMOD)
  message(STATUS "Including FMOD")
  add_definitions(-DUSE_FMOD)
  include_directories(${SDK_PATH}/fmod/api/inc)
  link_directories(${SDK_PATH}/fmod/api/lib)
endif()

# FREETYPE PRE ----------------------------------------------------------------

include_directories(${SDK_PATH}/freetype/include)
if(${CMAKE_CL_64})
  link_directories(${SDK_PATH}/freetype/lib/x64)
endif()

# CLIENT ----------------------------------------------------------------------

add_executable(rum_client ${src_files} ${imgui_files})

set(CMAKE_CXX_FLAGS_DEBUG "/EHsc /MTd /Zi /Od") # /DDEBUG /D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/EHsc /MT /DNDEBUG") #/DRELEASE")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/SAFESEH:NO /SUBSYSTEM:CONSOLE /ENTRY:\"mainCRTStartup\" /NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:msvcrt.lib /INCREMENTAL /DEBUG /MANIFESTUAC:NO")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/SAFESEH:NO /SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\" /NODEFAULTLIB:libc.lib /NODEFAULTLIB:msvcrt.lib /MANIFESTUAC:NO")

# Use C++ 17 by default
set_target_properties(rum_client PROPERTIES
                      DEBUG_POSTFIX "_debug"
                      CXX_STANDARD 17)

add_dependencies(rum_client rum_lib rum_debugger)

# CUO LINK --------------------------------------------------------------------

target_link_libraries(rum_client optimized rum_lib debug rum_lib_debug)

# DEBUGGER LINK ---------------------------------------------------------------

target_link_libraries(rum_client optimized rum_debugger debug rum_debugger_debug)

# GLFW LINK -------------------------------------------------------------------

#target_link_libraries(rum_client ddraw.lib dsound.lib dinput.lib dxguid.lib)

if(USE_GLFW)
  target_link_libraries(rum_client glfw3)
  target_link_libraries(rum_client LibPNG)
  target_link_libraries(rum_client optimized FreeImageLib debug FreeImageLibD)
endif()

# FMOD LINK -------------------------------------------------------------------

if(USE_FMOD)
  if(${CMAKE_CL_64})
    target_link_libraries(rum_client fmodex64_vc)
  else()
    target_link_libraries(rum_client fmodex_vc)
  endif()
endif()

target_link_libraries(rum_client freetype)

# SQLITE LINK -----------------------------------------------------------------

target_link_libraries(rum_client sqlite3)

# SQUIRREL LINK ---------------------------------------------------------------

target_link_libraries(rum_client optimized sqstdlib debug sqstdlibD)
target_link_libraries(rum_client optimized squirrel debug squirrelD)

# ZLIB LINK -------------------------------------------------------------------

target_link_libraries(rum_client ZLib)

# WINDOWS LINK ----------------------------------------------------------------

if(WIN32)
  target_link_libraries(rum_client gdi32 ole32 winmm ws2_32)
endif()

# POST BUILD ------------------------------------------------------------------

# Copy the executable file to root
add_custom_command(TARGET rum_client POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:rum_client>" ${PROJECT_SOURCE_DIR})

# Copy the .pdb over, but only in debug mode. For release mode, this will attempt to copy the client executable again,
# which won't happen because they won't be different
add_custom_command(TARGET rum_client POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<$<CONFIG:debug>:$<TARGET_FILE_DIR:rum_client>/rum_client_debug.pdb>$<$<CONFIG:release>:$<TARGET_FILE:rum_client>>" "$<$<CONFIG:debug>:${PROJECT_SOURCE_DIR}/rum_client_debug.pdb>$<$<CONFIG:release>:${PROJECT_SOURCE_DIR}>")

target_precompile_headers(rum_client PRIVATE
                          "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/client/c_pch.h>")
