
include_directories(external)

add_subdirectory(external/stb)
add_subdirectory(external/glm)

add_subdirectory(external/imgui)

add_subdirectory(external/SDL3)
include_directories(external/SDL3/include)

set_target_properties(SDL3-shared PROPERTIES FOLDER external)
set_target_properties(SDL3_test PROPERTIES FOLDER external)

add_subdirectory(external/concurrencpp)
include_directories(external/concurrencpp/include)

add_subdirectory(app)
include_directories(app)

add_subdirectory(demos)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/media DESTINATION ${PROJECT_BINARY_DIR})