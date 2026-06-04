#include "base64.h"

#define PAD_CHAR '='
#define PADDING (-2)
#define INVALID (-1)

static const char* B64_ALPHABET =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static const char B64_REVERSE_ALPHABET[] = {
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, 62,      INVALID, INVALID,
    52,      53,      54,      55,      56,      57,      58,      59,
    60,      61,      INVALID, INVALID, INVALID, PADDING, INVALID, INVALID,
    INVALID, 0,       1,       2,       3,       4,       5,       6,
    7,       8,       9,       10,      11,      12,      13,      14,
    15,      16,      17,      18,      19,      20,      21,      22,
    23,      24,      25,      INVALID, INVALID, INVALID, INVALID, 63,
    INVALID, 26,      27,      28,      29,      30,      31,      32,
    33,      34,      35,      36,      37,      38,      39,      40,
    41,      42,      43,      44,      45,      46,      47,      48,
    49,      50,      51,      INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID};

void Base64_Encode(const void* in, unsigned int in_len, char* out)
{
    const byte* in_bytes = in;
    unsigned int bit = 0;

    for (bit = 0; bit < in_len * 8; bit += 6) {
        unsigned int byte_index = bit >> 3;
        byte hi = in_bytes[byte_index++];
        byte lo = byte_index < in_len ? in_bytes[byte_index] : 0;
        byte x = (((hi << 8) | lo) >> (10 - (bit & 7))) & 63;
        *out++ = B64_ALPHABET[x];
    }

    for (; bit & 7; bit -= 2) {
        *out++ = PAD_CHAR;
    }

    *out = '\0';
}

qboolean Base64_Decode(const char* in, unsigned int in_len, void* out)
{
    int i;
    const byte* in_bytes = (const byte*)in;
    byte* out_bytes = (byte*)out;

    if (in_len % 4 != 0) return qfalse;

    for (i = 0; i < in_len; i += 4) {
        char a = B64_REVERSE_ALPHABET[in_bytes[i]];
        char b = B64_REVERSE_ALPHABET[in_bytes[i + 1]];
        char c = B64_REVERSE_ALPHABET[in_bytes[i + 2]];
        char d = B64_REVERSE_ALPHABET[in_bytes[i + 3]];
        if (a < 0 || b < 0 || c == INVALID || d == INVALID) return qfalse;

        *out_bytes++ = ((byte)a << 2) | ((byte)b >> 4);

        if (c == PADDING) return d == PADDING && i + 4 == in_len;
        *out_bytes++ = ((byte)b << 4) | ((byte)c >> 2);

        if (d == PADDING) return i + 4 == in_len;
        *out_bytes++ = ((byte)c << 6) | (byte)d;
    }
    return qtrue;
}

#undef PAD_CHAR
#undef INVALID
#undef PADDING
