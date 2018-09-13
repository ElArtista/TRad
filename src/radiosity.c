#include "radiosity.h"
#include <string.h>
#include <assert.h>
#include <math.h>
#include <linalgb.h>
#include "opengl.h"
#include "hemicube.h"
#include "shader_util.h"
#include <stdio.h>

#define array_length(a) (sizeof(a)/sizeof(a[0]))

static struct {
    unsigned int lm_width, lm_height;
    GLuint fbo;
    GLuint attributes_shdr;
    GLuint max_pass_shdr;
    GLuint vis_pass_shdr;
    GLuint radiosity_shdr;
    GLuint texel_set_shdr;
    GLuint radiosity_tex;
    GLuint unshot_tex;
    GLuint position_tex;
    GLuint normal_tex;
    GLuint albedo_tex;
    GLuint max_pass_buf;
    GLuint shooter_info_buf;
    struct hemicube_rndr hemi_rndr;
    int attrib_pass;
} st;

struct shooter_info {
    int texel[2];
    int padding0[2];
    float position[3];
    float padding1;
    float normal[3];
    float padding2;
    float unshot[4];
};

void radiosity_init(int width, int height)
{
    memset(&st, 0, sizeof(st));
    st.attrib_pass = 0;

    /* Store dimensions */
    st.lm_width  = width;
    st.lm_height = height;

    /* Load shaders */
    st.attributes_shdr = shader_load(&(struct shader_files){
        .vs_loc = "res/shaders/attributes.vert",
        .gs_loc = "res/shaders/attributes.geom",
        .fs_loc = "res/shaders/attributes.frag"});

    st.max_pass_shdr = shader_load(&(struct shader_files){
        .cs_loc = "res/shaders/max.comp"});

    st.vis_pass_shdr = shader_load(&(struct shader_files){
        .vs_loc = "res/shaders/visibility.vert",
        .fs_loc = "res/shaders/visibility.frag"});

    st.radiosity_shdr = shader_load(&(struct shader_files){
        .cs_loc = "res/shaders/radiosity.comp"});

    st.texel_set_shdr = shader_load(&(struct shader_files){
        .cs_loc = "res/shaders/texel_set.comp"});

    /* Create framebuffer */
    glGenFramebuffers(1, &st.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, st.fbo);

    /* Create data textures */
    struct {
        GLuint* id;
        GLint ifmt;
        GLenum fmt;
        GLenum pix_dtype;
        GLenum attachment;
    } data_texs[] = {
        {
            &st.radiosity_tex,
            GL_RGBA16F,
            GL_RGBA,
            GL_FLOAT,
            GL_COLOR_ATTACHMENT0
        },
        {
            &st.unshot_tex,
            GL_RGBA16F,
            GL_RGBA,
            GL_FLOAT,
            GL_COLOR_ATTACHMENT1
        },
        {
            &st.position_tex,
            GL_RGBA16F,
            GL_RGBA,
            GL_FLOAT,
            GL_COLOR_ATTACHMENT2
        },
        {
            &st.normal_tex,
            GL_RGB16F,
            GL_RGB,
            GL_FLOAT,
            GL_COLOR_ATTACHMENT3
        },
        {
            &st.albedo_tex,
            GL_RGB8,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            GL_COLOR_ATTACHMENT4
        }
    };
    for (size_t i = 0; i < array_length(data_texs); ++i) {
        glGenTextures(1, data_texs[i].id);
        GLenum target = GL_TEXTURE_2D;
        glBindTexture(target, *(data_texs[i].id));
        glTexImage2D(target, 0,
                     data_texs[i].ifmt,
                     width, height, 0,
                     data_texs[i].fmt,
                     data_texs[i].pix_dtype, 0);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, data_texs[i].attachment, target, *(data_texs[i].id), 0);
    }
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    /* Create shader buffer for shooter seletion pass */
    const size_t num_work_groups = ceil(st.lm_width / 16) * ceil(st.lm_height / 16);
    glGenBuffers(1, &st.max_pass_buf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, st.max_pass_buf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, num_work_groups * (2 * sizeof(int) + sizeof(float)), 0, GL_DYNAMIC_COPY);

    /* Create shader buffer for the shooter info */
    glGenBuffers(1, &st.shooter_info_buf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, st.shooter_info_buf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(struct shooter_info), 0, GL_DYNAMIC_COPY);

    /* Initialize hemicube renderer instance */
    hemicube_rndr_init(&st.hemi_rndr);

    /* Unbind stuff */
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void radiosity_destroy()
{
    hemicube_rndr_destroy(&st.hemi_rndr);
    glDeleteBuffers(1, &st.shooter_info_buf);
    glDeleteBuffers(1, &st.max_pass_buf);
    GLuint textures[] = {
        st.radiosity_tex,
        st.unshot_tex,
        st.position_tex,
        st.normal_tex,
        st.albedo_tex
    };
    glDeleteTextures(array_length(textures), textures);
    glDeleteFramebuffers(1, &st.fbo);
    glDeleteProgram(st.texel_set_shdr);
    glDeleteProgram(st.radiosity_shdr);
    glDeleteProgram(st.vis_pass_shdr);
    glDeleteProgram(st.max_pass_shdr);
    glDeleteProgram(st.attributes_shdr);
}

static struct {
    GLuint prev_fbo;
    GLint prev_vp[4];
} attrib_pass;

