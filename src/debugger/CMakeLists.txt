message("Creating rum_debugger")

add_definitions(-DRUM_DEBUGGER)

include_directories(${SDK_PATH}/imgui)
include_directories(${SDK_PATH}/netImgui)

file(GLOB_RECURSE src_files RELATIVE ${CMAKE_CURRENT_LIST_DIR}
     *.cpp *.hpp *.c *.h *.inl)

set(CMAKE_CXX_FLAGS_DEBUG "/EHsc /MTd /Zi /Od") # /DDEBUG /D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/EHsc /MT /DNDEBUG") #/DRELEASE")

file(GLOB imgui_files ${SDK_PATH}/imgui
     ${SDK_PATH}/imgui/*.h
     ${SDK_PATH}/imgui/*.cpp)

file(GLOB_RECURSE netimgui_files ${SDK_PATH}/netImgui
     ${SDK_PATH}/netImgui/*.h
     ${SDK_PATH}/netImgui/*.cpp)

add_library(rum_debugger STATIC ${src_files} ${imgui_files} ${netimgui_files})

# Use C++ 17 by default
set_target_properties(rum_debugger PROPERTIES
                      DEBUG_POSTFIX "_debug"
                      CXX_STANDARD 17)
