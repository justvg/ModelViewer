#version 330 core
layout (location = 0) in vec3 aP;
layout (location = 1) in vec3 aN;
layout (location = 2) in vec2 aUV;

uniform mat4 Projection = mat4(1.0);
uniform mat4 View = mat4(1.0);
uniform mat4 Model = mat4(1.0);

out vec2 TexCoords;

void main()
{
    TexCoords = aUV;
    gl_Position = Projection * View * Model * vec4(aP, 1.0);
}
