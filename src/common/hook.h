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

// rather than introducing 3 different macros (hook, orig, both), it feels
// simpler to just do both here anyways...
#define DECLARE_HOOK(ret_type, name, param_list) \
    ret_type ORIGINAL(name) param_list;          \
    ret_type HOOKED(name) param_list

// The original function being declared during hook definition is intended,
// as most hooks will probably eventually call the original function.
#ifndef LINTER
#define DEFINE_HOOK(ret_type, name, param_list) \
    DECLARE_HOOK(ret_type, name, param_list)    \
    {
#else
// Shadow the original function with a function pointer inside the hook body to
// generate a diagnostic if the original isn't used, as most hooks should
// probably call the original at some point.
#define DEFINE_HOOK(ret_type, name, param_list) \
    DECLARE_HOOK(ret_type, name, param_list)    \
    {                                           \
        ret_type(*ORIGINAL(name)) param_list;

#endif

#define END_HOOK }

#endif  // HOOK_HEADER_GUARD_
