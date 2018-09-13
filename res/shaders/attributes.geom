#version 150
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 pos;
    vec3 nrm;
    vec3 col;
    vec2 luv;
} gs_in[];

out GS_OUT {
    // Vertex shader passthrough
    vec3 pos;
    vec3 nrm;
    vec3 col;
    vec2 luv;
    // Geometry shader outputs
    vec2 ndc_pos;
    vec4 ndc_aabb;
} gs_out;

//uniform vec2 half_pixel_size;
const vec2 half_pixel_size = vec2(1.0/128.0);

// See Gpu Gems 2, Chapter 42: Conservative Rasterization.
// (http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter42.html)
// See "Conservative and Tiled Rasterization Using a Modified Triangle Setup." Journal of Graphics Tools.
// See Plane-Plane Intersection (http://mathworld.wolfram.com/Plane-PlaneIntersection.html)
void main(void)
{
    vec4 vertex[3];
    for (int i = 0; i < gl_in.length(); i++) {
        vertex[i] = /* proj * view */ gl_in[i].gl_Position;
        vertex[i] /= vertex[i].w;
    }

    // Find largest face area
    vec3 triangle_normal = abs(normalize(cross(vertex[1].xyz - vertex[0].xyz, vertex[2].xyz - vertex[0].xyz)));

    // Triangle plane to later calculate the new z coordinate.
    vec4 triangle_plane;
    triangle_plane.xyz = normalize(cross(vertex[1].xyz - vertex[0].xyz, vertex[2].xyz - vertex[0].xyz));
    triangle_plane.w = -dot(vertex[0].xyz, triangle_plane.xyz);

    //
    // Axis aligned bounding box (AABB) initialized with maximum/minimum NDC values.
    //
    vec4 aabb = vec4(1.0, 1.0, -1.0, -1.0);
    for (int i = 0; i < gl_in.length(); i++) {
        // Get AABB of the triangle.
        aabb.xy = min(aabb.xy, vertex[i].xy);
        aabb.zw = max(aabb.zw, vertex[i].xy);
    }
    // Add offset of half pixel size to AABB.
    vec4 aabb_conservative = aabb + vec4(-half_pixel_size, half_pixel_size);

    //
    // Calculate triangle for conservative rasterization.
    //
    vec3 plane[3];
    for (int i = 0; i < gl_in.length(); i++) {
        // Calculate the plane through each edge of the triangle.
        // The plane equation is A*x + B*y + C*w = 0.
        // Note, that D component from the plane is zero, as it goes throught the origin.
        // Also, w is used as the third dimension.
        //
        // Notes:
        //  - The plane goes through the origin, as (vertex[i].x, vertex[i].y, vertex[i].w) are vectors from the origin.
        //  - A*x + B*y + C*w = 0 <=> A*x/w + B*y/w + C = 0 [w != 0] <=> A*xClip + B*yClip + C
        //  - A*xClip + B*yClip + C is a vector in 2D space. In this case, it is the normal of the edge in 2D space.
        //
        // By calculating the cross product of the two vectors (the end points of the edge),
        // we gather the normal of the plane.
        plane[i] = cross(vertex[i].xyw, vertex[(i + 2) % 3].xyw);

        // Move plane, by adjusting C.
        //
        // Note: A*(x+dx) + B*(y+dy) + C*w = 0 [Another plane with a specific distance d given by dx and dy] <=>
        //       A*x + A*dx + B*y + B*dy + C*w = 0 <=>
        //       A*xClip + B*yClip + C + A*dx + B*dy = 0
        //
        // Half pixel size is d. Absolute of plane's xy, as the sign is already in the normal of the plane.
        //plane[i].z -= dot(half_pixel_size, abs(plane[i].xy));
        plane[i].z += dot(half_pixel_size, abs(plane[i].xy));
    }

    // Create conservative rasterized triangle
    vec3 intersect[3];
    for (int i = 0; i < gl_in.length(); i++) {
        // As both planes go through the origin, the intersecting line goes also through the origin.
        // This simplifies the intersection calculation.
        // The intersecting line is perpendicular to both planes,
        // so the intersection line is just the cross product of both normals.
        intersect[i] = cross(plane[i], plane[(i+1) % 3]);
        // The line is a direction (x, y, w) but projects to the same point in window space.
        // Compare: (x, y, w) <=> (x/w, y/w, 1) => (xClip, yClip)
        intersect[i] /= intersect[i].z;
    }

#define NO_CONSERVATIVE
    for (int i = 0; i < gl_in.length(); i++) {
#ifndef NO_CONSERVATIVE
        gl_Position.xyw = intersect[i];
        // Calculate the new z-Coordinate derived from a point on a plane.
        gl_Position.z = -(triangle_plane.x * intersect[i].x + triangle_plane.y * intersect[i].y + triangle_plane.w) / triangle_plane.z;

        gs_out.ndc_pos = intersect[i].xy / intersect[i].z;
        gs_out.ndc_aabb = aabb_conservative;
#else
        // Create "normal" rasterized triangle
        gl_Position = vertex[i];
        gs_out.ndc_pos = vertex[i].xy;
        gs_out.ndc_aabb = aabb;
#endif
        // Passthrough values
        gs_out.pos = gs_in[i].pos;
        gs_out.nrm = gs_in[i].nrm;
        gs_out.col = gs_in[i].col;
        gs_out.luv = gs_in[i].luv;
        EmitVertex();
    }
    EndPrimitive();
}
