
set(TARGET         framework)

ucm_add_dirs("" TO _SRC RECURSIVE FILTER_POP 1)

set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/ktx)
set(KTX_SOURCES
    ${KTX_DIR}/lib/texture.c
    ${KTX_DIR}/lib/hashlist.c
    ${KTX_DIR}/lib/checkheader.c
    ${KTX_DIR}/lib/swap.c
    ${KTX_DIR}/lib/memstream.c
    ${KTX_DIR}/lib/filestream.c)

LIST(APPEND _SRC ${KTX_SOURCES})

add_library (${TARGET} STATIC ${_SRC})

target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${KTX_DIR}/include)
include_directories(${KTX_DIR}/other_include)
include_directories(${KTX_DIR}/lib)

target_link_libraries(${TARGET} mygfx glm)
