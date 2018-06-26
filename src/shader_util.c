#include "shader_util.h"
#include <stdlib.h>
#include <stdio.h>
#include <glad/glad.h>

/*---------------------------------------------------------------------------
 * Parsing Utils
 *---------------------------------------------------------------------------*/
/* Reads file from disk to memory allocating needed space */
static void* read_file_to_mem_buf(const char* fpath, size_t* buf_sz)
{
    /* Check file for existence */
    FILE* f = 0;
    f = fopen(fpath, "rb");
    if (!f)
        return 0;

    /* Gather size */
    fseek(f, 0, SEEK_END);
    long file_sz = ftell(f);
    if (file_sz == -1) {
        fclose(f);
        return 0;
    }

    /* Read contents into memory buffer */
    rewind(f);
    void* data_buf = calloc(1, file_sz + 1);
    fread(data_buf, 1, file_sz, f);
    fclose(f);

    /* Store buffer size */
    if (buf_sz)
        *buf_sz = file_sz;

    return data_buf;
}

static const char* shader_load_fsrc(const char* fpath)
{
    if (!fpath)
        return 0;
    return read_file_to_mem_buf(fpath, 0);
}

unsigned int shader_build(struct shader_attachment* attachments, size_t num_attachments)
{
    GLuint prog = glCreateProgram();
    for (size_t i = 0; i < num_attachments; ++i) {
        struct shader_attachment* sa = &attachments[i];
        if (sa->src) {
            GLuint s = glCreateShader(sa->type);
            glShaderSource(s, 1, &sa->src, 0);
            glCompileShader(s);
            glAttachShader(prog, s);
            glDeleteShader(s);
        }
    }
    glLinkProgram(prog);
    return prog;
}

unsigned int shader_load(struct shader_files* sf)
{
    const char* vs_src = shader_load_fsrc(sf->vs_loc);
    const char* gs_src = shader_load_fsrc(sf->gs_loc);
    const char* fs_src = shader_load_fsrc(sf->fs_loc);
    const char* cs_src = shader_load_fsrc(sf->cs_loc);
    unsigned int shdr = shader_build((struct shader_attachment[]){
        {GL_VERTEX_SHADER,   vs_src},
        {GL_GEOMETRY_SHADER, gs_src},
        {GL_FRAGMENT_SHADER, fs_src},
        {GL_COMPUTE_SHADER,  cs_src}}, 4);
    free((void*)vs_src);
    free((void*)gs_src);
    free((void*)fs_src);
    free((void*)cs_src);
    return shdr;
}
