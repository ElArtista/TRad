#version 330 core
layout (location = 0) in vec3 position;
layout (location = 3) in vec2 lm_uv;
out vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    uv = lm_uv;
    gl_Position = proj * view * model * vec4(position, 1.0);
}
