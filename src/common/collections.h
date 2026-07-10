#include "stb/stb_ds.h"

#define vec(T) T*
#define vec_free(v) arrfree(v)
#define vec_last(v) arrlast(v)
#define vec_len(v) arrlenu(v)
#define vec_push(v, value) arrpush(v, value)
#define vec_remove_swap(v, index) arrdelswap(v, index)
#define vec_reserve(v, n) arraddnptr(v, n)
#define vec_resize(v, n) arrsetlen(v, n)

#define hashmap(K, V) \
    struct {          \
        K key;        \
        V value;      \
    }*
#define hashmap_get(h, key) hmget(h, key)
#define hashmap_insert(h, key, value) hmput(h, key, value)
#define hashmap_len(h) hmlen(h)
