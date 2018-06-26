#include "glutil.h"
#include <glad/glad.h>
#include "shader_util.h"

static const char* rndr_tex_vs_src = GLSRC(
layout (location = 0) in vec3 position;
out vec2 uv;

void main()
{
    uv = position.xy * 0.5 + 0.5;
    gl_Position = vec4(position, 1.0);
}
);

static const char* rndr_tex_fs_src = GLSRC(
out vec4 color;
in vec2 uv;

uniform sampler2D tex;

void main()
{
    vec3 tc = texture(tex, uv).rgb;
    color = vec4(tc, 1.0);
}
);

static struct {
    GLuint quad_vao;
    GLuint quad_vbo;
    GLuint tex_shdr;
} st;

static void quad_create()
{
    static const GLfloat quad_vert[] =
    {
       -1.0f,  1.0f, 0.0f,
       -1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f
    };

    GLuint quad_vao;
    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);

    GLuint quad_vbo;
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vert), &quad_vert, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    st.quad_vao = quad_vao;
    st.quad_vbo = quad_vbo;
}

static void quad_destroy()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &st.quad_vbo);
    glDeleteVertexArrays(1, &st.quad_vao);
    st.quad_vao = 0;
    st.quad_vbo = 0;
}

void render_quad()
{
    glBindVertexArray(st.quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

static void texture_render_init()
{
    st.tex_shdr = shader_build((struct shader_attachment[]){
        {GL_VERTEX_SHADER,   rndr_tex_vs_src},
        {GL_FRAGMENT_SHADER, rndr_tex_fs_src}}, 2);
}

static void texture_render_destroy()
{
    glDeleteProgram(st.tex_shdr);
    st.tex_shdr = 0;
}

void render_texture(unsigned int tex)
{
    glUseProgram(st.tex_shdr);
    glUniform1i(glGetUniformLocation(st.tex_shdr, "tex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    render_quad();
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void glutil_init()
{
    quad_create();
    texture_render_init();
}

void glutil_deinit()
{
    texture_render_destroy();
    quad_destroy();
}
