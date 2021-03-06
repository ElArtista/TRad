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
#ifndef _CORNELL_BOX_H_
#define _CORNELL_BOX_H_

/* The geometry has been measured from the physical Cornell Box.
 * The surfaces are therefore not perfectly perpendicular.
 * All surfaces are quadrilaterals. They are specified below by their vertices. */
static const float cornell_box_vertices[] = {
    /* Floor (white) */
    552.8,   0.0,   0.0,
      0.0,   0.0,   0.0,
      0.0,   0.0, 559.2,
    549.6,   0.0, 559.2,

    /* Holes (for disc. mesh) */
    130.0,   0.0,  65.0,
     82.0,   0.0, 225.0,
    240.0,   0.0, 272.0,
    290.0,   0.0, 114.0,

    423.0,   0.0, 247.0,
    265.0,   0.0, 296.0,
    314.0,   0.0, 456.0,
    472.0,   0.0, 406.0,

    /* Light (light) */
    343.0, 548.8, 227.0,
    343.0, 548.8, 332.0,
    213.0, 548.8, 332.0,
    213.0, 548.8, 227.0,

    /* Ceiling (white) */
    556.0, 548.8,   0.0,
    556.0, 548.8, 559.2,
      0.0, 548.8, 559.2,
      0.0, 548.8,   0.0,

    /* Hole (for disc. mesh) */
    343.0, 548.8, 227.0,
    343.0, 548.8, 332.0,
    213.0, 548.8, 332.0,
    213.0, 548.8, 227.0,

    /* Back wall (white) */
    549.6,   0.0, 559.2,
      0.0,   0.0, 559.2,
      0.0, 548.8, 559.2,
    556.0, 548.8, 559.2,

    /* Right wall (green) */
      0.0,   0.0, 559.2,
      0.0,   0.0,   0.0,
      0.0, 548.8,   0.0,
      0.0, 548.8, 559.2,

    /* Left wall (red) */
    552.8,   0.0,   0.0,
    549.6,   0.0, 559.2,
    556.0, 548.8, 559.2,
    556.0, 548.8,   0.0,

    /* Short block (white) */
    130.0, 165.0,  65.0,
     82.0, 165.0, 225.0,
    240.0, 165.0, 272.0,
    290.0, 165.0, 114.0,

    290.0,   0.0, 114.0,
    290.0, 165.0, 114.0,
    240.0, 165.0, 272.0,
    240.0,   0.0, 272.0,

    130.0,   0.0,  65.0,
    130.0, 165.0,  65.0,
    290.0, 165.0, 114.0,
    290.0,   0.0, 114.0,

     82.0,   0.0, 225.0,
     82.0, 165.0, 225.0,
    130.0, 165.0,  65.0,
    130.0,   0.0,  65.0,

    240.0,   0.0, 272.0,
    240.0, 165.0, 272.0,
     82.0, 165.0, 225.0,
     82.0,   0.0, 225.0,

    /* Tall block (white) */
    423.0, 330.0, 247.0,
    265.0, 330.0, 296.0,
    314.0, 330.0, 456.0,
    472.0, 330.0, 406.0,

    423.0,   0.0, 247.0,
    423.0, 330.0, 247.0,
    472.0, 330.0, 406.0,
    472.0,   0.0, 406.0,

    472.0,   0.0, 406.0,
    472.0, 330.0, 406.0,
    314.0, 330.0, 456.0,
    314.0,   0.0, 456.0,

    314.0,   0.0, 456.0,
    314.0, 330.0, 456.0,
    265.0, 330.0, 296.0,
    265.0,   0.0, 296.0,

    265.0,   0.0, 296.0,
    265.0, 330.0, 296.0,
    423.0, 330.0, 247.0,
    423.0,   0.0, 247.0
};

static const float cornell_box_colors[] = {
    /* Floor (white) */
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    /* Holes (for disc. mesh) */
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    /* Light (light) */
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    /* Ceiling (white) */
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    /* Hole (for disc. mesh) */
    1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0,
    /* Back wall (white) */
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    /* Right wall (green) */
    0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
    /* Left wall (red) */
    1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
    /* Short block (white) */
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    /* Tall block (white) */
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
};

