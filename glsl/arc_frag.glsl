#version 330 core
out vec4 FragColor;

in vec3 FragRGB;

void main()
{
    FragColor = vec4(FragRGB, 1.0);
}
