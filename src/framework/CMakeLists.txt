
set(TARGET         framework)

ucm_add_dirs("" TO _SRC RECURSIVE FILTER_POP 1)


add_library (${TARGET} STATIC ${_SRC})

target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET} gfx glm ktx)
