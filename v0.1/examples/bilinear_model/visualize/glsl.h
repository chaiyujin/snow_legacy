#pragma once
#include <string>

static const std::string VERT_GLSL =
"    layout (location = 0) in vec3 aPos;"
"    layout (location = 1) in vec3 aNormal;"
"    layout (location = 2) in vec2 aTexCoords;"

"    out vec2 TexCoords;"
"    out vec3 FragPos;"
"    out vec3 Normal;"

"    uniform mat4 model;"
"    uniform mat4 view;"
"    uniform mat4 projection;"
"    uniform mat4 normal;"

"    void main()"
"    {"
"        TexCoords = aTexCoords;  "  
"        gl_Position = projection * view * model * vec4(aPos, 1.0);"
"        FragPos = vec3(model * vec4(aPos, 1.0));"
"        Normal = mat3(normal) * aNormal;"
"    }";

static const std::string FRAG_GLSL =
"    out vec4 FragColor;"

"    in vec2 TexCoords;"
"    in vec3 FragPos;"
"    in vec3 Normal;"

"    uniform vec3        lightPos;"
"    uniform vec3        lightColor;"
"    uniform sampler2D   texture_diffuse1;"

"    void main()"
"    {   "
"        float ambientStrength = 0.1;"
"        vec3 ambient = ambientStrength * lightColor;"

"        vec3 norm = normalize(Normal);"
"        vec3 lightDir = normalize(lightPos - FragPos);"
"        float diff = max(dot(norm, lightDir), 0.0);"
"        vec3 diffuse = diff * lightColor;"

"        vec3 result = (ambient + diffuse);"
"        FragColor = vec4(result, 1.0) * texture(texture_diffuse1, TexCoords);"
"    }";

static const std::string FRAG_NOTEX_GLSL =
"    out vec4 FragColor;"

"    in vec2 TexCoords;"
"    in vec3 FragPos;"
"    in vec3 Normal;"

"    uniform vec3        lightPos;"
"    uniform vec3        lightColor;"
"    uniform sampler2D   texture_diffuse1;"

"    void main()"
"    {   "
"        float ambientStrength = 0.1;"
"        vec3 ambient = ambientStrength * lightColor;"

"        vec3 norm = normalize(Normal);"
"        vec3 lightDir = normalize(lightPos - FragPos);"
"        float diff = max(dot(norm, lightDir), 0.0);"
"        vec3 diffuse = diff * lightColor;"

"        vec3 result = (ambient + diffuse);"
"        FragColor = vec4(result, 1.0);"
"    }";
