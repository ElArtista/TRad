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
uniform sampler2D lightmap;

vec3 radiance(vec3 N, vec3 ws_pos, vec3 albedo)
{
    vec3 light_dir = normalize(light_pos - ws_pos);
    vec3 light_col = vec3(1.0, 1.0, 1.0);

    float dist = length(light_pos - ws_pos);
    float light_intensity = 30000;
    float attenuation = 1.0 / (dist * dist);

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

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 lmuv_dbg(vec2 lmuv)
{
    vec2 sz = vec2(128.0); // Virtual texture size
    vec2 st = fs_in.lmuv;
    float r = rand(vec2(floor(st.x * sz.x), floor(st.y * sz.y)));
    return vec3(r);
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
    } else if (mode == 2) {
        // Lightmap shading
        frag_color = vec4(texture(lightmap, fs_in.lmuv).rgb, 1.0);
    } else {
        // Normal shading
        frag_color = vec4(color, 1.0);
    }
}
