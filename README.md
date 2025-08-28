# OtGfx

This repository is used to develop a future Graphic Engine for [ObjectTalk](https://github.com/goossens/ObjectTalk).It is inspired by the [RavEngine Graphics Library (RGL)](https://github.com/RavEngine/RGL)API and will hopefully provide an abstraction that will make future ports much simpler.

Objectives:

* Abstract graphics pipeline components with low overhead.
* Replace [bx](https://github.com/bkaradzic/bx), [bimg](https://github.com/bkaradzic/bimg) and [bgfx](https://github.com/bkaradzic/bgfx) with SDL Image and SDL3 GPU.
* Cross compiler shaders from pure GLSL to Metal, Vulkan and DirectX12.
* Support build and runtime shader compilation.
