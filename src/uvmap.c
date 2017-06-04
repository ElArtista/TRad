#include "uvmap.h"
#include <stdio.h>
#include <math.h>
#include <vector.h>

struct quadrilateral {
    vec2 edge, offset;
    vec2 p[4];
};

struct node {
    struct node* childs[2];
    vec4 rect;
};

static struct node* node_insert(struct node* n, struct quadrilateral* q)
{
    /* If the node has no children, try to recursively insert into them */
    if (n->childs[0] && n->childs[1]) {
        struct node* c = node_insert(n->childs[0], q);
        return c ? c : node_insert(n->childs[1], q);
    } else {
        /* Can this rectangle be packed into this node? */
        if (q->edge.x > n->rect.z || q->edge.y > n->rect.w) {
            return 0;
        }
        /* Does this rectangle have exactly the same size as this node? */
        if (q->edge.x == n->rect.z && q->edge.y == n->rect.w) {
            vec2 offset = vec2_new(n->rect.x, n->rect.y);
            for (unsigned int i = 0; i < 4; ++i)
                q->p[i] = vec2_add(q->p[i], offset);
            return n;
        }

        n->childs[0] = calloc(1, sizeof(struct node));
        n->childs[1] = calloc(1, sizeof(struct node));

        /* Decide which way to split */
        float dw = n->rect.z - q->edge.x;
        float dh = n->rect.w - q->edge.y;

        const vec2 border = vec2_new(0.0f, 0.0f);
        if (dw < dh) {
            n->childs[0]->rect = vec4_new(n->rect.x + q->edge.x, n->rect.y, n->rect.z - q->edge.x, q->edge.y);
            n->childs[1]->rect = vec4_new(n->rect.x, n->rect.y + q->edge.y, n->rect.z, n->rect.w - q->edge.y);
            n->childs[0]->rect = vec4_add(n->childs[0]->rect, vec4_new(border.x, 0.0f, -border.x, 0.0f));
            n->childs[1]->rect = vec4_add(n->childs[1]->rect, vec4_new(0.0f, border.y, 0.0f, -border.y));
        } else {
            n->childs[0]->rect = vec4_new(n->rect.x, n->rect.y + q->edge.y, q->edge.x, n->rect.w - q->edge.y);
            n->childs[1]->rect = vec4_new(n->rect.x + q->edge.x, n->rect.y, n->rect.z - q->edge.x, n->rect.w);
            n->childs[0]->rect = vec4_add(n->childs[0]->rect, vec4_new(0.0f, border.y, 0.0f, -border.y));
            n->childs[1]->rect = vec4_add(n->childs[1]->rect, vec4_new(border.x, 0.0f, -border.x, 0.0f));
        }

        /* Update the uv of the rectangle */
        vec2 offset = vec2_new(n->rect.x, n->rect.y);
        q->offset = offset;
        for (unsigned int i = 0; i < 4; ++i)
            q->p[i] = vec2_add(q->p[i], offset);
        return n;
    }
}

static void node_free(struct node* n)
{
    for (unsigned int i = 0; i < 2; ++i) {
        if (n->childs[i]) {
            node_free(n->childs[i]);
            n->childs[i] = 0;
        }
    }
    free(n);
}

static int quad_cmp(const void* a, const void* b)
{
    struct quadrilateral* q1 = (struct quadrilateral*)a;
    struct quadrilateral* q2 = (struct quadrilateral*)b;
    float a1 = q1->edge.x * q1->edge.y;
    float a2 = q2->edge.x * q2->edge.y;
    if (a1 < a2)
        return 1;
    if (a1 > a2)
        return -1;
    return 0;
}

void uvmap_planar_project(vec2* uv, vec3* vertices, vec3* normals, unsigned int* indices, size_t num_indices)
{
    struct vector quads;
    vector_init(&quads, sizeof(struct quadrilateral));

    for (size_t i = 0; i < num_indices; i += 3) {
        unsigned int ind = indices[i];
        vec3 nm = vec3_abs(*(normals + ind));
        if (nm.x > nm.y && nm.x > nm.z) {
            /* Project to plane X */
            for (unsigned int j = 0; j < 3; ++j) {
                uv[ind + j].x = vertices[ind + j].y;
                uv[ind + j].y = vertices[ind + j].z;
            }
        } else if (nm.y > nm.x && nm.y > nm.z) {
            /* Project to plane Y */
            for (unsigned int j = 0; j < 3; ++j) {
                uv[ind + j].x = vertices[ind + j].x;
                uv[ind + j].y = vertices[ind + j].z;
            }
        } else {
            /* Project to plane Z */
            for (unsigned int j = 0; j < 3; ++j) {
                uv[ind + j].x = vertices[ind + j].y;
                uv[ind + j].y = vertices[ind + j].x;
            }
        }

        /* Find bounding box */
        vec2 uvmin = uv[ind], uvmax = uv[ind];
        for (unsigned int i = 1; i < 3; ++i) {
            uvmin.x = min(uvmin.x, uv[ind + i].x);
            uvmin.y = min(uvmin.y, uv[ind + i].y);
            uvmax.x = max(uvmax.x, uv[ind + i].x);
            uvmax.y = max(uvmax.y, uv[ind + i].y);
        }

        /* Make uv values relative to the origin */
        vec2 uvdelta = vec2_sub(uvmax, uvmin);
        for (unsigned int j = 0; j < 3; ++j) {
            uv[ind + j].x -= uvdelta.x;
            uv[ind + j].y -= uvdelta.y;
        }

        struct quadrilateral q;
        q.edge = uvdelta;
        q.p[0] = vec2_new(0.0f, 0.0f);
        q.p[1] = vec2_new(q.edge.x, 0.0f);
        q.p[2] = q.edge;
        q.p[3] = vec2_new(0.0f, q.edge.y);
        vector_append(&quads, &q);
    }

    /* Compute area max */
    float scale = 0.0f;
    for (unsigned int i = 0; i < quads.size; ++i) {
        struct quadrilateral* q = ((struct quadrilateral*)vector_at(&quads, i));
        scale += q->edge.x * q->edge.y;
    }
    scale = sqrt(scale) * 1.35;

    /* Normalize to fit to texture */
    for (unsigned int i = 0; i < quads.size; ++i) {
        struct quadrilateral* q = ((struct quadrilateral*)vector_at(&quads, i));
        q->edge = vec2_div(q->edge, scale);
        for (unsigned int j = 0; j < 4; ++j)
            q->p[j] = vec2_div(q->p[j], scale);
    }
    for (size_t i = 0; i < num_indices; i += 3) {
        unsigned int ind = indices[i];
        for (unsigned int j = 0; j < 3; ++j) {
            uv[ind + j].x /= scale;
            uv[ind + j].y /= scale;
        }
    }

    /* Sort by area */
    qsort(quads.data, quads.size, quads.item_sz, quad_cmp);

    /* Recursive packing */
    struct node* root = calloc(1, sizeof(struct node));
    root->rect = vec4_new(0.f, 0.f, 1.f, 1.f);
    for (size_t i = 0; i < quads.size; ++i) {
        struct quadrilateral* q = ((struct quadrilateral*)vector_at(&quads, i));
        if (!node_insert(root, q)) {
            printf("Error! UV Map problem: [%lu](%f, %f)\n", i, q->edge.x, q->edge.y);
        }
    }
    node_free(root);
    vector_destroy(&quads);
}
