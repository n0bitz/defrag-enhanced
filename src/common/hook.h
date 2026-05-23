#ifndef HOOK_HEADER_GUARD_
#define HOOK_HEADER_GUARD_

#ifndef HOOK_SUFFIX
#define HOOK_SUFFIX _H00K
#endif  // HOOK_SUFFIX

#ifndef ORIG_SUFFIX
#define ORIG_SUFFIX _OR161N4L
#endif  // ORIG_SUFFIX

#define HOOK_LOCAL_CONCAT_(a, b) a##b
#define HOOK_LOCAL_EVAL_THEN_CONCAT_(a, b) HOOK_LOCAL_CONCAT_(a, b)

#define HOOKED(name) HOOK_LOCAL_EVAL_THEN_CONCAT_(name, HOOK_SUFFIX)
#define ORIGINAL(name) HOOK_LOCAL_EVAL_THEN_CONCAT_(name, ORIG_SUFFIX)

// rather than introducing 3 different macros (hook, orig, both), it feels
// simpler to just do both here anyways...
#define DECLARE_HOOK(ret_type_and_name, param_list) \
    ORIGINAL(ret_type_and_name) param_list;         \
    HOOKED(ret_type_and_name) param_list

// the definition deliberately declares the original function, as most
// hooks will probably eventually call the original function...
#define DEFINE_HOOK(ret_type_and_name, param_list) \
    DECLARE_HOOK(ret_type_and_name, param_list)

#endif  // HOOK_HEADER_GUARD_
