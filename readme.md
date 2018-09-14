# SNOW
A wrapper of my frequently used libraries:
- OpenGL, SDL2 and ImGui (used for Gui and easy OpenGL initialization)
- Assimp (loading model)
- FFmpeg (media codecs)
- stb_image
- glm

Expect the third-party libraries, there are some useful tools:
- Arcball, ArcballCamera

# Modules
- Core:
    - glm
    - extened_types
    - string operation
    - [ ] stb_image
    - stream, frame, input (for data streaming)
- OpenGL: (optional)
    - GUI: glad, SDL2, ImGui
    - Model:
        - Naive obj reader
        - Assimp (optional)
- FFmpeg  (optional)
    - Video reader: Audio tracks are read into a vector. Video tracks are kept synced.
- Vision: (optional)
    - [ ] OpenCV    (optional)
    - [ ] Dlib      (optional)
