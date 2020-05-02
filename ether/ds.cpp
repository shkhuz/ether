#include <stdlib.h>
#include <assert.h>
#include <ds.hpp>
#include <math.hpp>

void* buf__grow_raw(const void* buf, u64 new_len, u64 elem_size) {
	assert(buf_cap(buf) <= (__SIZE_MAX__ - 1) / 2);
	u64 new_cap = CLAMP_MIN(2 * buf_cap(buf), MAX(new_len, 16));
	assert(new_len <= new_cap);
	assert(new_cap <= (__SIZE_MAX__ - offsetof(BufHdr, buf)) / elem_size);
	u64 new_size = offsetof(BufHdr, buf) + (new_cap * elem_size);
	BufHdr* new_hdr;

	if (buf) {
		new_hdr = (BufHdr*)realloc(buf__hdr(buf), new_size);
	}
	else {
		new_hdr = (BufHdr*)malloc(new_size);
		new_hdr->len = 0;
	}

	new_hdr->cap = new_cap;
	return new_hdr->buf;
}

void buf__shrink(const void* buf, u64 size) {
	if (size > buf_len(buf)) {
		size = buf_len(buf);
	}

	buf__hdr(buf)->len -= size;
}
