#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aRGB;

out vec3 FragRGB;

uniform mat4 model;

void main()
{
    gl_Position = model * vec4(aPos, 1.0);
    FragRGB = aRGB * 0.8;
}
