#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
uniform vec3        viewPos;
uniform vec3        lightPosList[2];
uniform vec3        lightColor;
uniform sampler2D   texture_diffuse1;

void main()
{   
    float ambientStrength = 0.05;
    vec3 ambient = ambientStrength * lightColor;
    vec3 result = vec3(0.0);
    for (int i = 0; i < 2; ++i) {
        vec3 lightPos = lightPosList[i];
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 viewDir    = normalize(viewPos - FragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
        vec3 specular = vec3(0.1) * spec;
        result += (diffuse / 2.0 + specular);
    }
    FragColor = vec4(ambient + result, 1.0);
};
