#	TextEditor - A syntax highlighting text editor for ImGui
#	Copyright (c) 2024-2025 Johan A. Goossens. All rights reserved.
#
#	This work is licensed under the terms of the MIT license.
#	For a copy, see <https://opensource.org/licenses/MIT>.

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(OtGfx VERSION 1.0 DESCRIPTION "ObjectTalk Graphics Framework" LANGUAGES C CXX)

if(APPLE)
	set(OT_INCLUDE_UI ON)
endif()

if(UNIX AND NOT APPLE)
	include(FindX11)
	set(OT_INCLUDE_UI ${X11_FOUND})
endif()

if(WIN32)
	set(OT_INCLUDE_UI ON)
endif()

add_compile_definitions("OT_DEBUG=$<CONFIG:Debug>")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(deps)
add_subdirectory(3rdparty)
add_subdirectory(language)
add_subdirectory(network)

if(OT_INCLUDE_UI)
	add_subdirectory(shaders)
	add_subdirectory(gfx)
	add_subdirectory(gui)
	add_subdirectory(nodes)
	add_subdirectory(ide)
endif()

add_executable(ot main.cpp)

target_include_directories(ot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/demos)
target_link_libraries(ot language network argparse)

if(OT_INCLUDE_UI)
	target_compile_definitions(ot PRIVATE OT_INCLUDE_UI)
	target_link_libraries(ot shaders gfx gui nodes ide)

	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
		if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
			target_link_options(ot PRIVATE LINKER:-no_warn_duplicate_libraries)
		endif()
	endif()
endif()

target_compile_options(ot PRIVATE
	$<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>:-Werror -Wall -Wpedantic -Wextra>
	$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX>)

if(APPLE)
	target_link_options(ot PRIVATE LINKER:-no_warn_duplicate_libraries)
endif()


add_custom_command(TARGET ot POST_BUILD COMMAND $<$<CONFIG:release>:${CMAKE_STRIP}> ARGS ${CMAKE_BINARY_DIR}/Release/ot)

if(CMAKE_GENERATOR STREQUAL Xcode)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
	set_target_properties(ot PROPERTIES OUTPUT_NAME "ObjectTalk")
	set_target_properties(ot PROPERTIES MACOSX_BUNDLE ON)
	set_target_properties(ot PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/info.plist)
	set_target_properties(ot PROPERTIES XCODE_GENERATE_SCHEME ON)
	set_target_properties(ot PROPERTIES XCODE_ATTRIBUTE_EXECUTABLE_NAME "ot")
	set_target_properties(ot PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.goosje.ot")
	set_source_files_properties(${CMAKE_SOURCE_DIR}/resources/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ot)
endif()
