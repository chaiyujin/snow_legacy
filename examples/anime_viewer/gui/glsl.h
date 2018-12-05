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
"    uniform vec3        viewPos;"
"    uniform vec3        lightPosList[2];"
"    uniform vec3        lightColor;"
"    uniform sampler2D   texture_diffuse1;"

"    void main()"
"    {   "
"        float ambientStrength = 0.05;"
"        vec3 ambient = ambientStrength * lightColor;"
"        vec3 result = vec3(0.0);"
"        for (int i = 0; i < 2; ++i) {"
"           vec3 lightPos = lightPosList[i];"
"           vec3 norm = normalize(Normal);"
"           vec3 lightDir = normalize(lightPos - FragPos);"
"           vec3 viewDir    = normalize(viewPos - FragPos);"
"           vec3 halfwayDir = normalize(lightDir + viewDir);"
"           float diff = max(dot(norm, lightDir), 0.0);"
"           vec3 diffuse = diff * lightColor;"
"           float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);"
"           vec3 specular = vec3(0.1) * spec;"
"           result += (diffuse / 2.0 + specular);"
"        }"
"        FragColor = vec4(ambient + result, 1.0) * texture(texture_diffuse1, TexCoords);"
"    }";


static const std::string FRAG_NOTEX_GLSL =
"    out vec4 FragColor;"

"    in vec2 TexCoords;"
"    in vec3 FragPos;"
"    in vec3 Normal;"
"    uniform vec3        viewPos;"
"    uniform vec3        lightPosList[2];"
"    uniform vec3        lightColor;"
"    uniform sampler2D   texture_diffuse1;"

"    void main()"
"    {   "
"        float ambientStrength = 0.05;"
"        vec3 ambient = ambientStrength * lightColor;"
"        vec3 result = vec3(0.0);"
"        for (int i = 0; i < 2; ++i) {"
"           vec3 lightPos = lightPosList[i];"
"           vec3 norm = normalize(Normal);"
"           vec3 lightDir = normalize(lightPos - FragPos);"
"           vec3 viewDir    = normalize(viewPos - FragPos);"
"           vec3 halfwayDir = normalize(lightDir + viewDir);"
"           float diff = max(dot(norm, lightDir), 0.0);"
"           vec3 diffuse = diff * lightColor;"
"           float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);"
"           vec3 specular = vec3(0.1) * spec;"
"           result += (diffuse / 2.0 + specular);"
"        }"
"        FragColor = vec4(ambient + result, 1.0);"
"    }";
