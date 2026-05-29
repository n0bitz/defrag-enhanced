#ifndef HOOK_HEADER_GUARD_
#define HOOK_HEADER_GUARD_

// The builder will override these suffixes, the ifndefs are just for editors
#ifndef HOOK_SUFFIX
#define HOOK_SUFFIX _H00K_
#endif  // HOOK_SUFFIX

#ifndef ORIG_SUFFIX
#define ORIG_SUFFIX _OR16_
#endif  // ORIG_SUFFIX

#define HOOK_LOCAL_CONCAT_(a, b) a##b
#define HOOK_LOCAL_EVAL_THEN_CONCAT_(a, b) HOOK_LOCAL_CONCAT_(a, b)

#define HOOKED(name) HOOK_LOCAL_EVAL_THEN_CONCAT_(name, HOOK_SUFFIX)
#define ORIGINAL(name) HOOK_LOCAL_EVAL_THEN_CONCAT_(name, ORIG_SUFFIX)

// NOLINTBEGIN(bugprone-macro-parentheses)

// Yeah, this declares both the hook and the original, since they are often
// tied together and it's simpler this way rather than introducing multiple
// macros -_o_-
// NOTE: The `extern` is to prevent anyone from mistakenly adding a static
// specifier, as they don't quite make sense for hooks. It would allow multiple
// compilation units to have conflicting hook definitions for the same symbol.
// Thus, it makes sense that hooks are extern linkage if only to prevent others
// from defining another hook for the same symbol. Moreover, the static
// specifier is a lie in q3asm and the symbols are globally visible anyway.
#define DECLARE_HOOK(ret_type, name, param_list) \
    extern ret_type ORIGINAL(name) param_list;   \
    extern ret_type HOOKED(name) param_list

// The original function being declared during hook definition is intended,
// as most hooks will probably eventually call the original function.
#define DEFINE_HOOK(ret_type, name, param_list) \
    DECLARE_HOOK(ret_type, name, param_list)    \
    {
#ifdef LINTER
#undef DEFINE_HOOK
// Shadow the original function with a function pointer inside the hook body to
// generate a diagnostic if the original isn't used, as most hooks should
// probably call the original at some point.
#define DEFINE_HOOK(ret_type, name, param_list) \
    DECLARE_HOOK(ret_type, name, param_list)    \
    {                                           \
        ret_type(*ORIGINAL(name)) param_list;
#endif  // LINTER

// NOLINTEND(bugprone-macro-parentheses)

#define END_HOOK }

#endif  // HOOK_HEADER_GUARD_
