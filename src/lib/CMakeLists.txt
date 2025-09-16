message("Creating rum_lib")

add_definitions(-DRUM_LIB)

file(GLOB_RECURSE src_files RELATIVE ${CMAKE_CURRENT_LIST_DIR}
     *.cpp *.hpp *.c *.h *.inl)

set(src_files ${src_files} ../platform/platform.cpp)

set(CMAKE_CXX_FLAGS_DEBUG "/EHsc /MTd /Zi /Od") # /DDEBUG /D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/EHsc /MT /DNDEBUG") #/DRELEASE")

add_library(rum_lib STATIC ${src_files})

# Use C++ 17 by default
set_target_properties(rum_lib PROPERTIES
                      DEBUG_POSTFIX "_debug"
                      CXX_STANDARD 17)

target_precompile_headers(rum_lib PRIVATE
                          "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/lib/u_pch.h>")
