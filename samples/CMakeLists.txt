
set(TARGET         samples)

project(${TARGET})


ucm_add_dirs("" TO _SRC RECURSIVE FILTER_POP 1)

IF(WIN32)
	link_libraries(shaderc_combined.lib)
ELSE(WIN32)
	link_libraries(libshaderc_combined.a)
ENDIF(WIN32)


add_executable(${TARGET} ${_SRC})

target_include_directories(${TARGET} PRIVATE "./")
target_link_libraries(${TARGET} gfx)

set_target_properties(${TARGET} PROPERTIES FOLDER mygfx)