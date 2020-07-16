#version 330 core
out vec4 FragCoord;

uniform sampler2D Albedo;

in vec2 TexCoords;

void main()
{
    FragCoord = vec4(texture(Albedo, TexCoords).rgb, 1.0);
}
