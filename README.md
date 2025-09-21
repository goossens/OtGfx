# OtGfx

This repository is used to develop a future Graphic Engine for [ObjectTalk](https://github.com/goossens/ObjectTalk).

Objectives:

* Abstract graphics/compute pipeline components with low overhead.
* Replace [bx](https://github.com/bkaradzic/bx), [bimg](https://github.com/bkaradzic/bimg) and [bgfx](https://github.com/bkaradzic/bgfx) with [SDL3, SDL3 GPU](https://github.com/libsdl-org/SDL) and [SDL Image](https://github.com/libsdl-org/SDL_image).
* Cross compiler shaders from pure GLSL to Metal, Vulkan and DirectX12.
* Support build and runtime shader compilation.
