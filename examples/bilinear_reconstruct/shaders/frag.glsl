#version 330
in vec3  inFragPos;                                     
in vec3  inNormal;                                      
in vec2  inTexCoord;                                    
out vec4 outFragColor;                                                      

void main() {
    // ambient
    const float ambientStrength = 0.1;
    vec3 ambientColor = ambientStrength * gl_LightSource[0].ambient.xyz;
    // diffuse
    vec3 normal = normalize(inNormal);
    vec3 lightDir = normalize(gl_LightSource[0].position.xyz - inFragPos);
    vec3 diffuseColor = max(dot(normal, lightDir), 0) * gl_LightSource[0].diffuse.xyz;
    // ligth
    vec3 result = (ambientColor + diffuseColor);

    outFragColor = vec4(result, 1.0);
}
