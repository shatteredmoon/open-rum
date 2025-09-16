message("Creating rum_portal")

add_definitions(-DRUM_PORTAL)

file(GLOB_RECURSE src_files RELATIVE ${CMAKE_CURRENT_LIST_DIR}
     *.cpp *.hpp *.c *.h *.inl *.rc)

set(src_files ${src_files} ${SDK_PATH}/iniparser/iniparser.c)
set(src_files ${src_files} ${SDK_PATH}/md5/md5.cpp)

# QT PRE ----------------------------------------------------------------------

file(GLOB_RECURSE qt_ui_files RELATIVE ${CMAKE_CURRENT_LIST_DIR}
     ${CMAKE_CURRENT_LIST_DIR}/ui/*.ui)

file(GLOB_RECURSE qt_h_files RELATIVE ${CMAKE_CURRENT_LIST_DIR}
     ${CMAKE_CURRENT_LIST_DIR}/ui/*.h *${CMAKE_CURRENT_LIST_DIR}/ui/*.hpp)

message("USING QT $ENV{QTDIR}")

list(APPEND CMAKE_PREFIX_PATH $ENV{QTDIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Sql REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

include_directories(${CMAKE_CURRENT_LIST_DIR}/ui)

# PORTAL ----------------------------------------------------------------------

add_executable(rum_portal ${src_files} ${qt_moc_headers} ${qt_forms_headers})

set(CMAKE_CXX_FLAGS_DEBUG "/EHsc /MTd /Zi /Od") # /DDEBUG /D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/EHsc /MT /DNDEBUG") #/DRELEASE")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE /ENTRY:\"mainCRTStartup\" /NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:msvcrt.lib /INCREMENTAL /DEBUG /MANIFESTUAC:NO")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\" /NODEFAULTLIB:libc.lib /NODEFAULTLIB:msvcrt.lib /MANIFESTUAC:NO")

# Use C++ 17 by default
set_target_properties(rum_portal PROPERTIES
                      DEBUG_POSTFIX "_debug"
                      CXX_STANDARD 17)

add_dependencies(rum_portal rum_lib)

# CUO LINK --------------------------------------------------------------------

target_link_libraries(rum_portal optimized rum_lib debug rum_lib_debug)

# QT LINK ---------------------------------------------------------------------

target_link_libraries(rum_portal Qt5::Core)
target_link_libraries(rum_portal Qt5::Widgets)
target_link_libraries(rum_portal Qt5::Gui)
target_link_libraries(rum_portal Qt5::OpenGL)
target_link_libraries(rum_portal Qt5::Sql)
target_link_libraries(rum_portal Qt5::Network)

# SQLITE LINK -----------------------------------------------------------------

target_link_libraries(rum_portal sqlite3)

# SQUIRREL LINK ---------------------------------------------------------------

target_link_libraries(rum_portal optimized sqstdlib debug sqstdlibD)
target_link_libraries(rum_portal optimized squirrel debug squirrelD)

# ZLIB LINK -------------------------------------------------------------------

target_link_libraries(rum_portal ZLib)

# WINDOWS LINK ----------------------------------------------------------------

target_link_libraries(rum_portal ws2_32)

# POST BUILD ------------------------------------------------------------------

# Copy the executable file to root
add_custom_command(TARGET rum_portal POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:rum_portal>" ${PROJECT_SOURCE_DIR})

# Copy the .pdb over, but only in debug mode. For release mode, this will attempt to copy the portal executable again,
# which won't happen because they won't be different
add_custom_command(TARGET rum_portal POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<$<CONFIG:debug>:$<TARGET_FILE_DIR:rum_portal>/rum_portal_debug.pdb>$<$<CONFIG:release>:$<TARGET_FILE:rum_portal>>" "$<$<CONFIG:debug>:${PROJECT_SOURCE_DIR}/rum_portal_debug.pdb>$<$<CONFIG:release>:${PROJECT_SOURCE_DIR}>")
