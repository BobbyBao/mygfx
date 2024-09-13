
set(TARGET         demos)

project(${TARGET})


ucm_add_dirs("" TO _SRC RECURSIVE FILTER_POP 1)


add_executable(${TARGET} ${_SRC})
target_link_libraries(${TARGET} mygfx app concurrencpp)