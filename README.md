# QRasterizer
A simple rasterizer for me to understand how OpenGL and DirectX work under the hood.

---

## Sections:
- [Configuration](#configuration)
- [Licensing](#license)
- [Development notes](#development-notes)
- [Resources](#resources)
- [Credit](#credit)

---

## Configuration
Q: How to run and test the project? Below is an example:
- Run `git clone https://github.com/HarryNguyen1998/QRasterizer.git`
- Open the project folder in Visual Studio and let it parse.
- Click the drop-down arrow in Configuration > Manage Configurations to create a CMakeSettings.json file
- CMAKE tab > Build Only QRasterizer target. Then, Select Startup Item to QRasterizer.exe, then
  click on it to see the project running.

---

## License
- The project uses GNU General Public License v3.0. Details can be found in [LICENSE.md](./LICENSE.md)

---

## Development notes
How the project is currently structured:
```
.gitignore
LICENSE.md
README.md
Assets (texture, .obj files)
Game (CMakeCache, exe files)
Src
----CMakelists.txt      <--- The main CMake file
----Main.cpp
----External
--------SubDir1
------------Include
------------Lib
------------SubDir1-config.cmake
----Include
--------SubDir1
------------Foo.h (Foo1.h, Foo2.h, etc)
--------SubDir2
------------Bar.h (Bar1.h, Bar2.h, etc)
----SubDir1
--------CMakelists.txt
--------Foo.cpp (Foo1.cpp, Foo2.cpp, etc)
----SubDir2
--------CMakelists.txt
--------Bar.cpp (Bar1.cpp, Bar2.cpp, etc)
----Test
--------CMakelists.txt
--------TestFoo.cpp (TestFoo1.cpp, etc)
--------TestBar.cpp (TestBar1.cpp, etc)
```
- All dependencies (.dll, .lib files) are stored in `External` folder and referenced by
  `find_package`. When CMake build the project, the Assets folder and necessary .dll files will be
  copied to the build folder.
- @todo

---

## Resources
- Main resource [1](https://www.scratchapixel.com/index.php) and [2](https://gabrielgambetta.com/computer-graphics-from-scratch/)
- [thebennybox 3D game rendering series](https://www.youtube.com/watch?v=Y_vvC2G7vRo&list=PLEETnX-uPtBUbVOok816vTl1K9vV1GgH5)
- [How to write a math library](https://www.reedbeta.com/blog/on-vector-math-libraries/)
- [Game loop](http://gameprogrammingpatterns.com/game-loop.html)
- [Timer code based on this](https://stackoverflow.com/questions/33304351/sdl2-fast-pixel-manipulation)
- [Bresenham's line algorithm](https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm)
- [Texture manager](https://codereview.stackexchange.com/questions/87367/texture-managing)

---

## Credit
- Newell teaset is taken from [Utah website](https://www.cs.utah.edu/~natevm/newell_teaset/newell_teaset.zip)
- All other .obj files are taken from thebennybox's series, or self-created from Blender.