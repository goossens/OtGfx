//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtCommandBuffer.h"
#include "OtGraphicsDevice.h"


//
//	OtFrameworkApp class
//

class OtFrameworkApp {
public:
	// destructor
	virtual inline ~OtFrameworkApp() {}

	// event handlers
	virtual inline void onSetup([[maybe_unused]] OtGraphicsDevice& device) {}
	virtual inline void onRender([[maybe_unused]] OtGraphicsDevice& device, [[maybe_unused]] OtCommandBuffer& commands) {}
	virtual inline void onTerminate() {}
	virtual inline bool onCanQuit() { return true; }
};
