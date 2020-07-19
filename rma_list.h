/* rma_list.h - 0.2.0
**   Type-safe generic dynamic arrays for C.
**
**   Dual-licensed under MIT or the UNLICENSE.
**
** Simple example:
** 
**     #include <stdio.h>
**     #include <stdlib.h>
**     #define RMA_LIST__IMPLEMENTATION
**     #define RMA_LIST__INNER char
**     #include "rma_list.h"
**     
**     int main(void) {
**         List_char v = List_char__new();
**         char *s = "Hello, World!";
**         
**         List_char__extend_from_parts(&v, strlen(s), s);
**         List_char__push(&v, '\0');
**         printf("%s\n", List_char__as_ptr(&v));
**         return 0;
**     }
*/

#include <string.h> /* memcpy */
#include <stddef.h> /* size_t */

#ifndef RMA_LIST__INNER
#  error "error: RMA_LIST__INNER is undefined"
#endif

#ifndef RMA_LIST__STORAGE
#  define RMA_LIST__STORAGE
#endif

#ifndef RMA__JOIN
#  define RMA__JOIN(x, y) RMA__JOIN_EXPAND_ARGS(x, y)
#  define RMA__JOIN_EXPAND_ARGS(x, y) x ## y
#endif

#ifndef RMA_LIST__NAME
#  ifndef RMA_LIST__NAME_PREFIX
#    define RMA_LIST__NAME_PREFIX List_
#  endif
#  ifndef RMA_LIST__NAME_SUFFIX
#    define RMA_LIST__NAME_SUFFIX RMA_LIST__INNER
#  endif
#  define RMA_LIST__NAME RMA__JOIN(RMA_LIST__NAME_PREFIX, RMA_LIST__NAME_SUFFIX)
#endif

#undef  RMA_LIST__CAP_MAX
#define RMA_LIST__CAP_MAX (SIZE_MAX / sizeof(RMA_LIST__INNER))

#ifndef RMA_LIST__FREE
#  include <stdlib.h> /* free, malloc, realloc */
#  define RMA_LIST__FREE(x)          free(x)
#  define RMA_LIST__MALLOC(x)        malloc(x)
#  define RMA_LIST__REALLOC(ptr, sz) realloc((ptr), (sz))
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

typedef struct RMA_LIST__NAME {
    RMA_LIST__INNER *ptr;
    size_t           cap;
    size_t           len;
} RMA_LIST__NAME;

RMA_LIST__STORAGE int  RMA__JOIN(RMA_LIST__NAME, __clone)(RMA_LIST__NAME *dst, RMA_LIST__NAME *src);
RMA_LIST__STORAGE int  RMA__JOIN(RMA_LIST__NAME, __extend_from_parts)(RMA_LIST__NAME *v, size_t len, const RMA_LIST__INNER *ptr);
RMA_LIST__STORAGE void RMA__JOIN(RMA_LIST__NAME, __free   )(RMA_LIST__NAME *v);
RMA_LIST__STORAGE int  RMA__JOIN(RMA_LIST__NAME, __pop    )(RMA_LIST__NAME *v, RMA_LIST__INNER *out);
RMA_LIST__STORAGE int  RMA__JOIN(RMA_LIST__NAME, __push   )(RMA_LIST__NAME *v, RMA_LIST__INNER elem);
RMA_LIST__STORAGE int  RMA__JOIN(RMA_LIST__NAME, __reserve)(RMA_LIST__NAME *v, size_t cap);
RMA_LIST__STORAGE int  RMA__JOIN(RMA_LIST__NAME, __with_capacity)(RMA_LIST__NAME *out, size_t cap);

static RMA_LIST__INNER * RMA__UNUSED
RMA__JOIN(RMA_LIST__NAME, __as_ptr)(RMA_LIST__NAME *v) {
    return v->ptr;
}

static size_t RMA__UNUSED
RMA__JOIN(RMA_LIST__NAME, __cap)(RMA_LIST__NAME *v) {
    return v->cap;
}

static int RMA__UNUSED
RMA__JOIN(RMA_LIST__NAME, __extend)(RMA_LIST__NAME *v, RMA_LIST__NAME *other) {
    return RMA__JOIN(RMA_LIST__NAME, __extend_from_parts)(v, other->len, other->ptr);
}

static size_t RMA__UNUSED
RMA__JOIN(RMA_LIST__NAME, __len)(RMA_LIST__NAME *v) {
    return v->len;
}

static RMA_LIST__NAME RMA__UNUSED
RMA__JOIN(RMA_LIST__NAME, __new)(void) {
    RMA_LIST__NAME v;
    v.ptr = (RMA_LIST__INNER *)RMA__ALIGNOF(RMA_LIST__INNER);
    v.cap = 0;
    v.len = 0;
    return v;
}

