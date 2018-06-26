#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 lm_uv;

out VS_OUT {
    vec3 ws_pos;
    vec3 normal;
    vec3 color;
    vec2 lmuv;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform bool lm_mode;

void main()
{
    vs_out.ws_pos = (model * vec4(position, 1.0)).xyz;
    vs_out.normal = normal;
    vs_out.color = color;
    vs_out.lmuv = lm_uv;
    if (!lm_mode)
        gl_Position = proj * view * model * vec4(position, 1.0);
    else
        gl_Position = vec4(vs_out.lmuv * 2 - 1, 0.0, 1.0);
}
