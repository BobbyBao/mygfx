
file(GLOB_RECURSE ROOT_SOURCE ./*.shader ./*.glsl ./*.compute ./*.frag ./*.vert)

file(GLOB GLSL_SOURCE ./glsl/*.glsl ./glsl/*.inl ./glsl/*.h ./glsl/*.frag ./glsl/*.vert ./glsl/*.fs ./glsl/*.vs)

source_group("glsl" FILES ${GLSL_SOURCE} )

add_library (shaders INTERFACE 
${ROOT_SOURCE}
${GLSL_SOURCE}
)

set_source_files_properties(${Source_src} PROPERTIES VS_TOOL_OVERRIDE "Text")

set_property(TARGET shaders PROPERTY FOLDER "demo")