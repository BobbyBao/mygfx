
set(TARGET         mygfx)

project(${TARGET})


ucm_add_dirs("" TO _SRC RECURSIVE FILTER_POP 1)
    
link_libraries(shaderc_combined.lib spirv-cross-core.lib spirv-cross-glsl.lib )

add_library(${TARGET} STATIC ${_SRC})

target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})