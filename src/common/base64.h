#ifndef BASE64_HEADER_GUARD_
#define BASE64_HEADER_GUARD_

#include "q_shared.h"

#define BASE64_ENCODED_LEN(len) (((len) + 2) / 3 * 4)
#define BASE64_DECODED_LEN(len) ((len) / 4 * 3)

// Caller must ensure the result can fit in `out`, can use the
// `BASE64_ENCODED_LEN` macro for this.
// `out` is not NUL-terminated.
void Base64_Encode(const void* in, unsigned int in_len, char* out);

// Caller must ensure the result can fit in `out`, can use the
// `BASE64_DECODED_LEN` macro for this.
// Returns false on failure.
qboolean Base64_Decode(const char* in, unsigned int in_len, void* out);

#endif  // BASE64_HEADER_GUARD_
