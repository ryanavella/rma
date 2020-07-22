/* rma_str.h - 0.2.1
**   Safer null-terminated string type for C.
**
**   Dual-licensed under MIT or the UNLICENSE.
**
** Simple example:
** 
**     #include <stdio.h>
**     #include <stdlib.h>
**     #define RMA_STR__IMPLEMENTATION
**     #include "rma_str.h"
**     
**     int main(void) {
**         rma_Str v;
**         char *s1 = "Hello, ";
**         char *s2 = "World!";
**         
**         rma_Str__from_cstr(&v, s1);
**         rma_Str__push_cstr(&v, s2);
**         printf("%s\n", rma_Str__as_ptr(&v));
**         return 0;
**     }
*/

#include <errno.h>  /* ENOMEM */
#include <stddef.h> /* size_t */
#include <string.h> /* memcpy, memmove, strcpy, strlen */
#include <stdint.h> /* SIZE_MAX */

#ifdef _MSC_VER
/* ignore MSVC deprecation warnings */
#  pragma warning(disable: 4996)
#endif

#ifndef RMA_STR__STORAGE
#  define RMA_STR__STORAGE
#endif

#ifndef RMA__JOIN
#  define RMA__JOIN(x, y) RMA__JOIN_EXPAND_ARGS(x, y)
#  define RMA__JOIN_EXPAND_ARGS(x, y) x ## y
#endif

#ifndef RMA_STR__NAME
#define RMA_STR__NAME rma_Str
#endif

#ifndef RMA_STR__FREE
#  include <stdlib.h> /* free, malloc, realloc */
#  define RMA_STR__FREE(x)          free(x)
#  define RMA_STR__MALLOC(x)        malloc(x)
#  define RMA_STR__REALLOC(ptr, sz) realloc((ptr), (sz))
#endif

#ifndef RMA__MIN
#  define RMA__MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef RMA__ALIGNOF
#  ifdef _MSC_VER
#    define RMA__ALIGNOF(t) __alignof(t)
#  elif defined(__GNUC__)
#    define RMA__ALIGNOF(t) __alignof__(t)
#  else
#    define RMA__ALIGNOF(t) \
        RMA__MIN( \
            sizeof(t), \
            offsetof(struct {char c; t x;}, x) \
        )
#  endif
#endif

#ifndef RMA__UNUSED
#  ifdef __GNUC__
#    define RMA__UNUSED __attribute__((unused))
#  else
#    define RMA__UNUSED
#  endif
#endif

#ifndef RMA_STR__ENOMEM
#  ifdef ENOMEM
#    define RMA_STR__ENOMEM ENOMEM
#  else
#    define RMA_STR__ENOMEM 12
#  endif
#endif

#undef RMA__OOM
#undef RMA__OOB

#define RMA__OOM RMA_STR__ENOMEM /* out of memory */
#define RMA__OOB -1               /* out of bounds */

typedef struct RMA_STR__NAME {
    char  *ptr;
    size_t cap;
    size_t len;
} RMA_STR__NAME;

RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __clone)(RMA_STR__NAME *dst, RMA_STR__NAME *src);
RMA_STR__STORAGE void RMA__JOIN(RMA_STR__NAME, __free )(RMA_STR__NAME *v);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __from_cstr)(RMA_STR__NAME *v, const char *ptr);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __insert)(RMA_STR__NAME *v, size_t pos, char elem);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __pop )(RMA_STR__NAME *v, char *out);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __push)(RMA_STR__NAME *v, char elem);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __push_cstr)(RMA_STR__NAME *v, const char *ptr);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __push_raw_parts)(RMA_STR__NAME *v, size_t len, const char *ptr);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __push_str)(RMA_STR__NAME *v, RMA_STR__NAME *other);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __reserve )(RMA_STR__NAME *v, size_t cap);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __with_capacity )(RMA_STR__NAME *out, size_t cap);

static char * RMA__UNUSED
RMA__JOIN(RMA_STR__NAME, __as_ptr)(RMA_STR__NAME *v) {
    return v->ptr;
}

static size_t RMA__UNUSED
RMA__JOIN(RMA_STR__NAME, __cap)(RMA_STR__NAME *v) {
    return v->cap;
}

static size_t RMA__UNUSED
RMA__JOIN(RMA_STR__NAME, __len)(RMA_STR__NAME *v) {
    return v->len;
}

static RMA_STR__NAME RMA__UNUSED
RMA__JOIN(RMA_STR__NAME, __new)(void) {
    RMA_STR__NAME v;
    v.ptr = (char *)RMA__ALIGNOF(char);
    v.cap = 0;
    v.len = 0;
    return v;
}

#ifdef RMA_STR__IMPLEMENTATION

