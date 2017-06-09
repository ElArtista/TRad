#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <prof.h>
#include <glad/glad.h>
#include <gfxwnd/window.h>
#include <linalgb.h>
#include "shader_util.h"
#include "cornell_box.h"
#include "uvmap.h"

#define WND_TITLE "TRad"
#define WND_WIDTH 1280
#define WND_HEIGHT 720

static const char* vs_src = GLSRC(
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;

out VS_OUT {
    vec3 ws_pos;
    vec3 normal;
    vec3 color;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    vs_out.ws_pos = (model * vec4(position, 1.0)).xyz;
    vs_out.normal = normal;
    vs_out.color = color;
    gl_Position = proj * view * model * vec4(position, 1.0);
}
);

static const char* fs_src = GLSRC(
out vec4 frag_color;

in VS_OUT {
    vec3 ws_pos;
    vec3 normal;
    vec3 color;
} fs_in;

uniform vec3 view_pos;
uniform vec3 light_pos;

void main()
{
    vec3 N = normalize((fs_in.normal));
    vec3 light_dir = normalize(light_pos - fs_in.ws_pos);
    vec3 light_col = vec3(1.0, 1.0, 1.0);

    float distance = length(light_pos - fs_in.ws_pos);
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
    vec3 V = normalize(view_pos - fs_in.ws_pos);
    vec3 R = reflect(-light_dir, N);

    float kD = max(dot(N, light_dir), 0.0);
    float kS = pow(max(dot(V, R), 0.0), 32);

    vec3 Lo = (kD + kS) * attenuation * light_col * light_intensity * fs_in.color;

    // Final color
    vec3 color = Lo;
#ifdef POSTPROCESSING
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Gamma correct
    color = pow(color, vec3(1.0 / 2.2));
#endif

    frag_color = vec4(color, 1.0);
}
);

static void APIENTRY gl_debug_proc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
{
    (void) source;
    (void) id;
    (void) severity;
    (void) length;
    (void) user_param;

    if (type == GL_DEBUG_TYPE_ERROR) {
        fprintf(stderr, "%s\n", message);
        assert(0);
    }
}

static void on_key(struct window* wnd, int key, int scancode, int action, int mods)
{
    (void)scancode; (void)mods;
    struct game_context* ctx = window_get_userdata(wnd);
    if (action == 0 && key == KEY_ESCAPE)
        *(ctx->should_terminate) = 1;
}

struct cornell_box {
    float* vertices;
    size_t num_vertices;
    float* colors;
    size_t num_colors;
    float* normals;
    size_t num_normals;
    float* lmuvs;
    size_t num_lmuvs;
    unsigned int* indices;
    size_t num_indices;
};

static void load_cornell_box(GLuint* cb_vao, GLuint* cb_vbo, GLuint* cb_ebo, GLuint* cb_nrm, GLuint* cb_col, GLuint* cb_num_indices, struct cornell_box* cbox)
{
    GLuint vao, vbo, nrm, col, ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * cbox->num_vertices, cbox->vertices, GL_STATIC_DRAW);
    GLuint pos_attrib = 0;
    glEnableVertexAttribArray(pos_attrib);
    glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glGenBuffers(1, &nrm);
    glBindBuffer(GL_ARRAY_BUFFER, nrm);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * cbox->num_normals, cbox->normals, GL_STATIC_DRAW);
    GLuint nrm_attrib = 1;
    glEnableVertexAttribArray(nrm_attrib);
    glVertexAttribPointer(nrm_attrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glGenBuffers(1, &col);
    glBindBuffer(GL_ARRAY_BUFFER, col);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * cbox->num_colors, cbox->colors, GL_STATIC_DRAW);
    GLuint col_attrib = 2;
    glEnableVertexAttribArray(col_attrib);
    glVertexAttribPointer(col_attrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLfloat) * cbox->num_indices, cbox->indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    *cb_vao = vao;
    *cb_vbo = vbo;
    *cb_ebo = ebo;
    *cb_nrm = nrm;
    *cb_col = col;
    *cb_num_indices = cbox->num_indices;
}

static void free_cornell_box(GLuint* cb_vao, GLuint* cb_vbo, GLuint* cb_ebo, GLuint* cb_nrm, GLuint* cb_col)
{
    glDeleteBuffers(1, cb_col);
    glDeleteBuffers(1, cb_nrm);
    glDeleteBuffers(1, cb_ebo);
    glDeleteBuffers(1, cb_vbo);
    glDeleteVertexArrays(1, cb_vao);
}

