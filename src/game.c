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
#include "opengl.h"
#include "shader_util.h"
#include "cornell_box.h"
#include "uvmap.h"
#include "glutil.h"
#include "hemicube.h"
#include "radiosity.h"

#define WND_TITLE "TRad"
#define WND_WIDTH 1280
#define WND_HEIGHT 720
#define LIGHTMAP_SIZE 128

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

static void load_cornell_box(GLuint* cb_vao, GLuint* cb_vbo, GLuint* cb_ebo, GLuint* cb_nrm, GLuint* cb_col, GLuint* cb_lm_uvs, GLuint* cb_num_indices, struct cornell_box* cbox)
{
    GLuint vao, vbo, nrm, col, lm_uvs, ebo;
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

    glGenBuffers(1, &lm_uvs);
    glBindBuffer(GL_ARRAY_BUFFER, lm_uvs);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * cbox->num_lmuvs, cbox->lmuvs, GL_STATIC_DRAW);
    GLuint lm_uvs_attrib = 3;
    glEnableVertexAttribArray(lm_uvs_attrib);
    glVertexAttribPointer(lm_uvs_attrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

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
    *cb_lm_uvs = lm_uvs;
    *cb_num_indices = cbox->num_indices;
}

static void free_cornell_box(GLuint* cb_vao, GLuint* cb_vbo, GLuint* cb_ebo, GLuint* cb_nrm, GLuint* cb_col, GLuint* cb_lm_uvs)
{
    glDeleteBuffers(1, cb_lm_uvs);
    glDeleteBuffers(1, cb_col);
    glDeleteBuffers(1, cb_nrm);
    glDeleteBuffers(1, cb_ebo);
    glDeleteBuffers(1, cb_vbo);
    glDeleteVertexArrays(1, cb_vao);
}

static void unpack_attrib(float* attrib_out, float* attrib_in, size_t attrib_sz, unsigned int* indices, size_t num_indices)
{
    for (size_t i = 0; i < num_indices; ++i) {
        unsigned int ind = indices[i];
        float* from = (float*)(((unsigned char*)attrib_in) + attrib_sz * ind);
        float* to = (float*)(((unsigned char*)attrib_out) + attrib_sz * i);
        memcpy(to, from, attrib_sz);
    }
}

/* Must free output cornell box's data */
static void unpack_cornell_box(struct cornell_box* cbout, struct cornell_box* cbin)
{
    cbout->num_indices  = cbin->num_indices;
    cbout->num_vertices = cbin->num_indices * 3;
    cbout->num_colors   = cbin->num_indices * 3;
    cbout->num_normals  = cbin->num_indices * 3;
    cbout->vertices     = malloc(cbout->num_vertices * sizeof(float) * 3);
    cbout->colors       = malloc(cbout->num_colors   * sizeof(float) * 3);
    cbout->normals      = malloc(cbout->num_normals  * sizeof(float) * 3);
    cbout->indices      = malloc(cbout->num_indices  * sizeof(unsigned int));
    unpack_attrib(cbout->vertices, cbin->vertices, sizeof(float) * 3, cbin->indices, cbin->num_indices);
    unpack_attrib(cbout->colors,   cbin->colors,   sizeof(float) * 3, cbin->indices, cbin->num_indices);
    unpack_attrib(cbout->normals,  cbin->normals,  sizeof(float) * 3, cbin->indices, cbin->num_indices);
    for (size_t i = 0; i < cbin->num_indices; ++i)
        cbout->indices[i] = i;
}

static void free_upacked_cornell_box(struct cornell_box* cbox)
{
    free(cbox->vertices);
    free(cbox->colors);
    free(cbox->normals);
    free(cbox->indices);
}

static void opengl_err_cb(void* ud, const char* msg)
{
    struct game_context* ctx = ud;
    (void)ctx;
    fprintf(stderr, "%s\n", msg);
    assert(0);
}

static void on_key(struct window* wnd, int key, int scancode, int action, int mods)
{
    (void)scancode; (void)mods;
    struct game_context* ctx = window_get_userdata(wnd);
    if (action == 0 && key == KEY_ESCAPE)
        *(ctx->should_terminate) = 1;
    if (action == KEY_ACTION_RELEASE && key == KEY_SPACE)
        ctx->rndr_mode = ctx->rndr_mode < 2 ? ctx->rndr_mode + 1 : 0;
}

