#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normal;

void main()
{
    gl_Position = projection * view * model * vec4(aPosition, 1.0f);
    vs_out.FragPos = vec3(model * vec4(aPosition, 1.0));   
    vs_out.TexCoords = aTexCoords;

    vec3 T = normalize(vec3(normal * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(normal * vec4(aNormal, 0.0)));
    T = normalize(T - dot(T, N) * N); // re-orthogonalize T with respect to N
    // then retrieve perpendicular vector B with the cross product of T and N
    vec3 B = cross(T, N);

    vs_out.TBN = mat3(T, B, N);
}
