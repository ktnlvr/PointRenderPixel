# PointRenderPixel
C++ Pixel Rendering Event-based Engine, for any graphical representations you can think of!

## Usage
1. The entire renderer is compressed into one file: [this one](PointRenderPixel.hpp). Download it!
2. Include! Put it somewhere in your project and include, as you would with any other file.
```cpp
#include "PointRenderPixel.hpp"
```
3. Link GLFW! Visit their [official website](https://www.glfw.org/) or [GitHub repo](https://github.com/glfw/glfw) and follow the installation guide for your system.
4. Configure! Data, which can not change runtime is defined in advance. As of now two defines are used:
```cpp
#define JOIN_ON_START // if defined, Renderer will run in the thread this method was called on, otherwise - in parallel
#define POINT_SIZE n  // set a fitting point size, it's like a pixel, but size of n
```
5. Look up an example program available: 

## Documentation
Coming soon...

## Contribution
Wanting to extend PointRenderPixel? Great!

Feel free to pull request, fork or issue, it can help a lot. All the code is separated into regions with 
corresponding functionality, so it shouldn't be too hard to get your head around.

## LICENSE
Coming soon...
