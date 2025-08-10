//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtFramework.h"


//
//	Simple app
//

class SimpleApp : public OtFrameworkApp {
	void onRender() override {
	}
};


//
//	main
//

int main(int, char**) {
	OtFramework framework;
	SimpleApp app;
	framework.run(&app);
	return 0;
}
