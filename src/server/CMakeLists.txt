message("Creating rum_server")

add_definitions(-DRUM_SERVER)

file(GLOB_RECURSE src_files RELATIVE ${CMAKE_CURRENT_LIST_DIR}
     *.cpp *.hpp *.c *.h *.inl)

set(src_files ${src_files} ${SDK_PATH}/iniparser/iniparser.c)
set(src_files ${src_files} ${SDK_PATH}/md5/md5.cpp)

# SERVER ----------------------------------------------------------------------

add_executable(rum_server ${src_files})

set(CMAKE_CXX_FLAGS_DEBUG "/EHsc /MTd /Zi /Od") # /DDEBUG /D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/EHsc /MT /DNDEBUG") #/DRELEASE")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/SAFESEH:NO /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:msvcrtd.lib /NODEFAULTLIB:msvcprtd.lib /NODEFAULTLIB:libcmt.lib /INCREMENTAL /DEBUG /MANIFESTUAC:NO")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/SAFESEH:NO /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:msvcrtd.lib /NODEFAULTLIB:msvcprtd.lib /MANIFESTUAC:NO")

#/VERBOSE:LIB

# Use C++ 17 by default
set_target_properties(rum_server PROPERTIES
                      DEBUG_POSTFIX "_debug"
                      CXX_STANDARD 17)

add_dependencies(rum_server rum_lib rum_debugger)

# CUO LINK --------------------------------------------------------------------

target_link_libraries(rum_server optimized rum_lib debug rum_lib_debug)

# DEBUGGER LINK ---------------------------------------------------------------

target_link_libraries(rum_server optimized rum_debugger debug rum_debugger_debug)

# SQLITE LINK -----------------------------------------------------------------

target_link_libraries(rum_server sqlite3)

# SQUIRREL LINK ---------------------------------------------------------------

target_link_libraries(rum_server optimized sqstdlib debug sqstdlibD)
target_link_libraries(rum_server optimized squirrel debug squirrelD)

# ZLIB LINK -------------------------------------------------------------------

target_link_libraries(rum_server ZLib)

# WINDOWS LINK ----------------------------------------------------------------

if(WIN32)
  target_link_libraries(rum_server winmm ws2_32)
endif()

# POST BUILD ------------------------------------------------------------------

# Copy the executable file to root
add_custom_command(TARGET rum_server POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:rum_server>" ${PROJECT_SOURCE_DIR})

# Copy the .pdb over, but only in debug mode. For release mode, this will attempt to copy the server executable again,
# which won't happen because they won't be different
add_custom_command(TARGET rum_server POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<$<CONFIG:debug>:$<TARGET_FILE_DIR:rum_server>/rum_server_debug.pdb>$<$<CONFIG:release>:$<TARGET_FILE:rum_server>>" "$<$<CONFIG:debug>:${PROJECT_SOURCE_DIR}/rum_server_debug.pdb>$<$<CONFIG:release>:${PROJECT_SOURCE_DIR}>")

target_precompile_headers(rum_server PRIVATE
                          "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/server/s_pch.h>")
