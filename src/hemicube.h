/*********************************************************************************************************************/
/*                                                  /===-_---~~~~~~~~~------____                                     */
/*                                                 |===-~___                _,-'                                     */
/*                  -==\\                         `//~\\   ~~~~`---.___.-~~                                          */
/*              ______-==|                         | |  \\           _-~`                                            */
/*        __--~~~  ,-/-==\\                        | |   `\        ,'                                                */
/*     _-~       /'    |  \\                      / /      \      /                                                  */
/*   .'        /       |   \\                   /' /        \   /'                                                   */
/*  /  ____  /         |    \`\.__/-~~ ~ \ _ _/'  /          \/'                                                     */
/* /-'~    ~~~~~---__  |     ~-/~         ( )   /'        _--~`                                                      */
/*                   \_|      /        _)   ;  ),   __--~~                                                           */
/*                     '~~--_/      _-~/-  / \   '-~ \                                                               */
/*                    {\__--_/}    / \\_>- )<__\      \                                                              */
/*                    /'   (_/  _-~  | |__>--<__|      |                                                             */
/*                   |0  0 _/) )-~     | |__>--<__|     |                                                            */
/*                   / /~ ,_/       / /__>---<__/      |                                                             */
/*                  o o _//        /-~_>---<__-~      /                                                              */
/*                  (^(~          /~_>---<__-      _-~                                                               */
/*                 ,/|           /__>--<__/     _-~                                                                  */
/*              ,//('(          |__>--<__|     /                  .----_                                             */
/*             ( ( '))          |__>--<__|    |                 /' _---_~\                                           */
/*          `-)) )) (           |__>--<__|    |               /'  /     ~\`\                                         */
/*         ,/,'//( (             \__>--<__\    \            /'  //        ||                                         */
/*       ,( ( ((, ))              ~-__>--<_~-_  ~--____---~' _/'/        /'                                          */
/*     `~/  )` ) ,/|                 ~-_~>--<_/-__       __-~ _/                                                     */
/*   ._-~//( )/ )) `                    ~~-'_/_/ /~~~~~~~__--~                                                       */
/*    ;'( ')/ ,)(                              ~~~~~~~~~~                                                            */
/*   ' ') '( (/                                                                                                      */
/*     '   '  `                                                                                                      */
/*********************************************************************************************************************/
#ifndef _HEMICUBE_H_
#define _HEMICUBE_H_

#include "linalgb.h"

#define HEMICUBE_SRES 128

enum hemicube_face {
    HF_POSITIVE_X = 0,
    HF_NEGATIVE_X,
    HF_POSITIVE_Y,
    HF_NEGATIVE_Y,
    HF_NEGATIVE_Z,
    HF_MAX
};

struct hemicube_rndr {
    unsigned int fbo;
    unsigned int col_tex;
    unsigned int depth_rb;
    struct {
        struct {
            int vp[4];
            int scissor_test;
        } prev;
        enum hemicube_face cur_face;
        float pos[3], norm[3];
    } run_st;
};

void hemicube_rndr_init(struct hemicube_rndr* hr);
void hemicube_render_begin(struct hemicube_rndr* hr, const float pos[3], const float norm[3]);
int hemicube_render_next(struct hemicube_rndr* hr, mat4* view, mat4* proj);
void hemicube_render_end(struct hemicube_rndr* hr);
void hemicube_rndr_clear(struct hemicube_rndr* hr);
void hemicube_rndr_destroy(struct hemicube_rndr* hr);

#endif /* ! _HEMICUBE_H_ */
