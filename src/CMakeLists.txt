
set(TARGET         mygfx)

project(${TARGET})


ucm_add_dirs("" TO _SRC RECURSIVE FILTER_POP 1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    
link_libraries(shaderc_combined.lib spirv-cross-core.lib spirv-cross-glsl.lib )

add_library(${TARGET} STATIC ${_SRC})
