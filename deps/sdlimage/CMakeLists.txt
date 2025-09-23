#	ObjectTalk Scripting Language
#	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
#
#	This work is licensed under the terms of the MIT license.
#	For a copy, see <https://opensource.org/licenses/MIT>.

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Populate(sdlimage QUIET GIT_REPOSITORY "https://github.com/libsdl-org/SDL_image" GIT_TAG "release-3.2.4")
add_subdirectory(${sdlimage_SOURCE_DIR} ${sdlimage_BINARY_DIR} EXCLUDE_FROM_ALL)

set_target_properties(SDL3_image-static PROPERTIES FOLDER deps/sdl)
