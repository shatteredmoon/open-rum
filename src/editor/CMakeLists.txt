message("Creating rum_editor")

add_definitions(-DRUM_EDITOR)

file(GLOB_RECURSE src_files RELATIVE ${CMAKE_CURRENT_LIST_DIR}
     *.cpp *.hpp *.c *.h *.inl *.qrc)

set(src_files ${src_files} ${SDK_PATH}/iniparser/iniparser.c)
set(src_files ${src_files} ${SDK_PATH}/md5/md5.cpp)

# FREEIMAGE PRE ---------------------------------------------------------------

include_directories(${SDK_PATH}/freeimage)

add_definitions(-DFREEIMAGE_LIB)

if(${CMAKE_CL_64})
  link_directories(${SDK_PATH}/FreeImage/lib/x64)
endif()

# QT PRE ----------------------------------------------------------------------

file(GLOB_RECURSE qt_ui_files RELATIVE ${CMAKE_CURRENT_LIST_DIR}
     ${CMAKE_CURRENT_LIST_DIR}/ui/*.ui)

file(GLOB_RECURSE qt_h_files RELATIVE ${CMAKE_CURRENT_LIST_DIR}
     ${CMAKE_CURRENT_LIST_DIR}/ui/*.h *${CMAKE_CURRENT_LIST_DIR}/ui/*.hpp)

message("USING QT $ENV{QTDIR}")

list(APPEND CMAKE_PREFIX_PATH $ENV{QTDIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Sql REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

include_directories(${CMAKE_CURRENT_LIST_DIR}/ui)
include_directories(${CMAKE_CURRENT_LIST_DIR}/ui/qtFindReplace)

# EDITOR ----------------------------------------------------------------------

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(rum_editor ${src_files} ${qt_moc_headers} ${qt_forms_headers})

set(CMAKE_CXX_FLAGS_DEBUG "/EHsc /MTd /Zi /Od") # /DDEBUG /D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/EHsc /MT /DNDEBUG") #/DRELEASE")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE /ENTRY:\"mainCRTStartup\" /NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:msvcrt.lib /INCREMENTAL /DEBUG /MANIFESTUAC:NO")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\" /NODEFAULTLIB:libc.lib /NODEFAULTLIB:msvcrt.lib /MANIFESTUAC:NO")

# Use C++ 17 by default
set_target_properties(rum_editor PROPERTIES
                      DEBUG_POSTFIX "_debug"
                      CXX_STANDARD 17)

add_dependencies(rum_editor rum_lib)

# CUO LINK --------------------------------------------------------------------

target_link_libraries(rum_editor optimized rum_lib debug rum_lib_debug)

# FREEIMAGE LINK --------------------------------------------------------------

  target_link_libraries(rum_editor optimized FreeImageLib debug FreeImageLibD)

# QT LINK ---------------------------------------------------------------------

target_link_libraries(rum_editor Qt5::Core)
target_link_libraries(rum_editor Qt5::Widgets)
target_link_libraries(rum_editor Qt5::Gui)
target_link_libraries(rum_editor Qt5::OpenGL)
target_link_libraries(rum_editor Qt5::Sql)
target_link_libraries(rum_editor Qt5::Network)

# SQLITE LINK -----------------------------------------------------------------

target_link_libraries(rum_editor sqlite3)

# SQUIRREL LINK ---------------------------------------------------------------

target_link_libraries(rum_editor optimized sqstdlib debug sqstdlibD)
target_link_libraries(rum_editor optimized squirrel debug squirrelD)

# ZLIB LINK -------------------------------------------------------------------

target_link_libraries(rum_editor ZLib)

# WINDOWS LINK ----------------------------------------------------------------

target_link_libraries(rum_editor winmm ws2_32 opengl32 glu32)

# POST BUILD ------------------------------------------------------------------

# Copy the executable file to root
add_custom_command(TARGET rum_editor POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:rum_editor>" ${PROJECT_SOURCE_DIR})

# Copy the .pdb over, but only in debug mode. For release mode, this will attempt to copy the editor executable again,
# which won't happen because they won't be different
add_custom_command(TARGET rum_editor POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<$<CONFIG:debug>:$<TARGET_FILE_DIR:rum_editor>/rum_editor_debug.pdb>$<$<CONFIG:release>:$<TARGET_FILE:rum_editor>>" "$<$<CONFIG:debug>:${PROJECT_SOURCE_DIR}/rum_editor_debug.pdb>$<$<CONFIG:release>:${PROJECT_SOURCE_DIR}>")

target_precompile_headers(rum_editor PRIVATE
                          "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/editor/e_pch.h>")
