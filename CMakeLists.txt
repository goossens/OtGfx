#	TextEditor - A syntax highlighting text editor for ImGui
#	Copyright (c) 2024-2025 Johan A. Goossens. All rights reserved.
#
#	This work is licensed under the terms of the MIT license.
#	For a copy, see <https://opensource.org/licenses/MIT>.

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(OtGfx VERSION 1.0 DESCRIPTION "ObjectTalk Graphics Framework" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(deps)
add_subdirectory(3rdparty)

add_subdirectory(language)
add_subdirectory(gfx)

add_executable(example main.cpp)
target_link_libraries(example
	language
	SDL3::SDL3
	SDL3_Headers
	imguifd
	gfx)

target_compile_options(example PRIVATE
	$<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>:-Werror -Wall -Wpedantic -Wextra>
	$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX>)

if(APPLE)
	target_link_options(example PRIVATE LINKER:-no_warn_duplicate_libraries)
endif()

add_custom_command(TARGET example POST_BUILD COMMAND $<$<CONFIG:release>:${CMAKE_STRIP}> ARGS ${CMAKE_BINARY_DIR}/Release/example)

if(CMAKE_GENERATOR STREQUAL Xcode)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
endif()

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT example)
endif()
