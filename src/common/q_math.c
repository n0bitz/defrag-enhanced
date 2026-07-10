#include "q_math.h"

void Vector4Lerp(const vec4_t a, const vec4_t b, float t, vec4_t c)
{
    int i;
    for (i = 0; i < 4; i++) {
        c[i] = ((1 - t) * a[i]) + (t * b[i]);
    }
}

float MoveToward(float from, float to, float maxDelta)
{
    return (from <= to) ? fminf(from + maxDelta, to)
                        : fmaxf(from - maxDelta, to);
}

vec(vec3_t) ClipPoly(vec(vec3_t) poly, dplane_t* plane)
{
    vec(vec3_t) out = NULL;
    vec3_t a, b;
    float a_dist, b_dist;
    int i;

    if (vec_len(poly) < 3) return NULL;

    VectorCopy(vec_last(poly), a);
    for (i = 0; i < vec_len(poly); i++) {
        VectorCopy(poly[i], b);

        a_dist = DotProduct(a, plane->normal) - plane->dist;
        b_dist = DotProduct(b, plane->normal) - plane->dist;

        if (a_dist <= 0.0) {
            vec3_t* o = vec_reserve(out, 1);
            VectorCopy(a, *o);
        }

        if (a_dist * b_dist < 0.0) {
            vec3_t* o = vec_reserve(out, 1);
            VectorSubtract(b, a, *o);
            VectorMA(a, a_dist / (a_dist - b_dist), *o, *o);
        }

        VectorCopy(b, a);
    }

    return out;
}
