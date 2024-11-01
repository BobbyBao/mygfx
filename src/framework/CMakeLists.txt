
set(TARGET         framework)

ucm_add_dirs("" TO _SRC RECURSIVE FILTER_POP 1)

IF(WIN32)
	link_libraries(shaderc_combined.lib)
ELSE(WIN32)
	link_libraries(libshaderc_combined.a)
ENDIF(WIN32)

add_library (${TARGET} STATIC ${_SRC})

target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET} gfx glm ktx)
