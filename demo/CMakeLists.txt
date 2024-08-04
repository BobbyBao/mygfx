
include_directories(external)

add_subdirectory(external/stb)
add_subdirectory(external/glm)

add_subdirectory(external/imgui)
add_subdirectory(external/ktx)

add_subdirectory(external/SDL3)
include_directories(external/SDL3/include)

set_target_properties(SDL3-shared PROPERTIES FOLDER external)
set_target_properties(SDL3_test PROPERTIES FOLDER external)

add_subdirectory(external/concurrencpp)
include_directories(external/concurrencpp/include)
set_target_properties(concurrencpp PROPERTIES FOLDER external)

include_directories(external/cgltf)

add_subdirectory(framework)
set_target_properties(framework PROPERTIES FOLDER demo)

add_subdirectory(app)
set_target_properties(app PROPERTIES FOLDER demo)

add_subdirectory(demos)
set_target_properties(demos PROPERTIES FOLDER demo)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/media DESTINATION ${PROJECT_BINARY_DIR})