#ifdef RMA_LIST__IMPLEMENTATION

RMA_LIST__STORAGE int RMA__JOIN(RMA_LIST__NAME, __clone)(RMA_LIST__NAME *dst, RMA_LIST__NAME *src) {
#ifdef RMA_LIST__INNER_CLONE
    size_t i;
#endif

    dst->ptr = RMA_LIST__MALLOC(src->cap * sizeof(RMA_LIST__INNER));
    if (!dst->ptr) {
        return RMA__ENOMEM;
    }
#ifdef RMA_LIST__INNER_CLONE
    for (i=0; i < src->len; i++) {
        int err = RMA_LIST__INNER_CLONE(&(dst->ptr[i]), &(src->ptr[i]));
        if (err) {
            dst->len = i;
            RMA__JOIN(RMA_LIST__NAME, __free)(dst);
            return err;
        }
    }
#else
    memcpy(dst->ptr, src->ptr, src->len);
#endif
    dst->cap = src->cap;
    dst->len = src->len;
    return 0;
}

RMA_LIST__STORAGE int RMA__JOIN(RMA_LIST__NAME, __extend_from_parts)(RMA_LIST__NAME *v, size_t len, const RMA_LIST__INNER *ptr) {
    int err;

    if (v->len > RMA_LIST__CAP_MAX - len) {
        return RMA__ENOMEM;
    }
    err = RMA__JOIN(RMA_LIST__NAME, __reserve)(v, v->len + len);
    if (err) {
        return err;
    }
    memcpy(v->ptr + v->len, ptr, len);
    v->len += len;
    return 0;
}

RMA_LIST__STORAGE void RMA__JOIN(RMA_LIST__NAME, __free)(RMA_LIST__NAME *v) {
    if (v) {
#ifdef RMA_LIST__INNER_FREE
        size_t i;
        for (i=0; i < v->len; i++) {
            RMA_LIST__INNER_FREE(&(v->ptr[i]));
        }
#endif
        RMA_LIST__FREE(v->ptr);
    }
}

RMA_LIST__STORAGE int RMA__JOIN(RMA_LIST__NAME, __pop)(RMA_LIST__NAME *v, RMA_LIST__INNER *out) { 
    if (!v->len) {
        return -1;
    }
    v->len--;
    *out = v->ptr[v->len];
    return 0;
}

RMA_LIST__STORAGE int RMA__JOIN(RMA_LIST__NAME, __push)(RMA_LIST__NAME *v, RMA_LIST__INNER elem) {
    int err;

    if (v->len > RMA_LIST__CAP_MAX - 1) {
        return RMA__ENOMEM;
    }
    err = RMA__JOIN(RMA_LIST__NAME, __reserve)(v, v->len + 1);
    if (err) {
        return err;
    }
    v->ptr[v->len] = elem;
    v->len++;
    return 0;
}

RMA_LIST__STORAGE int RMA__JOIN(RMA_LIST__NAME, __reserve)(RMA_LIST__NAME *v, size_t cap) {
    RMA_LIST__INNER *ptr_tmp;  
    size_t           cap_new;

    if (cap <= v->cap) {
        return 0;
    }
    if (cap > RMA_LIST__CAP_MAX) {
        return RMA__ENOMEM;
    }
    if (cap > RMA_LIST__CAP_MAX / 2) {
        cap_new = RMA_LIST__CAP_MAX;
    } else {
        cap_new = v->cap ? v->cap : 1;
        while (cap_new < cap) {
            cap_new *= 2;
        }
    }
    ptr_tmp = v->cap ? v->ptr : NULL;
    ptr_tmp = RMA_LIST__REALLOC(ptr_tmp, cap_new * sizeof(RMA_LIST__INNER));
    if (!ptr_tmp) {
        return RMA__ENOMEM;
    }
    v->ptr = ptr_tmp;
    v->cap = cap_new;
    return 0;
}

RMA_LIST__STORAGE int RMA__JOIN(RMA_LIST__NAME, __with_capacity)(RMA_LIST__NAME *out, size_t cap) {
    if (cap > RMA_LIST__CAP_MAX) {
        return RMA__ENOMEM;
    }
    out->ptr = RMA_LIST__MALLOC(cap * sizeof(RMA_LIST__INNER));
    if (!out->ptr) {
        return RMA__ENOMEM;
    }
    out->cap = cap;
    out->len = 0;
    return 0;
}

#endif /* RMA_LIST__IMPLEMENTATION */
