
set(TARGET         ktx)

set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KTX_SOURCES
    ${KTX_DIR}/lib/texture.c
    ${KTX_DIR}/lib/hashlist.c
    ${KTX_DIR}/lib/checkheader.c
    ${KTX_DIR}/lib/swap.c
    ${KTX_DIR}/lib/memstream.c
    ${KTX_DIR}/lib/filestream.c)

add_library (${TARGET} STATIC ${KTX_SOURCES})

target_include_directories(${TARGET} PUBLIC ${KTX_DIR}/include
PUBLIC ${KTX_DIR}/lib
PRIVATE ${KTX_DIR}/other_include
)

target_link_libraries(${TARGET} glm)

set_target_properties(${TARGET} PROPERTIES FOLDER external)