void game_init(struct game_context* ctx)
{
    /* Create window */
    const char* title = WND_TITLE;
    int width = WND_WIDTH, height = WND_HEIGHT, mode = 0;
    ctx->wnd = window_create(title, width, height, mode, (struct context_params){OPENGL, {3, 3}, 1});

    /* Assosiate context to be accessed from callback functions */
    window_set_userdata(ctx->wnd, ctx);

    /* Set event callbacks */
    struct window_callbacks wnd_callbacks;
    memset(&wnd_callbacks, 0, sizeof(struct window_callbacks));
    wnd_callbacks.key_cb = on_key;
    window_set_callbacks(ctx->wnd, &wnd_callbacks);

    /* Setup OpenGL debug handler */
    opengl_register_error_handler(opengl_err_cb, ctx);

    /* Bundle cornell box data */
    struct cornell_box cbox_packed;
    cbox_packed.vertices     = (float*) cornell_box_vertices;
    cbox_packed.num_vertices = (sizeof(cornell_box_vertices) / sizeof(cornell_box_vertices[0])) / 3;
    cbox_packed.colors       = (float*) cornell_box_colors;
    cbox_packed.num_colors   = (sizeof(cornell_box_colors) / sizeof(cornell_box_colors[0])) / 3;
    cbox_packed.normals      = (float*) cornell_box_normals;
    cbox_packed.num_normals  = (sizeof(cornell_box_normals) / sizeof(cornell_box_normals[0])) / 3;
    cbox_packed.indices      = (unsigned int*) cornell_box_indices;
    cbox_packed.num_indices  = sizeof(cornell_box_indices) / sizeof(cornell_box_indices[0]);

    /* Unpack cbox */
    struct cornell_box cbox_unpacked;
    unpack_cornell_box(&cbox_unpacked, &cbox_packed);
    struct cornell_box cbox = cbox_unpacked;

    /* Generate lightmap uvs */
    cbox.num_lmuvs = cbox.num_vertices;
    cbox.lmuvs = calloc(cbox.num_lmuvs, sizeof(vec2));
    uvmap_planar_project(
        (vec2*) cbox.lmuvs,
        (vec3*) cbox.vertices,
        (vec3*) cbox.normals,
        cbox.num_vertices,
        cbox.indices,
        cbox.num_indices,
        LIGHTMAP_SIZE, LIGHTMAP_SIZE, 2);

    /* Load model */
    load_cornell_box(
        &ctx->mesh.vao,
        &ctx->mesh.vbo,
        &ctx->mesh.ebo,
        &ctx->mesh.nrm,
        &ctx->mesh.col,
        &ctx->mesh.lm_uvs,
        &ctx->mesh.num_indices,
        &cbox
    );
    free(cbox.lmuvs);
    free_upacked_cornell_box(&cbox_unpacked);

    /* Load shader */
    ctx->shdr = shader_load(&(struct shader_files){
        .vs_loc = "res/shaders/standard.vert",
        .fs_loc = "res/shaders/standard.frag"});

    /* GLutils */
    glutil_init();

    /* Hemicube renderer */
    ctx->hc_rndr = calloc(1, sizeof(struct hemicube_rndr));
    hemicube_rndr_init(ctx->hc_rndr);

    /* Radiosity renderer */
    const int lightmap_res = LIGHTMAP_SIZE;
    radiosity_init(lightmap_res, lightmap_res);
}

void game_update(void* userdata, float dt)
{
    (void) dt;
    struct game_context* ctx = userdata;
    /* Process input events */
    window_update(ctx->wnd);
}

