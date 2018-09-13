#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 lm_uv;

out VS_OUT {
    vec3 pos;
    vec3 nrm;
    vec3 col;
    vec2 luv;
};

uniform mat4 model;

void main()
{
    pos = position;
    nrm = normal;
    col = color;
    luv = lm_uv;
    gl_Position = vec4(luv * 2.0 - 1.0, 0.0, 1.0);
}
