
include_directories(external)

add_subdirectory(external/stb)

add_subdirectory(external/VulkanMemoryAllocator)

add_subdirectory(external/imgui)

add_subdirectory(external/SDL3)
include_directories(external/SDL3/include)

set_target_properties(SDL3-shared PROPERTIES FOLDER external)
set_target_properties(SDL3_test PROPERTIES FOLDER external)


add_subdirectory(app)
include_directories(app)

add_subdirectory(demos)