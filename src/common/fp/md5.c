/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1996, 1997, 1999, 2006, 2007 Peter Miller;
 *      All rights reserved.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 *
 * Derived from code marked
 *      derived from RSADSI MD5 Message-Digest Algorithm
 *      Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991.
 *      All rights reserved.
 *
 *      License to copy and use this software is granted provided that it
 *      is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 *      Algorithm" in all material mentioning or referencing this software
 *      or this function.
 *
 *      License is also granted to make and use derivative works provided
 *      that such works are identified as "derived from the RSA Data
 *      Security, Inc. MD5 Message-Digest Algorithm" in all material
 *      mentioning or referencing the derived work.
 *
 *      RSA Data Security, Inc. makes no representations concerning either
 *      the merchantability of this software or the suitability of this
 *      software for any particular purpose. It is provided "as is"
 *      without express or implied warranty of any kind.
 *
 *      These notices must be retained in any copies of any part of this
 *      documentation and/or software.
 */

#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/limits.h>

#include <common/trace.h> /* for assert */
#include <common/fp/md5.h>


/*
 * work out a 32-bit type
 */
#if (SIZEOF_SHORT * CHAR_BIT == 32)
typedef unsigned short uint32;
#else
#if (SIZEOF_INT * CHAR_BIT == 32)
typedef unsigned int uint32;
#else
#if (SIZEOF_LONG * CHAR_BIT == 32)
typedef unsigned long uint32;
#else
#error "no type is exactly 32 bits"
#endif
#endif
#endif

#define MD5_HASH_LEN 16

typedef struct md5_ty md5_ty;
struct md5_ty
{
    FINGERPRINT_BASE_CLASS
    unsigned long   state[4];
    unsigned long   count[2];
    unsigned char   buffer[64];
};


/*
 * Constants for MD5Transform routine.
 */
#define S11             7
#define S12             12
#define S13             17
#define S14             22
#define S21             5
#define S22             9
#define S23             14
#define S24             20
#define S31             4
#define S32             11
#define S33             16
#define S34             23
#define S41             6
#define S42             10
#define S43             15
#define S44             21

static unsigned char PADDING[64] =
{
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z)      (((x)&(y)) | ((~x)&(z)))
#define G(x, y, z)      (((x)&(z)) | ((y)&(~z)))
#define H(x, y, z)      ((x) ^ (y) ^ (z))
#define I(x, y, z)      ((y) ^ ((x) | (~z)))

/*
 * ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) ((uint32)((x) << (n)) | ((uint32)(x) >> (32 - (n))))

/*
 * FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
 * Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac)                                        \
    do {                                                                \
        (a) += F((b), (c), (d)) + (x) + (unsigned long)(ac);    \
        (a) = ROTATE_LEFT((a), (s));                            \
        (a) += (b);                                             \
    } while(0)
#define GG(a, b, c, d, x, s, ac)                                        \
    do {                                                                \
        (a) += G((b), (c), (d)) + (x) + (unsigned long)(ac);    \
        (a) = ROTATE_LEFT((a), (s));                            \
        (a) += (b);                                             \
    } while(0)
#define HH(a, b, c, d, x, s, ac)                                        \
    do {                                                                \
        (a) += H((b), (c), (d)) + (x) + (unsigned long)(ac);    \
        (a) = ROTATE_LEFT((a), (s));                            \
        (a) += (b);                                             \
    } while(0)
#define II(a, b, c, d, x, s, ac)                                        \
    do {                                                                \
        (a) += I((b), (c), (d)) + (x) + (unsigned long)(ac);    \
        (a) = ROTATE_LEFT((a), (s));                            \
        (a) += (b);                                             \
    } while(0)


/*
 * MD5 initialization. Begins an MD5 operation, writing a new context.
 */

static void
reset(md5_ty *context)
{
    context->count[0] = 0;
    context->count[1] = 0;

    /*
     * Load magic initialization constants.
     */
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}


static void
md5_constructor(fingerprint_ty *p)
{
    md5_ty          *context;

    context = (md5_ty *)p;
    reset(context);
}


static void
md5_destructor(fingerprint_ty *p)
{
    (void)p;
}


/*
 * Encodes input (unsigned long) into output (unsigned char).
 * Assumes len is a multiple of 4.
 */

static void
Encode(unsigned char *output, unsigned long *input, size_t len)
{
    unsigned int    i;
    unsigned int    j;

    for (i = 0, j = 0; j < len; i++, j += 4)
    {
        output[j] = (unsigned char)input[i];
        output[j + 1] = (unsigned char)(input[i] >> 8);
        output[j + 2] = (unsigned char)(input[i] >> 16);
        output[j + 3] = (unsigned char)(input[i] >> 24);
    }
}


/*
 * Decodes input (unsigned char) into output (unsigned long).
 * Assumes len is a multiple of 4.
 */

