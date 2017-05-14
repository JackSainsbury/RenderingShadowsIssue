#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 normal;


out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N; // This is the inverse transpose of the mv matrix

uniform mat4 MV;
uniform mat4 MVP;


void main()
{
    gl_Position = MVP * vec4(position, 1.0);;

    Normal = normalize(N * normal);

    TexCoords = texCoords;
    //FragPosLightSpace = lightSpaceMatrix * vec4(position, 1.0f);
}