RMA_STR__STORAGE int RMA__JOIN(RMA_STR__NAME, __clone)(RMA_STR__NAME *dst, RMA_STR__NAME *src) {
    dst->ptr = RMA_STR__MALLOC(src->cap);
    if (!dst->ptr) {
        return RMA__OOM;
    }
    memcpy(dst->ptr, src->ptr, src->len);
    dst->cap = src->cap;
    dst->len = src->len;
    return 0;
}

RMA_STR__STORAGE void RMA__JOIN(RMA_STR__NAME, __free)(RMA_STR__NAME *v) {
    if (v) {
        RMA_STR__FREE(v->ptr);
    }
}

RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __from_cstr)(RMA_STR__NAME *v, const char *ptr) {
    v->len = strlen(ptr);
    v->ptr = RMA_STR__MALLOC(v->len + 1);
    if (!v->ptr) {
        return RMA__OOM;
    }
    strcpy(v->ptr, ptr);
    v->cap = v->len + 1;
    return 0;
}

RMA_STR__STORAGE int RMA__JOIN(RMA_STR__NAME, __insert)(RMA_STR__NAME *v, size_t pos, char elem) {
    int err;

    if (pos > v->len) {
        return RMA__OOB;
    }
    if (v->len >= SIZE_MAX - 1) {
        return RMA__OOM;
    }
    err = RMA__JOIN(RMA_STR__NAME, __reserve)(v, v->len + 2);
    if (err) {
        return err;
    }
    memmove(v->ptr + pos + 1, v->ptr + pos, v->len - pos);
    v->ptr[pos] = elem;
    v->len++;
    v->ptr[v->len] = '\0';
    return 0;
}

RMA_STR__STORAGE int RMA__JOIN(RMA_STR__NAME, __pop)(RMA_STR__NAME *v, char *out) { 
    if (!v->len) {
        return -1;
    }
    v->len--;
    *out = v->ptr[v->len];
    return 0;
}

RMA_STR__STORAGE int RMA__JOIN(RMA_STR__NAME, __push)(RMA_STR__NAME *v, char elem) {
    return RMA__JOIN(RMA_STR__NAME, __insert)(v, v->len, elem);
}

RMA_STR__STORAGE int RMA__JOIN(RMA_STR__NAME, __push_cstr)(RMA_STR__NAME *v, const char *ptr) {
    return RMA__JOIN(RMA_STR__NAME, __push_raw_parts)(v, strlen(ptr), ptr);
}

RMA_STR__STORAGE int RMA__JOIN(RMA_STR__NAME, __push_raw_parts)(RMA_STR__NAME *v, size_t len, const char *ptr) {
    size_t new_len;

    if (v->len > SIZE_MAX - len - 1) {
        return RMA__OOM;
    }
    new_len = v->len + len;
    if (new_len > v->cap) {
        char  *ptr_tmp;
        size_t cap_new;

        if (new_len > SIZE_MAX / 2) {
            cap_new = SIZE_MAX;
        } else {
            cap_new = v->cap ? v->cap : 1;
            while (cap_new < new_len) {
                cap_new *= 2;
            }
        }
        ptr_tmp = v->cap ? v->ptr : NULL;
        ptr_tmp = RMA_STR__REALLOC(ptr_tmp, cap_new);
        if (!ptr_tmp) {
            return RMA__OOM;
        }
        v->ptr = ptr_tmp;
        v->cap = cap_new;
    }
    memcpy(v->ptr + v->len, ptr, len);
    v->len += len;
    v->ptr[v->len] = '\0';
    return 0;
}

RMA_STR__STORAGE int RMA__JOIN(RMA_STR__NAME, __push_str)(RMA_STR__NAME *v, RMA_STR__NAME *other) {
    return RMA__JOIN(RMA_STR__NAME, __push_raw_parts)(v, other->len, other->ptr);
}

RMA_STR__STORAGE int RMA__JOIN(RMA_STR__NAME, __reserve)(RMA_STR__NAME *v, size_t cap) {
    char  *ptr_tmp;
    size_t cap_new;

    if (cap <= v->cap) {
        return 0;
    }
    if (cap > SIZE_MAX / 2) {
        cap_new = SIZE_MAX;
    } else {
        cap_new = v->cap ? v->cap : 1;
        while (cap_new < cap) {
            cap_new *= 2;
        }
    }
    ptr_tmp = v->cap ? v->ptr : NULL;
    ptr_tmp = RMA_STR__REALLOC(ptr_tmp, cap_new);
    if (!ptr_tmp) {
        return RMA__OOM;
    }
    v->ptr = ptr_tmp;
    v->cap = cap_new;
    return 0;
}

RMA_STR__STORAGE int RMA__JOIN(RMA_STR__NAME, __with_capacity)(RMA_STR__NAME *out, size_t cap) {
    if (cap > SIZE_MAX) {
        return RMA__OOM;
    }
    out->ptr = RMA_STR__MALLOC(cap);
    if (!out->ptr) {
        return RMA__OOM;
    }
    out->cap = cap;
    out->len = 0;
    return 0;
}

#endif /* RMA_STR__IMPLEMENTATION */
