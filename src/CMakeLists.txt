
include_directories(external)

add_subdirectory(external/stb)
add_subdirectory(external/glm)
include_directories(external/glm)

add_subdirectory(external/ktx)

add_subdirectory(external/SDL3)
include_directories(external/SDL3/include)

add_subdirectory(external/imgui)

#set_target_properties(SDL3-shared PROPERTIES FOLDER external)
#set_target_properties(SDL3_test PROPERTIES FOLDER external)
if (WIN32)
add_subdirectory(external/concurrencpp)
include_directories(external/concurrencpp/include)
set_target_properties(concurrencpp PROPERTIES FOLDER external)
endif (WIN32)

include_directories(external/cgltf)

add_subdirectory(framework)
target_include_directories(framework PUBLIC external/glm)

set_target_properties(framework PROPERTIES FOLDER mygfx)



add_subdirectory(media/shaders)

add_subdirectory(app)
set_target_properties(app PROPERTIES FOLDER mygfx)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/media DESTINATION ${PROJECT_BINARY_DIR})