static void
Decode(unsigned long *output, const unsigned char *input, size_t len)
{
    unsigned int    i;
    unsigned int    j;

    for (i = 0, j = 0; j < len; i++, j += 4)
    {
        output[i] =
            (
                ((unsigned long)input[j])
            |
                (((unsigned long)input[j + 1]) << 8)
            |
                (((unsigned long)input[j + 2]) << 16)
            |
                (((unsigned long)input[j + 3]) << 24)
            );
    }
}


/*
 * MD5 basic transformation. Transforms state based on block.
 * (each block is 64 bytes.)
 */

static void
MD5Transform(unsigned long state[4], const unsigned char *block)
{
    unsigned long   a = state[0];
    unsigned long   b = state[1];
    unsigned long   c = state[2];
    unsigned long   d = state[3];
    unsigned long   x[16];

    Decode(x, block, 64);
    /* Round 1 */
    FF(a, b, c, d, x[ 0], S11, 0xd76aa478);     /* 1 */
    FF(d, a, b, c, x[ 1], S12, 0xe8c7b756);     /* 2 */
    FF(c, d, a, b, x[ 2], S13, 0x242070db);     /* 3 */
    FF(b, c, d, a, x[ 3], S14, 0xc1bdceee);     /* 4 */
    FF(a, b, c, d, x[ 4], S11, 0xf57c0faf);     /* 5 */
    FF(d, a, b, c, x[ 5], S12, 0x4787c62a);     /* 6 */
    FF(c, d, a, b, x[ 6], S13, 0xa8304613);     /* 7 */
    FF(b, c, d, a, x[ 7], S14, 0xfd469501);     /* 8 */
    FF(a, b, c, d, x[ 8], S11, 0x698098d8);     /* 9 */
    FF(d, a, b, c, x[ 9], S12, 0x8b44f7af);     /* 10 */
    FF(c, d, a, b, x[10], S13, 0xffff5bb1);     /* 11 */
    FF(b, c, d, a, x[11], S14, 0x895cd7be);     /* 12 */
    FF(a, b, c, d, x[12], S11, 0x6b901122);     /* 13 */
    FF(d, a, b, c, x[13], S12, 0xfd987193);     /* 14 */
    FF(c, d, a, b, x[14], S13, 0xa679438e);     /* 15 */
    FF(b, c, d, a, x[15], S14, 0x49b40821);     /* 16 */
    /* Round 2 */
    GG(a, b, c, d, x[ 1], S21, 0xf61e2562);     /* 17 */
    GG(d, a, b, c, x[ 6], S22, 0xc040b340);     /* 18 */
    GG(c, d, a, b, x[11], S23, 0x265e5a51);     /* 19 */
    GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa);     /* 20 */
    GG(a, b, c, d, x[ 5], S21, 0xd62f105d);     /* 21 */
    GG(d, a, b, c, x[10], S22, 0x2441453);      /* 22 */
    GG(c, d, a, b, x[15], S23, 0xd8a1e681);     /* 23 */
    GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8);     /* 24 */
    GG(a, b, c, d, x[ 9], S21, 0x21e1cde6);     /* 25 */
    GG(d, a, b, c, x[14], S22, 0xc33707d6);     /* 26 */
    GG(c, d, a, b, x[ 3], S23, 0xf4d50d87);     /* 27 */
    GG(b, c, d, a, x[ 8], S24, 0x455a14ed);     /* 28 */
    GG(a, b, c, d, x[13], S21, 0xa9e3e905);     /* 29 */
    GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8);     /* 30 */
    GG(c, d, a, b, x[ 7], S23, 0x676f02d9);     /* 31 */
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);     /* 32 */
    /* Round 3 */
    HH(a, b, c, d, x[ 5], S31, 0xfffa3942);     /* 33 */
    HH(d, a, b, c, x[ 8], S32, 0x8771f681);     /* 34 */
    HH(c, d, a, b, x[11], S33, 0x6d9d6122);     /* 35 */
    HH(b, c, d, a, x[14], S34, 0xfde5380c);     /* 36 */
    HH(a, b, c, d, x[ 1], S31, 0xa4beea44);     /* 37 */
    HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9);     /* 38 */
    HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60);     /* 39 */
    HH(b, c, d, a, x[10], S34, 0xbebfbc70);     /* 40 */
    HH(a, b, c, d, x[13], S31, 0x289b7ec6);     /* 41 */
    HH(d, a, b, c, x[ 0], S32, 0xeaa127fa);     /* 42 */
    HH(c, d, a, b, x[ 3], S33, 0xd4ef3085);     /* 43 */
    HH(b, c, d, a, x[ 6], S34, 0x4881d05);      /* 44 */
    HH(a, b, c, d, x[ 9], S31, 0xd9d4d039);     /* 45 */
    HH(d, a, b, c, x[12], S32, 0xe6db99e5);     /* 46 */
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8);     /* 47 */
    HH(b, c, d, a, x[ 2], S34, 0xc4ac5665);     /* 48 */
    /* Round 4 */
    II(a, b, c, d, x[ 0], S41, 0xf4292244);     /* 49 */
    II(d, a, b, c, x[ 7], S42, 0x432aff97);     /* 50 */
    II(c, d, a, b, x[14], S43, 0xab9423a7);     /* 51 */
    II(b, c, d, a, x[ 5], S44, 0xfc93a039);     /* 52 */
    II(a, b, c, d, x[12], S41, 0x655b59c3);     /* 53 */
    II(d, a, b, c, x[ 3], S42, 0x8f0ccc92);     /* 54 */
    II(c, d, a, b, x[10], S43, 0xffeff47d);     /* 55 */
    II(b, c, d, a, x[ 1], S44, 0x85845dd1);     /* 56 */
    II(a, b, c, d, x[ 8], S41, 0x6fa87e4f);     /* 57 */
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0);     /* 58 */
    II(c, d, a, b, x[ 6], S43, 0xa3014314);     /* 59 */
    II(b, c, d, a, x[13], S44, 0x4e0811a1);     /* 60 */
    II(a, b, c, d, x[ 4], S41, 0xf7537e82);     /* 61 */
    II(d, a, b, c, x[11], S42, 0xbd3af235);     /* 62 */
    II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb);     /* 63 */
    II(b, c, d, a, x[ 9], S44, 0xeb86d391);     /* 64 */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /* Reset sensitive information. */
    memset(x, 0, sizeof(x));
}


