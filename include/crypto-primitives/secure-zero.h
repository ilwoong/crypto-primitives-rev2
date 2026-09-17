#ifndef CRYPTO_PRIMITIVES_SECURE_ZERO_H
#define CRYPTO_PRIMITIVES_SECURE_ZERO_H

#include <stddef.h>
#include <stdint.h>

// volatile keeps the compiler from dropping the stores as dead once the context is no longer read.
static inline void secure_zero(void *p, size_t n)
{
    volatile uint8_t *v = (volatile uint8_t *)p;
    for (size_t i = 0; i < n; ++i) {
        v[i] = 0;
    }
}

#endif
