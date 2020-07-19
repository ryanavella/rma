/* rma_str.h - 0.1.0
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

#include <string.h> /* memcpy, strcpy, strlen */
#include <stddef.h> /* size_t */

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

#ifndef RMA__ENOMEM
#  define RMA__ENOMEM 12
#endif

typedef struct RMA_STR__NAME {
    char  *ptr;
    size_t cap;
    size_t len;
} RMA_STR__NAME;

RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __clone)(RMA_STR__NAME *dst, RMA_STR__NAME *src);
RMA_STR__STORAGE void RMA__JOIN(RMA_STR__NAME, __free )(RMA_STR__NAME *v);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __from_cstr)(RMA_STR__NAME *v, const char *ptr);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __pop )(RMA_STR__NAME *v, char *out);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __push)(RMA_STR__NAME *v, char elem);
RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __push_raw_parts)(RMA_STR__NAME *v,   size_t len, const char *ptr);
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

static int RMA__UNUSED
RMA__JOIN(RMA_STR__NAME, __push_str)(RMA_STR__NAME *v, RMA_STR__NAME *other) {
    return RMA__JOIN(RMA_STR__NAME, __push_raw_parts)(v, other->len, other->ptr);
}

static int RMA__UNUSED
RMA__JOIN(RMA_STR__NAME, __push_cstr)(RMA_STR__NAME *v, const char *ptr) {
    return RMA__JOIN(RMA_STR__NAME, __push_raw_parts)(v, strlen(ptr), ptr);
}

#ifdef RMA_STR__IMPLEMENTATION

RMA_STR__STORAGE int RMA__JOIN(RMA_STR__NAME, __clone)(RMA_STR__NAME *dst, RMA_STR__NAME *src) {
    dst->ptr = RMA_STR__MALLOC(src->cap);
    if (!dst->ptr) {
        return RMA__ENOMEM;
    }
    memcpy(dst->ptr, src->ptr, src->len);
    dst->cap = src->cap;
    dst->len = src->len;
    return 0;
}

RMA_STR__STORAGE int  RMA__JOIN(RMA_STR__NAME, __push_raw_parts)(RMA_STR__NAME *v, size_t len, const char *ptr) {
    size_t new_len;

    if (v->len > SIZE_MAX - len - 1) {
        return RMA__ENOMEM;
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
            return RMA__ENOMEM;
        }
        v->ptr = ptr_tmp;
        v->cap = cap_new;
    }
    memcpy(v->ptr + v->len, ptr, len);
    v->len += len;
    v->ptr[v->len] = '\0';
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
        return RMA__ENOMEM;
    }
    strcpy(v->ptr, ptr);
    v->cap = v->len + 1;
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
    if (v->len + 1 >= v->cap) {   
        char  *ptr_tmp;  
        size_t cap_new;

        if (v->cap <= SIZE_MAX / 2) {  
            cap_new = v->cap ? 2 * v->cap : 1;
        } else if (v->cap < SIZE_MAX) {     
            cap_new = SIZE_MAX;
        } else {       
            return RMA__ENOMEM;
        }
        ptr_tmp = v->cap ? v->ptr : NULL;
        ptr_tmp = RMA_STR__REALLOC(ptr_tmp, cap_new);
        if (!ptr_tmp) {
            return RMA__ENOMEM;
        }
        v->ptr = ptr_tmp;
        v->cap = cap_new;
    }
    v->ptr[v->len] = elem;
    v->len++;
    v->ptr[v->len] = '\0';
    return 0;
}

RMA_STR__STORAGE int RMA__JOIN(RMA_STR__NAME, __with_capacity)(RMA_STR__NAME *out, size_t cap) {
    if (cap > SIZE_MAX) {
        return RMA__ENOMEM;
    }
    out->ptr = RMA_STR__MALLOC(cap);
    if (!out->ptr) {
        return RMA__ENOMEM;
    }
    out->cap = cap;
    out->len = 0;
    return 0;
}

#endif /* RMA_STR__IMPLEMENTATION */
