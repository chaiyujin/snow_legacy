#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

struct Material {
    sampler2D diffuse1;
    sampler2D normal1;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform bool normalMapping;

#define NR_POINT_LIGHTS 4
uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords);

void main()
{
    // Obtain normal from normal map in range [0,1]
    vec3 normal = texture(material.normal1, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);   
    normal = normalize(fs_in.TBN * normal); // world space
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 result = vec3(0.0, 0.0, 0.0);
    result += CalcDirLight(dirLight, normal, viewDir, fs_in.TexCoords);
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir, fs_in.TexCoords);    
    FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse1, texCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse1, texCoords));
    vec3 specular = light.specular * spec; // * vec3(texture(material.specular, texCoords));
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse1, texCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse1, texCoords));
    vec3 specular = light.specular * spec; // * vec3(texture(material.specular, texCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
