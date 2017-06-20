#include "hemicube.h"
#include <math.h>
#include <assert.h>
#include <string.h>
#include <glad/glad.h>
#include "shader_util.h"

#define hres HEMICUBE_SRES

static const GLint scissors[5][4] = {
    { 3*hres/2,   hres/2, hres/2,   hres }, /* +x */
    {        0,   hres/2, hres/2,   hres }, /* -x */
    {   hres/2, 3*hres/2,   hres, hres/2 }, /* +y */
    {   hres/2,        0,   hres, hres/2 }, /* -y */
    {   hres/2,   hres/2,   hres,   hres }  /* -z */
};

static const GLint viewports[5][4] = {
    { 3*hres/2,   hres/2, hres, hres}, /* +x */
    {  -hres/2,   hres/2, hres, hres}, /* -x */
    {   hres/2, 3*hres/2, hres, hres}, /* +y */
    {   hres/2,  -hres/2, hres, hres}, /* -y */
    {   hres/2,   hres/2, hres, hres}  /* -z */
};

void hemicube_rndr_init(struct hemicube_rndr* hr)
{
    GLuint fbo, col_tex, depth_rb;

    /* Color buffer */
    glGenTextures(1, &col_tex);
    glBindTexture(GL_TEXTURE_2D, col_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, HEMICUBE_SRES * 2, HEMICUBE_SRES * 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    /* Depth buffer */
    glGenRenderbuffers(1, &depth_rb);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, HEMICUBE_SRES * 2, HEMICUBE_SRES * 2);

    /* Fbo */
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, col_tex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    /* Store handles */
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    hr->fbo = fbo;
    hr->col_tex = col_tex;
    hr->depth_rb = depth_rb;
}

void hemicube_render_begin(struct hemicube_rndr* hr, const float pos[3], const float norm[3])
{
    memcpy(hr->run_st.pos, pos, 3 * sizeof(float));
    memcpy(hr->run_st.norm, norm, 3 * sizeof(float));
    hr->run_st.cur_face = HF_POSITIVE_X;
    hr->run_st.prev.scissor_test = glIsEnabled(GL_SCISSOR_TEST);
    glGetIntegerv(GL_VIEWPORT, (GLint*)hr->run_st.prev.vp);
    glBindFramebuffer(GL_FRAMEBUFFER, hr->fbo);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.05f, 0.05f, 0.05f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
}

static void calc_vp_face_matrices(mat4* view, mat4* proj, enum hemicube_face face, vec3 eye, vec3 norm)
{
    vec3 ffront, fup, fright;
    ffront = norm;
    fright = vec3_new(1, 0, 0);
    fup    = vec3_cross(fright, ffront);
    fright = vec3_cross(ffront, fup);

    vec3 up = fup, right = fright, front = ffront;
    vec3 lfronts[] = { right, vec3_neg(right), up, vec3_neg(up), front };
    vec3 lups[] = { up, up, vec3_neg(front), front, up };

    vec3 lfront = lfronts[face], lup = lups[face];
    vec3 target = vec3_add(eye, lfront);
    *view = mat4_view_look_at(eye, target, lup);
    *proj = mat4_perspective(radians(90.0), 0.1, 3000.0, 1.0f);
}

int hemicube_render_next(struct hemicube_rndr* hr, mat4* view, mat4* proj)
{
    if (hr->run_st.cur_face >= HF_MAX)
        return 0;
    unsigned int idx = hr->run_st.cur_face++;
    calc_vp_face_matrices(view, proj, idx, *(vec3*)hr->run_st.pos, *(vec3*) hr->run_st.norm);
    GLint* vp = (GLint*) viewports[idx];
    GLint* sc = (GLint*) scissors[idx];
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    glScissor(sc[0], sc[1], sc[2], sc[3]);
    return 1;
}

void hemicube_render_end(struct hemicube_rndr* hr)
{
    GLint* vp = hr->run_st.prev.vp;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    glScissor(vp[0], vp[1], vp[2], vp[3]);
    if (hr->run_st.prev.scissor_test == GL_FALSE)
        glDisable(GL_SCISSOR_TEST);
}

void hemicube_rndr_destroy(struct hemicube_rndr* hr)
{
    glBindFramebuffer(GL_FRAMEBUFFER, hr->fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteRenderbuffers(1, &hr->depth_rb);
    glDeleteTextures(1, &hr->col_tex);
    glDeleteFramebuffers(1, &hr->fbo);
}
