#version 330 core
layout (location = 0) out vec4 position;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec3 albedo;
layout (location = 3) out vec3 radiosity;
layout (location = 4) out vec3 unshot;

in GS_OUT {
    // Vertex shader passthrough
    vec3 pos;
    vec3 nrm;
    vec3 col;
    vec2 luv;
    // Geometry shader outputs
    vec2 ndc_pos;
    vec4 ndc_aabb;
};

vec3 radiance(vec3 N, vec3 ws_pos, vec3 light_pos)
{
    vec3 light_dir = normalize(light_pos - ws_pos);
    vec3 light_col = vec3(1.0, 1.0, 1.0);

    float dist = length(light_pos - ws_pos);
    float light_intensity = 30000;
    float attenuation = 1.0 / (dist * dist);

    float kD = max(dot(N, light_dir), 0.0);
    vec3 Lo = kD * attenuation * light_col * light_intensity;
    return Lo;
}

void main()
{
    // Discard pixels, which are not inside the AABB.
    if (ndc_pos.x < ndc_aabb.x || ndc_pos.y < ndc_aabb.y || ndc_pos.x > ndc_aabb.z || ndc_pos.y > ndc_aabb.w)
        discard;

    // Store attributes
    position = vec4(pos, 1.0);
    normal = normalize(nrm);
    albedo = col;

    // Add direct light into unshot values
    vec3 light_pos = vec3(278, 450, 279.5);
    vec3 Lo = radiance(normal, position.rgb, light_pos);
    radiosity = vec3(0.0);
    unshot = Lo * albedo;
}
