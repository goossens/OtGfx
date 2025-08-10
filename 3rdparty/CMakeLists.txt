#	ObjectTalk Scripting Language
#	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
#
#	This work is licensed under the terms of the MIT license.
#	For a copy, see <https://opensource.org/licenses/MIT>.

add_library(isocline STATIC isocline/isocline.c isocline/isocline.h)
target_include_directories(isocline PUBLIC isocline)
set_target_properties(isocline PROPERTIES FOLDER 3rdParty)