/*
 * MD5 block update operation. Continues an MD5 message-digest
 * operation, processing another message block, and updating the
 * context.
 */

static void
md5_addn(fingerprint_ty *p, const void *vip, size_t inputLen)
{
    md5_ty         *context;
    unsigned int    i;
    unsigned int    idx;
    unsigned int    partLen;
    const unsigned char *input;

    input = vip;
    context = (md5_ty *)p;
    /* Compute number of bytes mod 64 */
    idx = (unsigned int)((context->count[0] >> 3) & 0x3F);
    /* Update number of bits */
    if
    (
        (context->count[0] += ((unsigned long)inputLen << 3))
    <
        ((unsigned long)inputLen << 3)
    )
        context->count[1]++;
    context->count[1] += ((unsigned long)inputLen >> 29);
    partLen = 64 - idx;

    /*
     * Transform as many times as possible.
     */
    if (inputLen >= partLen)
    {
        memcpy(&context->buffer[idx], input, partLen);
        MD5Transform(context->state, context->buffer);
        for (i = partLen; i + 63 < inputLen; i += 64)
            MD5Transform(context->state, &input[i]);
        idx = 0;
    }
    else
        i = 0;
    /* Buffer remaining input */
    memcpy(&context->buffer[idx], &input[i], inputLen - i);
}



/*
 * MD5 finalization.
 * Ends an MD5 message-digest operation, writing the message digest
 * and resetting the context.
 */

static int
md5_hash(fingerprint_ty *p, unsigned char *digest, size_t digest_len)
{
    md5_ty          *context;
    unsigned char   bits[8];
    unsigned int    idx;
    unsigned int    padLen;

    (void)digest_len;
    assert(digest_len >= MD5_HASH_LEN);

    /*
     * Save number of bits
     */
    context = (md5_ty *)p;
    Encode(bits, context->count, 8);

    /*
     * Pad out to 56 mod 64.
     */
    idx = (unsigned int)((context->count[0] >> 3) & 0x3f);
    padLen = (idx < 56) ? (56 - idx) : (120 - idx);
    md5_addn(p, PADDING, padLen);

    /*
     * Append length (before padding)
     */
    md5_addn(p, bits, 8);

    /*
     * Store state in digest
     */
    Encode(digest, context->state, MD5_HASH_LEN);

    /*
     * reset sensitive information.
     */
    reset(context);

    /*
     * return length of hash
     */
    return MD5_HASH_LEN;
}


static void
md5_sum(fingerprint_ty *p, char *obuf, size_t obuf_len)
{
    md5_ty          *context;
    unsigned char   h[MD5_HASH_LEN];
    char            *cp;
    int             i;
    char            *ep;

    context = (md5_ty *)p;
    md5_hash(p, h, sizeof(h));
    cp = obuf;
    ep = obuf + obuf_len;
    for (i = 0; i < MD5_HASH_LEN; ++i)
    {
        snprintf(cp, ep - cp, "%2.2X", h[i]);
        cp += strlen(cp);
    }
    *cp = 0;
}


fingerprint_methods_ty fp_md5 =
{
    sizeof(md5_ty),
    "md5",
    md5_constructor,
    md5_destructor,
    md5_addn,
    md5_hash,
    md5_sum
};
