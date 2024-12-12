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

IF(WIN32)
ELSEIF (APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_METAL_EXT -DVK_EXAMPLE_XCODE_GENERATED")
IF(CMAKE_GENERATOR MATCHES "Makefiles" OR CMAKE_GENERATOR MATCHES "Ninja")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -ObjC++")
ELSE() # SRS - use objective-c++ language flag vs. ObjC++ for Xcode builds
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -xobjective-c++")
ENDIF()

ENDIF(WIN32)

IF(WIN32)
    # Nothing here (yet)
ELSEIF(APPLE)
    link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
ELSE(WIN32)
    link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${Vulkan_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(WIN32)


# Force validation at compile time
if (FORCE_VALIDATION)
	add_definitions(-DFORCE_VALIDATION)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")

include_directories("$ENV{VULKAN_SDK}/Include")
link_directories("$ENV{VULKAN_SDK}/Lib")

include(cmake/helper.cmake)

add_subdirectory(third_party/SPIRV-Cross)
add_subdirectory(src)
add_subdirectory(samples)
