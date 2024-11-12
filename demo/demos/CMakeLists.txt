
set(TARGET         demos)

project(${TARGET})


ucm_add_dirs("" TO _SRC RECURSIVE FILTER_POP 1)


add_executable(${TARGET} ${_SRC})
if(WIN32)
target_link_libraries(${TARGET} app concurrencpp)
else (WIN32)
target_link_libraries(${TARGET} app)

endif(WIN32)