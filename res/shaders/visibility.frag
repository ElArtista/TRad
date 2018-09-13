#version 330 core
out vec3 item_id;
in vec2 uv;

void main()
{
    item_id = vec3(uv, 1.0);
}
