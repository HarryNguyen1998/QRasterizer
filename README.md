# QRasterizer
A simple rasterizer i am implementing to understand how OpenGL and DirectX work under the hood.

---

## Sections:
- [Configuration](#configuration)
- [Licensing](#license)
- [Development notes](#development-notes)
- [Resources](#resources)
- [Credit](#credit)

---

## Configuration
- Run `git clone https://github.com/HarryNguyen1998/QRasterizer.git`

---

## License
- The project uses GNU General Public License v3.0. Details can be found in [LICENSE.md](./LICENSE.md)

---

## Development notes
Current project structure:
```
ProjectRoot
----.gitignore
----LICENSE.md
----README.md
----Assets
----Game (CMakeCache, exe files)
----Src
--------CMakelists.txt
--------Main.cpp
--------External
------------SubDir1
----------------Include
----------------Lib
----------------SubDir1-config.cmake
--------Include
------------SubDir1
----------------Foo.h (Foo1.h, Foo2.h, etc)
------------SubDir2
----------------Bar.h (Bar1.h, Bar2.h, etc)
--------SubDir1
------------CMakelists.txt
------------Foo.cpp (Foo1.cpp, Foo2.cpp, etc)
--------SubDir2
------------CMakelists.txt
------------Bar.cpp (Bar1.cpp, Bar2.cpp, etc)
--------Test
------------CMakelists.txt
------------TestFoo.cpp (TestFoo1.cpp, etc)
------------TestBar.cpp (TestBar1.cpp, etc)
----
```
- All dependencies (.dll, .lib files) are stored in `External` folder and referenced by `find_package`. When CMake build the project, the Assets folder and necessary .dll files will be copied to the build folder.

- TODO

---

## Resources
- [Main resource](https://www.scratchapixel.com/index.php)
- [thebennybox 3D game rendering series](https://www.youtube.com/watch?v=Y_vvC2G7vRo&list=PLEETnX-uPtBUbVOok816vTl1K9vV1GgH5)
- [Game loop](http://gameprogrammingpatterns.com/game-loop.html)
- [Timer](https://bell0bytes.eu/keeping-track-of-time/)
- [tinyrenderer (mostly DrawLine)](https://github.com/ssloy/tinyrenderer/wiki)
- [How to write math class](https://www.reedbeta.com/blog/on-vector-math-libraries/)
- [Texture manager](https://codereview.stackexchange.com/questions/87367/texture-managing)

---

## Credit
- AfricanHead.obj is taken from tinyrenderer course.
- Newell teaset is taken from [Utah website](https://www.cs.utah.edu/~natevm/newell_teaset/newell_teaset.zip)
- All other .obj files are taken from thebennybox's series.