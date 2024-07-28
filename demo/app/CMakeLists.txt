
set(TARGET         app)

ucm_add_dirs("" TO _SRC RECURSIVE FILTER_POP 1)

add_library (${TARGET} STATIC ${_SRC})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET} mygfx ImGUI SDL3-shared)
