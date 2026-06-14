#ifndef Q_MATH_HEADER_GUARD__
#define Q_MATH_HEADER_GUARD__

#include "collections.h"
#include "qfiles.h"
#include "q_shared.h"

#define DotProduct4(x, y) \
    ((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2] + (x)[3] * (y)[3])
#define Vector4Subtract(a, b, c)                         \
    ((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1], \
     (c)[2] = (a)[2] - (b)[2], (c)[3] = (a)[3] - (b)[3])
#define Vector4Add(a, b, c)                              \
    ((c)[0] = (a)[0] + (b)[0], (c)[1] = (a)[1] + (b)[1], \
     (c)[2] = (a)[2] + (b)[2], (c)[3] = (a)[3] + (b)[3])
#define Vector4MA(v, s, b, o)                                            \
    ((o)[0] = (v)[0] + ((b)[0] * (s)), (o)[1] = (v)[1] + ((b)[1] * (s)), \
     (o)[2] = (v)[2] + ((b)[2] * (s)), (o)[3] = (v)[3] + ((b)[3] * (s)))

#define Vector4Clear(a) ((a)[0] = (a)[1] = (a)[2] = (a)[3] = 0)
#define Vector4Negate(a, b) \
    ((b)[0] = -(a)[0], (b)[1] = -(a)[1], (b)[2] = -(a)[2], (b)[3] = -(a)[3])
#define Vector4Set(v, x, y, z, w) \
    ((v)[0] = (x), (v)[1] = (y), (v)[2] = (z), (v)[3] = (w))

void Vector4Lerp(const vec4_t a, const vec4_t b, float t, vec4_t c);
float MoveToward(float from, float to, float maxDelta);
vec(vec3_t) ClipPoly(vec(vec3_t) poly, dplane_t* plane);

#endif  // Q_MATH_HEADER_GUARD__