static const float cornell_box_normals[] = {
     0.000000,  1.000000,  0.000000,  0.000000,  1.000000,  0.000000,  0.000000,  1.000000,  0.000000,  0.000000,  1.000000,  0.000000,
     0.000000,  1.000000, -0.000000,  0.000000,  1.000000, -0.000000,  0.000000,  1.000000, -0.000000,  0.000000,  1.000000, -0.000000,
     0.000000,  1.000000,  0.000000,  0.000000,  1.000000,  0.000000,  0.000000,  1.000000,  0.000000,  0.000000,  1.000000,  0.000000,
     0.000000, -1.000000,  0.000000,  0.000000, -1.000000,  0.000000,  0.000000, -1.000000,  0.000000,  0.000000, -1.000000,  0.000000,
     0.000000, -1.000000,  0.000000,  0.000000, -1.000000,  0.000000,  0.000000, -1.000000,  0.000000,  0.000000, -1.000000,  0.000000,
     0.000000, -1.000000,  0.000000,  0.000000, -1.000000,  0.000000,  0.000000, -1.000000,  0.000000,  0.000000, -1.000000,  0.000000,
     0.000000,  0.000000, -1.000000,  0.000000,  0.000000, -1.000000,  0.000000,  0.000000, -1.000000,  0.000000,  0.000000, -1.000000,
     1.000000,  0.000000,  0.000000,  1.000000,  0.000000,  0.000000,  1.000000,  0.000000,  0.000000,  1.000000,  0.000000,  0.000000,
    -0.999916,  0.011661, -0.005722, -0.999916,  0.011661, -0.005722, -0.999916,  0.011661, -0.005722, -0.999916,  0.011661, -0.005722,
     0.000000,  1.000000, -0.000000,  0.000000,  1.000000, -0.000000,  0.000000,  1.000000, -0.000000,  0.000000,  1.000000, -0.000000,
     0.953400, -0.000000,  0.301709,  0.953400, -0.000000,  0.301709,  0.953400, -0.000000,  0.301709,  0.953400, -0.000000,  0.301709,
     0.292826,  0.000000, -0.956166,  0.292826,  0.000000, -0.956166,  0.292826,  0.000000, -0.956166,  0.292826,  0.000000, -0.956166,
    -0.957826,  0.000000, -0.287348, -0.957826,  0.000000, -0.287348, -0.957826,  0.000000, -0.287348, -0.957826,  0.000000, -0.287348,
    -0.285121,  0.000000,  0.958492, -0.285121,  0.000000,  0.958492, -0.285121,  0.000000,  0.958492, -0.285121,  0.000000,  0.958492,
     0.000000,  1.000000,  0.000000,  0.000000,  1.000000,  0.000000,  0.000000,  1.000000,  0.000000,  0.000000,  1.000000,  0.000000,
     0.955649,  0.000000, -0.294508,  0.955649,  0.000000, -0.294508,  0.955649,  0.000000, -0.294508,  0.955649,  0.000000, -0.294508,
     0.301709, -0.000000,  0.953400,  0.301709, -0.000000,  0.953400,  0.301709, -0.000000,  0.953400,  0.301709, -0.000000,  0.953400,
    -0.956166,  0.000000,  0.292826, -0.956166,  0.000000,  0.292826, -0.956166,  0.000000,  0.292826, -0.956166,  0.000000,  0.292826,
    -0.296209,  0.000000, -0.955123, -0.296209,  0.000000, -0.955123, -0.296209,  0.000000, -0.955123, -0.296209,  0.000000, -0.955123
};

static const unsigned int cornell_box_indices[] = {
     0,  1,  2,
     2,  3,  0,
     4,  5,  6,
     6,  7,  4,
     8,  9, 10,
    10, 11,  8,
    12, 13, 14,
    14, 15, 12,
    16, 17, 18,
    18, 19, 16,
    20, 21, 22,
    22, 23, 20,
    24, 25, 26,
    26, 27, 24,
    28, 29, 30,
    30, 31, 28,
    32, 33, 34,
    34, 35, 32,
    36, 37, 38,
    38, 39, 36,
    40, 41, 42,
    42, 43, 40,
    44, 45, 46,
    46, 47, 44,
    48, 49, 50,
    50, 51, 48,
    52, 53, 54,
    54, 55, 52,
    56, 57, 58,
    58, 59, 56,
    60, 61, 62,
    62, 63, 60,
    64, 65, 66,
    66, 67, 64,
    68, 69, 70,
    70, 71, 68,
    72, 73, 74,
    74, 75, 72
};

/*
 * -= Camera parameters =-
 *  Position     278 273 -800
 *  Direction      0   0    1
 *  Up direction   0   1    0
 */
static const float cornell_box_cam_pos[3] = {278, 273, -800};
static const float cornell_box_cam_to[3] = {278, 273, 0};
static const float cornell_box_cam_up[3] = {0, 1, 0};

#endif /* ! _CORNELL_BOX_H_ */