void radiosity_attrib_pass_begin()
{
    if (st.attrib_pass)
        return;

    /* Store previous values */
    glGetIntegerv(GL_VIEWPORT, attrib_pass.prev_vp);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&attrib_pass.prev_fbo);

    glViewport(0, 0, st.lm_width, st.lm_height);
    glBindFramebuffer(GL_FRAMEBUFFER, st.fbo);
    GLuint attachments[] = {
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4,
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
    };
    glDrawBuffers(array_length(attachments), attachments);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(st.attributes_shdr);
}

void radiosity_attrib_pass_end()
{
    if (st.attrib_pass)
        return;

    /* Restore previous values */
    GLint* vp = attrib_pass.prev_vp;
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, attrib_pass.prev_fbo);
    st.attrib_pass = 1;
}

void radiosity_next_shooter_pass()
{
    GLuint shdr = st.max_pass_shdr;
    glUseProgram(shdr);

    GLuint data_tex[] = {
        st.position_tex,
        st.normal_tex,
    };
    for (unsigned int i = 0; i < array_length(data_tex); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, data_tex[i]);
    }

    glBindImageTexture(0, st.unshot_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, st.max_pass_buf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, st.shooter_info_buf);
    glUniform1i(glGetUniformLocation(shdr, "pass"), 0);
    glDispatchCompute(ceil(st.lm_width / 16), ceil(st.lm_height / 16), 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS); /* TODO: Use proper barrier */
    glUniform1i(glGetUniformLocation(shdr, "pass"), 1);
    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS); /* TODO: Use proper barrier */
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUseProgram(0);
}

static struct {
    GLuint prev_fbo;
    GLint prev_vp[4];
    GLint cur_face;
    mat4 view_proj[5];
} vis_pass;

static struct shooter_info si;

void radiosity_visibility_pass_begin()
{
    /* Store previous values */
    glGetIntegerv(GL_VIEWPORT, vis_pass.prev_vp);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&vis_pass.prev_fbo);

    /* Gather next shooter info to construct view matrix */
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, st.shooter_info_buf);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    memcpy(&si, p, sizeof(si));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    /*
    printf("(%.2f %.2f %.2f), (%.2f %.2f %.2f)\n",
            si.position[0], si.position[1], si.position[2],
            si.normal[0], si.normal[1], si.normal[2]);
     */

    /* Render shooter visibility texture */
    hemicube_rndr_clear(&st.hemi_rndr);

    /* Push it up a notch ?? */
    /*
    vec3 nnm = vec3_normalize(*(vec3*)si.normal);
    vec3 npos = vec3_add(*(vec3*)si.position, vec3_mul(nnm, 0.05));
    hemicube_render_begin(&st.hemi_rndr, npos.xyz, nnm.xyz);
     */
    hemicube_render_begin(&st.hemi_rndr, si.position, si.normal);
    glUseProgram(st.vis_pass_shdr);
    vis_pass.cur_face = 0;
}

int radiosity_visibility_pass_next()
{
    mat4 modl = mat4_id();
    mat4 sview, sproj;
    int r = hemicube_render_next(&st.hemi_rndr, &sview, &sproj);
    if (!r)
        return 0;
    GLuint shdr = st.vis_pass_shdr;
    glUniformMatrix4fv(glGetUniformLocation(shdr, "proj"), 1, GL_FALSE, sproj.m);
    glUniformMatrix4fv(glGetUniformLocation(shdr, "view"), 1, GL_FALSE, sview.m);
    glUniformMatrix4fv(glGetUniformLocation(shdr, "model"), 1, GL_FALSE, modl.m);
    vis_pass.view_proj[vis_pass.cur_face++] = mat4_mul_mat4(sproj, sview);
    return r;
}

void radiosity_visibility_pass_end()
{
    hemicube_render_end(&st.hemi_rndr);
    glUseProgram(0);

    /* Restore previous values */
    GLint* vp = vis_pass.prev_vp;
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, vis_pass.prev_fbo);
}

void radiosity_light_transfer_pass()
{
    GLuint shdr = st.radiosity_shdr;
    glUseProgram(shdr);

    GLuint data_tex[] = {
        st.position_tex,
        st.normal_tex,
        st.albedo_tex,
        st.hemi_rndr.col_tex
    };
    for (unsigned int i = 0; i < array_length(data_tex); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, data_tex[i]);
    }

    glUniformMatrix4fv(glGetUniformLocation(shdr, "view_proj"), 5, GL_FALSE, (GLvoid*)vis_pass.view_proj);
    glBindImageTexture(0, st.radiosity_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    glBindImageTexture(1, st.unshot_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, st.shooter_info_buf);
    glDispatchCompute(ceil(st.lm_width / 16), ceil(st.lm_height / 16), 1);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUseProgram(0);

    /* TODO: Check what is necessary */
    glTextureBarrier();
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    shdr = st.texel_set_shdr;
    glUseProgram(shdr);
    glUniform2iv(glGetUniformLocation(shdr, "coords"), 1, si.texel);
    glUniform4fv(glGetUniformLocation(shdr, "val"), 1, (float[]){0.0, 0.0, 0.0, 0.0});
    glBindImageTexture(0, st.unshot_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    glDispatchCompute(1, 1, 1);
    glUseProgram(0);
}

void radiosity_gi_pass_begin()
{
    radiosity_next_shooter_pass();
    radiosity_visibility_pass_begin();
}

int radiosity_gi_pass_next()
{
    return radiosity_visibility_pass_next();
}

void radiosity_gi_pass_end()
{
    radiosity_visibility_pass_end();
    radiosity_light_transfer_pass();
}

unsigned int radiosity_lightmap() { return st.radiosity_tex; }
unsigned int radiosity_unshot() { return st.unshot_tex; }
unsigned int radiosity_visibility() { return st.hemi_rndr.col_tex; }
