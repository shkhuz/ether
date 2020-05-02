#pragma once

#include <typedef.hpp>
#include <stddef.h>

struct BufHdr {
	u64 len;
	u64 cap;
	char buf[];
};

#ifndef __cplusplus
typedef struct BufHdr BufHdr;
#endif

#define buf__hdr(b) ((BufHdr*)((char*)(b) - offsetof(BufHdr, buf)))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_sizeof(b) ((b) ? buf_len(b) * sizeof(*b) : 0)

#define buf_free(b)		   ((b) ? (free(buf__hdr(b)), (b) = null) : 0)
#define buf_fit(b, n)	   ((n) <= buf_cap(b) ? 0 :	\
							((b) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...)   (buf_fit((b), 1 + buf_len(b)), \
							(b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_pop(b)		   ((b) ? buf__shrink((b), 1) : 0)
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_clear(b)	   ((b) ? buf__hdr(b)->len = 0 : 0)
#define buf_empty(b)	   ((b) ? (buf_len(b) == 0 ? true : false) : 0)
#define buf_last(b)		   ((b) ? ((!buf_empty(b)) ? (b)[buf_len(b)-1] : 0) : 0)

#define buf_loop(buf, c) for (u64 c = 0; c < buf_len(buf); ++c) 

void* buf__grow_raw(const void* buf, u64 new_len, u64 elem_size);

#ifdef __cplusplus
template <class T>
static T* buf__grow(T* buf, u64 new_len, u64 elem_size) {
	return (T*)buf__grow_raw((void*)buf, new_len, elem_size);
}
#else
#define buf__grow buf__grow_raw
#endif

void buf__shrink(const void* buf, u64 size);
