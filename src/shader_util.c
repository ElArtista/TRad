#include "shader_util.h"
#include <stdlib.h>
#include <stdio.h>
#include <glad/glad.h>

static void gl_check_last_compile_error(GLuint id, const char** err)
{
    /* Check if last compile was successful */
    GLint compileStatus;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        /* Gather the compile log size */
        GLint logLength;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength != 0) {
            /* Fetch and print log */
            GLchar* buf = malloc(logLength * sizeof(GLchar));
            glGetShaderInfoLog(id, logLength, 0, buf);
            *err = buf;
        }
    }
}

static void gl_check_last_link_error(GLuint id, const char** err)
{
    /* Check if last link was successful */
    GLint status;
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        /* Gather the link log size */
        GLint logLength;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength != 0) {
            /* Fetch and print log */
            GLchar* buf = malloc(logLength * sizeof(GLchar));
            glGetProgramInfoLog(id, logLength, 0, buf);
            *err = buf;
        }
    }
}

#define NSHDR_TYPES 3

unsigned int shader_from_srcs(const char* vs_src, const char* gs_src, const char* fs_src)
{
    const char* shader_sources[NSHDR_TYPES] = {vs_src, gs_src, fs_src};
    const GLenum shader_types[NSHDR_TYPES]  = {GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER};

    GLuint shader_objs[NSHDR_TYPES] = {};
    GLuint prog = glCreateProgram();

    const char** err = 0;
    for (unsigned int i = 0; i < NSHDR_TYPES; ++i) {
        if (!shader_sources[i])
            continue;
        shader_objs[i] = glCreateShader(shader_types[i]);
        glShaderSource(shader_objs[i], 1, &shader_sources[i], 0);
        glCompileShader(shader_objs[i]);
        gl_check_last_compile_error(shader_objs[i], err);
        if (!err)
            glAttachShader(prog, shader_objs[i]);
        else {
            for (unsigned int j = i; j > 0; --j)
                if (shader_objs[j])
                    glDeleteShader(shader_objs[j]);
            goto shader_error;
        }
    }

    glLinkProgram(prog);
    gl_check_last_link_error(prog, err);
    for (unsigned int i = 0; i < NSHDR_TYPES; ++i)
        if (shader_objs[i])
            glDeleteShader(shader_objs[i]);
    if (!err)
        return prog;

shader_error:
    glDeleteProgram(prog);
    fprintf(stderr, "Shader error: %s", *err);
    free(err);
    return 0;
}
