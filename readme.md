# Refactoring
- Core (the core class and interfaces)
    - basic math: vec, quat, matrix and transform
    - eigen3: for linear algebra
    - data types: audio, image
    - logger: based on spdlog
    - timer: record time
    - memory allocater (an interface)
- Media (the codes related to media)
- OpenGL (the codes related to opengl)

# Modules
- Core:
    - glm, eigen3, extened_types(such as `int3`)
    - simple simd functions, string operations, path operations (like python os.path)
    - image (based on stb_image)
    - stream, frame, input (for data streaming)
    - argparse
- OpenGL:
    - GUI: glad, SDL2, ImGui
    - Model: Naive obj reader, Assimp (optional)
- Media
    - `WAV` writer and reader
    - FFmpeg(optional): media writer and reader
- [ ] Vision:
    - [ ] OpenCV
    - [ ] Dlib
- [ ] Functional Programming
