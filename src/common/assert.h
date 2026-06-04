#ifndef ASSERT_HEADER_GUARD_
#define ASSERT_HEADER_GUARD_

// Unfortunately, ANSI C doesn't have a static_assert, nor will it let us mix
// statements and declarations, so we need two versions of static_assert...
#define static_assert(expr, msg)                 \
    int ASSERT_INTERNAL_PREFIX_##static_assert_( \
       int static_assert[(expr) ? 1 : -1])

#define static_assert_stmt(expr, msg) \
    do {                              \
        static_assert(expr, msg);     \
    } while (0)

#ifdef LINTER
#undef static_assert
// Turning format off cause it keeps trying to break the pragma literals up
// and pragmas don't like that.
// clang-format off
#define static_assert(expr, msg)                                \
    _Pragma("clang diagnostic push")                            \
    _Pragma("clang diagnostic ignored \"-Wc11-extensions\"")    \
    _Static_assert _Pragma("clang diagnostic pop")              \
    (expr, msg)
// clang-format on
#endif  // LINTER

#endif  // ASSERT_HEADER_GUARD_
