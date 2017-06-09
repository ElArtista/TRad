#include "uvmap.h"
#include <stdio.h>
#include <math.h>
#include <vector.h>

struct quadrilateral {
    vec2 size;
    vec2* tp[3];
};

struct node {
    struct node* childs[2];
    struct nrect { float x, y, width, height; } rect;
    int occupied;
};

static struct node* node_insert(struct node* n, struct quadrilateral* q)
{
    /* If the node has no children, try to recursively insert into them */
    if (n->childs[0] && n->childs[1]) {
        struct node* c = node_insert(n->childs[0], q);
        return c ? c : node_insert(n->childs[1], q);
    } else {
        /* Can this rectangle be packed into this node? */
        if (n->occupied || q->size.x > n->rect.width || q->size.y > n->rect.height) {
            return 0;
        }
        /* Does this rectangle have exactly the same size as this node? */
        if (q->size.x == n->rect.width && q->size.y == n->rect.height) {
            n->occupied = 1;
            vec2 offset = vec2_new(n->rect.x, n->rect.y);
            for (unsigned int i = 0; i < 3; ++i)
                *(q->tp[i]) = vec2_add(*(q->tp[i]), offset);
            return n;
        }

        n->childs[0] = calloc(1, sizeof(struct node));
        n->childs[1] = calloc(1, sizeof(struct node));

        /* Decide which way to split */
        float dw = n->rect.width - q->size.x;
        float dh = n->rect.height - q->size.y;

        if (dw > dh) {
            /* Vertical partition */
            n->childs[0]->rect = (struct nrect){n->rect.x, n->rect.y, q->size.x, n->rect.height};
            n->childs[1]->rect = (struct nrect){n->rect.x + q->size.x, n->rect.y, n->rect.width - q->size.x, n->rect.height};
        } else {
            /* Horizontal partition */
            n->childs[0]->rect = (struct nrect){n->rect.x, n->rect.y, n->rect.width, q->size.y};
            n->childs[1]->rect = (struct nrect){n->rect.x, n->rect.y + q->size.y, n->rect.width, n->rect.height - q->size.y};
        }

        return node_insert(n->childs[0], q);
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
    float a1 = q1->size.x * q1->size.y;
    float a2 = q2->size.x * q2->size.y;
    if (a1 < a2)
        return 1;
    if (a1 > a2)
        return -1;
    return 0;
}

void uvmap_planar_project(vec2* uv, vec3* vertices, size_t num_vertices, vec3* normals, unsigned int* indices, size_t num_indices)
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
        for (unsigned int j = 1; j < 3; ++j) {
            uvmin.x = min(uvmin.x, uv[ind + j].x);
            uvmin.y = min(uvmin.y, uv[ind + j].y);
            uvmax.x = max(uvmax.x, uv[ind + j].x);
            uvmax.y = max(uvmax.y, uv[ind + j].y);
        }

        /* Make uv values relative to the origin */
        for (unsigned int j = 0; j < 3; ++j) {
            uv[ind + j].x -= uvmin.x;
            uv[ind + j].y -= uvmin.y;
        }

        struct quadrilateral q;
        q.size = vec2_sub(uvmax, uvmin);
        for (unsigned int j = 0; j < 3; ++j)
            q.tp[j] = uv + (ind + j);
        vector_append(&quads, &q);
    }

    /* Compute area max */
    float scale = 0.0f;
    for (unsigned int i = 0; i < quads.size; ++i) {
        struct quadrilateral* q = ((struct quadrilateral*)vector_at(&quads, i));
        scale += q->size.x * q->size.y;
    }
    scale = sqrt(scale) * 1.35;

    /* Normalize to fit to texture */
    for (unsigned int i = 0; i < quads.size; ++i) {
        struct quadrilateral* q = ((struct quadrilateral*)vector_at(&quads, i));
        q->size = vec2_div(q->size, scale);
    }
    for (unsigned int i = 0; i < num_vertices; ++i)
        uv[i] = vec2_div(uv[i], scale);

    /* Sort by area */
    qsort(quads.data, quads.size, quads.item_sz, quad_cmp);

    /* Recursive packing */
    struct node* root = calloc(1, sizeof(struct node));
    root->rect = (struct nrect){ 0.f, 0.f, 1.f, 1.f };
    for (size_t i = 0; i < quads.size; ++i) {
        struct quadrilateral* q = ((struct quadrilateral*)vector_at(&quads, i));
        if (!node_insert(root, q)) {
            printf("Error! UV Map problem: [%lu](%f, %f)\n", i, q->size.x, q->size.y);
        }
    }

    node_free(root);
    vector_destroy(&quads);
}
