cmake_minimum_required(VERSION 3.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(NAME mygfx)

project(${NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

IF(MSVC)
add_compile_options(/MP)
add_compile_options(/MD)
ENDIF()

add_definitions(
-D_CRT_SECURE_NO_WARNINGS
-D_ITERATOR_DEBUG_LEVEL=0
-D_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS
-D_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS
)

OPTION(FORCE_VALIDATION "Forces validation on for all samples at compile time (prefer using the -v / --validation command line arguments)" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# Clang specific stuff
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum")
endif()


# Force validation at compile time
if (FORCE_VALIDATION)
	add_definitions(-DFORCE_VALIDATION)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")

include_directories("$ENV{VULKAN_SDK}/Include")
link_directories("$ENV{VULKAN_SDK}/Lib")

include(cmake/helper.cmake)

add_subdirectory(src/gfx)
set_target_properties(gfx PROPERTIES FOLDER mygfx)

add_subdirectory(src)

add_subdirectory(demo/demos)
set_target_properties(demos PROPERTIES FOLDER demo)