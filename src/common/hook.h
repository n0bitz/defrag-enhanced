#ifndef __HOOK_HEADER_GUARD
#define __HOOK_HEADER_GUARD

#ifndef HOOK_SUFFIX
#define HOOK_SUFFIX _H00K
#endif  // HOOK_SUFFIX

#ifndef ORIG_SUFFIX
#define ORIG_SUFFIX _OR161N4L
#endif  // ORIG_SUFFIX

#define _HOOK_LOCAL_CONCAT(a, b) a##b
#define _HOOK_LOCAL_EVAL_THEN_CONCAT(a, b) _HOOK_LOCAL_CONCAT(a, b)

#define HOOKED(name) _HOOK_LOCAL_EVAL_THEN_CONCAT(name, HOOK_SUFFIX)
#define ORIGINAL(name) _HOOK_LOCAL_EVAL_THEN_CONCAT(name, ORIG_SUFFIX)

// rather than introducing 3 different macros (hook, orig, both), it feels
// simpler to just do both here anyways...
#define DECLARE_HOOK(ret_type_and_name, param_list) \
    ORIGINAL(ret_type_and_name) param_list;         \
    HOOKED(ret_type_and_name) param_list

// the definition deliberately declares the original function, as most
// hooks will probably eventually call the original function...
#define DEFINE_HOOK(ret_type_and_name, param_list) \
    DECLARE_HOOK(ret_type_and_name, param_list)

#endif  // __HOOK_HEADER_GUARD