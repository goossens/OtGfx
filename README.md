# OtGfx

This repository was used to develop a future Graphic Engine for
[ObjectTalk](https://github.com/goossens/ObjectTalk). It is now in archive mode as the code has been
merged with the main ObjectTalk repository.

## Objectives

* Abstract graphics/compute pipeline components with low overhead.
* Replace [bx](https://github.com/bkaradzic/bx), [bimg](https://github.com/bkaradzic/bimg) and [bgfx](https://github.com/bkaradzic/bgfx) with [SDL3, SDL3 GPU](https://github.com/libsdl-org/SDL).
* Cross compiler shaders from pure GLSL to Metal, Vulkan and DirectX12.
* Support build and runtime shader compilation.

## Lessons Learned

When the graphical part of ObjectTalk was first developed, every attempt was made to limit access to the
underlying BGFX, BIMG and BX libraries to just ObjectTalk's gfx module and therefore create an abstraction
layer (or so I thought). If you look at it from just the API perspective, this was successful as no calls
to the lower level libraries appeared in any other parts of ObjectTalk.

Unfortunately, the concepts behind the BGFX libraries were not abstracted away so porting to the SDL3 GPU
interface still became a major effort. As this work is now completed, the gfx module has a new API.
Undoubtedly, the new gfx API has plenty of SDL3 GPU-isms in it so it still is not a truly abstract API.
Luckily, the SDL3 GPU API is just a thin (abstract) layer over modern GPU frameworks like Metal, Vulkan and
DirectX12. Let's see how well our new API survives the next couple of years. Hopefully, a major port is not
required anytime soon as the SDL3 GPU capability is very new, surprisingly mature and well maintained.

## License

Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.

This work is licensed under the terms of the MIT license.
For a copy, see <https://opensource.org/licenses/MIT>.
