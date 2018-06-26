#version 330 core
out vec4 frag_color;

in VS_OUT {
    vec3 ws_pos;
    vec3 normal;
    vec3 color;
    vec2 lmuv;
} fs_in;

uniform vec3 view_pos;
uniform vec3 light_pos;
uniform int mode;

vec3 radiance(vec3 N, vec3 ws_pos, vec3 albedo)
{
    vec3 light_dir = normalize(light_pos - ws_pos);
    vec3 light_col = vec3(1.0, 1.0, 1.0);

    float distance = length(light_pos - ws_pos);
#ifdef PBR_PLIGHT
    float light_intensity = 30000;
    float attenuation = 1.0 / (distance * distance);
#else
    float light_intensity = 1000;
    float light_constant = 1.0;
    float light_linear = 0.09;
    float light_quadratic = 0.032;
    float attenuation = 1.0 / (1.0 + (light_linear * distance) + (light_quadratic * distance * distance));
#endif
    vec3 V = normalize(view_pos - ws_pos);
    vec3 R = reflect(-light_dir, N);

    float kD = max(dot(N, light_dir), 0.0);
    float kS = pow(max(dot(V, R), 0.0), 32);

    vec3 Lo = (kD + kS) * attenuation * light_col * light_intensity * albedo;
    return Lo;
}

vec3 postprocess(vec3 color)
{
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Gamma correct
    color = pow(color, vec3(1.0 / 2.2));
    return color;
}

vec3 lmuv_dbg(vec2 lmuv)
{
    vec2 sz = vec2(128.0); // Virtual texture size
    vec2 st = fs_in.lmuv;
    float m = mod(floor(st.x * sz.x) + floor(st.y * sz.y), 2.0);
    vec3 col = m < 1.0 ? vec3(0.0) : vec3(1.0);
    return col;
}

void main()
{
    vec3 N = normalize((fs_in.normal));
    vec3 Lo = radiance(N, fs_in.ws_pos, fs_in.color);
    // Final color
    vec3 color = Lo;
#ifdef POSTPROCESSING
    color = postprocess(color);
#endif

    if (mode == 1) {
        // Visualize lightmap UVs
        frag_color = vec4(lmuv_dbg(fs_in.lmuv), 1.0);
    } else {
        // Normal shading
        frag_color = vec4(color, 1.0);
    }
}