static void render_scene(struct game_context* ctx, mat4* view, mat4* proj)
{
    glEnable(GL_DEPTH_TEST);
    mat4 model = mat4_id();

    GLuint shdr = ctx->shdr;
    glUseProgram(shdr);
    glUniformMatrix4fv(glGetUniformLocation(shdr, "proj"), 1, GL_FALSE, proj->m);
    glUniformMatrix4fv(glGetUniformLocation(shdr, "view"), 1, GL_FALSE, view->m);
    glUniformMatrix4fv(glGetUniformLocation(shdr, "model"), 1, GL_FALSE, model.m);
    glUniform3fv(glGetUniformLocation(shdr, "view_pos"), 1, cornell_box_cam_pos);
    vec3 light_pos = {{ .x = 278, .y = 450, .z = 279.5}};
    glUniform3fv(glGetUniformLocation(shdr, "light_pos"), 1, light_pos.xyz);
    glUniform1i(glGetUniformLocation(ctx->shdr, "mode"), ctx->rndr_mode);
    glUniform1i(glGetUniformLocation(ctx->shdr, "lm_mode"), 0);
    glUniform1i(glGetUniformLocation(ctx->shdr, "lightmap"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, radiosity_lightmap());

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(ctx->mesh.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->mesh.ebo);
    glDrawElements(GL_TRIANGLES, ctx->mesh.num_indices, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static inline void render_lightmap_preview(struct game_context* ctx)
{
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(ctx->shdr);
    glUniform1i(glGetUniformLocation(ctx->shdr, "lm_mode"), 1);
    glBindVertexArray(ctx->mesh.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->mesh.ebo);
    glDrawElements(GL_TRIANGLES, ctx->mesh.num_indices, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

static inline void render_hemicube_preview(struct game_context* ctx)
{
    const float pos[3] = {278, 373, 450}, to[3] = {278, -200, 0};
    vec3 normal = vec3_normalize(vec3_sub(*(vec3*)to, *(vec3*)pos));

    /* Render hemicube to texture */
    hemicube_rndr_clear(ctx->hc_rndr);
    hemicube_render_begin(ctx->hc_rndr, pos, normal.rgb);
    mat4 sview, sproj;
    while (hemicube_render_next(ctx->hc_rndr, &sview, &sproj))
        render_scene(ctx, &sview, &sproj);
    hemicube_render_end(ctx->hc_rndr);

    /* Preview texture */
    render_texture(ctx->hc_rndr->col_tex);
}

void game_render(void* userdata, float interpolation)
{
    (void) interpolation;
    struct game_context* ctx = userdata;

    /* Scene render */
    mat4 proj = mat4_perspective(radians(40.0), 0.1, 3000.0, (float)WND_WIDTH / WND_HEIGHT);
    mat4 view = mat4_view_look_at(*(vec3*)cornell_box_cam_pos, *(vec3*)cornell_box_cam_to, *(vec3*)cornell_box_cam_up);
    render_scene(ctx, &view, &proj);

    /* Attribute buffer for radiosity */
    radiosity_attrib_pass {
        glBindVertexArray(ctx->mesh.vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->mesh.ebo);
        glDrawElements(GL_TRIANGLES, ctx->mesh.num_indices, GL_UNSIGNED_INT, 0);
    }

    /* Progress solution */
    for (int i = 0; i < 100; ++i)
    radiosity_gi_pass {
        glBindVertexArray(ctx->mesh.vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->mesh.ebo);
        glDrawElements(GL_TRIANGLES, ctx->mesh.num_indices, GL_UNSIGNED_INT, 0);
    };

    /* Start rendering mini-previews */
    GLint default_vp[4] = {0};
    glGetIntegerv(GL_VIEWPORT, default_vp);
    glEnable(GL_SCISSOR_TEST);
    GLint prv_w = WND_WIDTH / 5.0f, prv_h = WND_HEIGHT / 5.0f;

    /* Render mini-previews */
    for (size_t preview_idx = 0; preview_idx < 4; ++preview_idx) {
        /* Preview viewport setup */
        GLint new_vp[4] = {10, 10 + (10 + prv_h) * preview_idx, prv_w, prv_h};
        glViewport(new_vp[0], new_vp[1], new_vp[2], new_vp[3]);
        glScissor(new_vp[0], new_vp[1], new_vp[2], new_vp[3]);
        /* Preview render */
        switch(preview_idx) {
            case 0:
                render_lightmap_preview(ctx);
                break;
            case 1:
                render_hemicube_preview(ctx);
                break;
            case 2:
                render_texture(radiosity_lightmap());
                break;
            case 3:
                render_texture(radiosity_unshot());
                //render_texture(radiosity_visibility());
                break;
        }
    }

    /* End rendering mini-previews */
    glDisable(GL_SCISSOR_TEST);
    glScissor(default_vp[0], default_vp[1], default_vp[2], default_vp[3]);
    glViewport(default_vp[0], default_vp[1], default_vp[2], default_vp[3]);

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
    radiosity_destroy();
    hemicube_rndr_destroy(ctx->hc_rndr);
    free(ctx->hc_rndr);
    glutil_deinit();
    /* Free mesh */
    free_cornell_box(&ctx->mesh.vao, &ctx->mesh.vbo, &ctx->mesh.ebo, &ctx->mesh.nrm, &ctx->mesh.col, &ctx->mesh.lm_uvs);
    memset(&ctx->mesh, 0, sizeof(ctx->mesh));
    /* Close window */
    window_destroy(ctx->wnd);
}