void game_init(struct game_context* ctx)
{
    /* Create window */
    const char* title = WND_TITLE;
    int width = WND_WIDTH, height = WND_HEIGHT, mode = 0;
    ctx->wnd = window_create(title, width, height, mode);

    /* Assosiate context to be accessed from callback functions */
    window_set_userdata(ctx->wnd, ctx);

    /* Set event callbacks */
    struct window_callbacks wnd_callbacks;
    memset(&wnd_callbacks, 0, sizeof(struct window_callbacks));
    wnd_callbacks.key_cb = on_key;
    window_set_callbacks(ctx->wnd, &wnd_callbacks);

    /* Setup OpenGL debug handler */
    glDebugMessageCallback(gl_debug_proc, ctx);

    /* Bundle cornell box data */
    struct cornell_box cbox;
    cbox.vertices     = (float*) cornell_box_vertices;
    cbox.num_vertices = sizeof(cornell_box_vertices) / sizeof(cornell_box_vertices[0]);
    cbox.colors       = (float*) cornell_box_colors;
    cbox.num_colors   = sizeof(cornell_box_colors) / sizeof(cornell_box_colors[0]);
    cbox.normals      = (float*) cornell_box_normals;
    cbox.num_normals  = sizeof(cornell_box_normals) / sizeof(cornell_box_normals[0]);
    cbox.indices      = (unsigned int*) cornell_box_indices;
    cbox.num_indices  = sizeof(cornell_box_indices) / sizeof(cornell_box_indices[0]);

    /* Generate lightmap uvs */
    cbox.num_lmuvs = cbox.num_vertices;
    cbox.lmuvs = malloc(cbox.num_lmuvs * sizeof(vec2));
    uvmap_planar_project(
        (vec2*) cbox.lmuvs,
        (vec3*) cbox.vertices,
        cbox.num_vertices,
        (vec3*) cbox.normals,
        cbox.indices,
        cbox.num_indices);

    /* Load model */
    load_cornell_box(
        &ctx->mesh.vao,
        &ctx->mesh.vbo,
        &ctx->mesh.ebo,
        &ctx->mesh.nrm,
        &ctx->mesh.col,
        &ctx->mesh.num_indices,
        &cbox
    );
    free(cbox.lmuvs);

    /* Load shader */
    ctx->shdr = shader_from_srcs(vs_src, 0, fs_src);
}

void game_update(void* userdata, float dt)
{
    (void) dt;
    struct game_context* ctx = userdata;
    /* Process input events */
    window_update(ctx->wnd);
}

void game_render(void* userdata, float interpolation)
{
    (void) interpolation;
    struct game_context* ctx = userdata;
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    /* Render */
    mat4 proj = mat4_perspective(radians(40.0), 0.1, 3000.0, (float)WND_WIDTH / WND_HEIGHT);
    mat4 view = mat4_view_look_at(*(vec3*)cornell_box_cam_pos, *(vec3*)cornell_box_cam_to, *(vec3*)cornell_box_cam_up);
    mat4 model = mat4_id();

    GLuint shdr = ctx->shdr;
    glUseProgram(shdr);
    glUniformMatrix4fv(glGetUniformLocation(shdr, "proj"), 1, GL_FALSE, proj.m);
    glUniformMatrix4fv(glGetUniformLocation(shdr, "view"), 1, GL_FALSE, view.m);
    glUniformMatrix4fv(glGetUniformLocation(shdr, "model"), 1, GL_FALSE, model.m);
    glUniform3fv(glGetUniformLocation(shdr, "view_pos"), 1, cornell_box_cam_pos);
    vec3 light_pos = {{ .x = cornell_box_cam_pos[0],
                        .y = cornell_box_cam_pos[1],
                        .z = cornell_box_cam_pos[2] * (-0.5)}};
    glUniform3fv(glGetUniformLocation(shdr, "light_pos"), 1, light_pos.xyz);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(ctx->mesh.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->mesh.ebo);
    glDrawElements(GL_TRIANGLES, ctx->mesh.num_indices, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* Show rendered contents from the backbuffer */
    window_swap_buffers(ctx->wnd);
}

void game_perf_update(void* userdata, float msec, float fps)
{
    struct game_context* ctx = userdata;
    char suffix_buf[64];
    snprintf(suffix_buf, sizeof(suffix_buf), "[Msec: %.2f / Fps: %.2f]", msec, fps);
    window_set_title_suffix(ctx->wnd, suffix_buf);
}

void game_shutdown(struct game_context* ctx)
{
    /* Free mesh */
    free_cornell_box(&ctx->mesh.vao, &ctx->mesh.vbo, &ctx->mesh.ebo, &ctx->mesh.nrm, &ctx->mesh.col);
    memset(&ctx->mesh, 0, sizeof(ctx->mesh));
    /* Close window */
    window_destroy(ctx->wnd);
}
