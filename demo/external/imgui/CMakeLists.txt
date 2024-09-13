cmake_minimum_required(VERSION 3.6)
set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_GENERATOR_PLATFORM x64)
project (ImGUI)

set(imgui_src 
"imgui.cpp"
"imgui_demo.cpp"
"imgui_draw.cpp"
"imgui_tables.cpp"
"imgui_widgets.cpp"
"imconfig.h"
"imgui.h"
"imgui_internal.h"
"imstb_rectpack.h"
"imstb_textedit.h"
"imstb_truetype.h"
"backends/imgui_impl_sdl3.cpp"
)

if(WIN32)
	#list(APPEND imgui_src "misc/freetype/imgui_freetype.cpp")
endif(WIN32)

add_library (${PROJECT_NAME} STATIC ${imgui_src})

target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
target_link_libraries(${PROJECT_NAME} SDL3-static)
 else(WIN32)
target_link_libraries(${PROJECT_NAME} SDL3-static)
 
 endif(WIN32)

set_property(TARGET ImGUI PROPERTY FOLDER external)
