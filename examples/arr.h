/*
Copyright © 2021 Theodor Totev

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ARR_H_
#define ARR_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ARR_MALLOC) && defined(ARR_FREE) && defined(ARR_REALLOC)
// ok
#elif !defined(ARR_MALLOC) && !defined(ARR_FREE) && !defined(ARR_REALLOC)
#include <stdlib.h>
#define ARR_MALLOC(sz)     malloc(sz)
#define ARR_FREE(p)        free(p)
#define ARR_REALLOC(p, sz) realloc(p, sz)
#else
#error "Must define all or none of ARR_MALLOC, ARR_FREE, and ARR_REALLOC."
#endif

#include <stdint.h>

// The used elements
#define ARR_LEN(DATA)  (((int64_t*)(DATA))[-1])
// The allocated elements
#define ARR_CAP(DATA)  (((int64_t*)(DATA))[-2])
// The size of a single element
#define ARR_SIZE(DATA) (((int64_t*)(DATA))[-3])

#define arr_len(DATA) ARR_LEN(DATA)

#define ARR_DATA_OFFSET (3 * sizeof(int64_t))

#define ARR_BASE(DATA) ((void*)(DATA) - ARR_DATA_OFFSET)
#define ARR_DATA(BASE) ((void*)(BASE) + ARR_DATA_OFFSET)

#define ARR_HIGHER_CAP(DATA) (ARR_CAP(DATA) == 0 ? 1 : 2 * ARR_CAP(DATA))
#define ARR_LOWER_CAP(DATA)  (ARR_CAP(DATA) / 2)

void *arr_create_(int64_t size) {
    void *data = ARR_DATA(ARR_MALLOC(ARR_DATA_OFFSET + size));
    ARR_LEN(data) = 0;
    ARR_CAP(data) = 1; // We keep place for a new 'pushed' element
    ARR_SIZE(data) = size;
    return data;
}
#define arr_create(T) (T*)arr_create_(sizeof(T))

#define arr_free(data) ARR_FREE(ARR_BASE(data))

void arr_reserve_(void **data, int64_t cap) {
    cap += 1; // We keep place for a new 'pushed' element
    void *base = ARR_BASE(*data);
    base = ARR_REALLOC(base, ARR_DATA_OFFSET + cap*ARR_SIZE(*data));
    *data = ARR_DATA(base);
    ARR_CAP(*data) = cap;
}
#define arr_reserve(data, cap) arr_reserve_((void**)data, cap)

void arr_resize_(void **data, int64_t len) {
    if (len > ARR_LEN(*data)) {
        ARR_LEN(*data) = len;
        while (ARR_LEN(*data)+1 > ARR_CAP(*data)) // We keep place for a new 'pushed' element
            arr_reserve_(data, ARR_HIGHER_CAP(*data));
    } else if (len < ARR_LEN(*data)) {
        ARR_LEN(*data) = len;
        while (ARR_LEN(*data)+1 <= ARR_LOWER_CAP(*data)) // We keep the most recent 'popped' element
            arr_reserve_(data, ARR_LOWER_CAP(*data));
    }
}
#define arr_resize(data, len) arr_resize_((void**)(data), len)

#define arr_push(data, value) ( (*(data))[arr_len(*(data))] = (value), arr_resize((data), arr_len(*data)+1) )

#define arr_pop(data) ( arr_resize((data), arr_len(*data)-1), (*(data))[arr_len(*(data))] = (*(data))[arr_len(*(data))] )

#define arr_end(data) ((data) + arr_len(data))

#ifdef __cplusplus
}
#endif

#endif // ARR_H_
