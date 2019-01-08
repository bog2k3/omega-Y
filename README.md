# omegaY
re-iteration of the classic Omega-X first person shooter

(uses Bugs OpenGL Framework library from https://github.com/bog2k3/boglfw.git)

### how to build
1. install tools (cmake, make, g++)

2. install dependencies: 
- GLFW3 https://www.glfw.org/ 
- GLEW http://glew.sourceforge.net/ 
- ASIO https://think-async.com/Asio/Download.html 
- libpng-devel, libz-devel, mesa-devel 
  
3. clone boglfw repo and build it:
```
$ ./build.sh
```
4. clone this repo and in its root directory create a file with the path to boglfw output files:
```
$ echo '/abs/path/to/boglfw/build/dist' > .boglfw_path
```
5. build:
```
$ sh build.sh
```
That's it. 
