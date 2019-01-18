# Refactoring
- Core (the core class and interfaces)
    - [ ] basic math: vec, quat, matrix and transform
    - [ ] eigen3: for linear algebra
    - [x] data types: audio, image
    - [x] logger: based on spdlog
    - [x] timer: record time
    - [ ] memory allocater (an interface)
- Media (the codes related to media)
- OpenGL (the codes related to opengl)

# naming
- All global functions in namespace are lower-cased: such as `snow::log::info()`, `snow::path::dirname()`
- Class related functions and members are in camel style:
    - class name like `TimerGuard`
    - class constants like `Landmark::PointsNumber`
    - class member like `mStartTime`, `mImagePtr`(pointer), access with `startTime()` and `imagePtr()`
    - static methods like `LoadImage()`
    - non-static methods like `saveFile()`
