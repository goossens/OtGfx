//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Common include files
//

#include "imgui.h"


//
//	OtDemo
//

class OtDemo {
public:
	// destructor
	virtual ~OtDemo() {}

	// method to be implemented by each demo
	virtual void run() = 0;
};
