
set(TARGET         mygfx)

project(${TARGET})


ucm_add_dirs("" TO _SRC RECURSIVE FILTER_POP 1)
ucm_remove_directories("tiny_imageformat" FROM _SRC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories("tiny_imageformat/include")
    
link_libraries(shaderc_combined.lib spirv-cross-core.lib spirv-cross-glsl.lib )

add_library(${TARGET} STATIC ${_SRC})
