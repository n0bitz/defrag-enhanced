#ifndef HOOK_SUFFIX
#define HOOK_SUFFIX _H00K
#endif  // HOOK_SUFFIX

#ifndef ORIG_SUFFIX
#define ORIG_SUFFIX _OR161N4L
#endif  // ORIG_SUFFIX

#define HOOK_LOCAL_CONCAT(a, b) a##b
#define HOOK_LOCAL_EVAL_THEN_CONCAT(a, b) HOOK_LOCAL_CONCAT(a, b)

#define HOOKED(name) HOOK_LOCAL_EVAL_THEN_CONCAT(name, HOOK_SUFFIX)
#define ORIGINAL(name) HOOK_LOCAL_EVAL_THEN_CONCAT(name, ORIG_SUFFIX)

#define DECLARE_HOOK(ret_type_and_name, param_list) \
    ORIGINAL(ret_type_and_name) param_list;         \
    HOOKED(ret_type_and_name) param_list

#define DEFINE_HOOK(ret_type_and_name, param_list) \
    DECLARE_HOOK(ret_type_and_name, param_list)
