# QRasterizer
A simple rasterizer for me to understand how a rendering API (OpenGL, DirectX) works under the hood.

---

## Sections:
- [Configuration](#configuration)
- [Development notes](#development-notes)
- [Resources](#resources)
- [Credit](#credit)

---

## Configuration
- Run `git clone --recursive-submodules https://github.com/HarryNguyen1998/QRasterizer.git`
- From project root folder: `mkdir Game && cmake -S Src -B Game -G "Ninja" && cmake --build Game`

Q: How to build in VS2017:
- Run `git clone --recursive-submodules https://github.com/HarryNguyen1998/QRasterizer.git`
- Open the project folder in VS2017 and let it parse.
- Click the drop-down arrow in Configuration > Manage Configurations to create a CMakeSettings.json file
- CMAKE tab > Build Only QRasterizer target. Then, Select Startup Item to QRasterizer.exe, then
  click on it to see the project running.

---

## Development notes
- All dependencies (.dll, .lib files) are stored in `External` folder and referenced by
  `find_package`. When CMake build the project, the Assets folder and necessary .dll files will be
  copied to the build folder.
- Pixel format is RGBA32
- Math is done in right-hand convention, aka vector is pre-multiplied, winding order is CW.
- In NDC space, x, y in range [-1,1], z in range [0,1]. In raster space y is pointing up.

---

## Resources
- Main resource [1](https://www.scratchapixel.com/index.php) and [2](https://gabrielgambetta.com/computer-graphics-from-scratch/)
- [thebennybox 3D game rendering series](https://www.youtube.com/watch?v=Y_vvC2G7vRo&list=PLEETnX-uPtBUbVOok816vTl1K9vV1GgH5)
- [How to write a math library](https://www.reedbeta.com/blog/on-vector-math-libraries/)
- [Bresenham's line algorithm](https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm)
- [Texture manager](https://codereview.stackexchange.com/questions/87367/texture-managing)

---

## Credit
- .obj files are taken from the [thebennybox](https://github.com/BennyQBD/3DSoftwareRenderer/tree/master/res) or [here](https://github.com/alecjacobson/common-3d-test-models)