#ifndef NUMBER_THEORY_H
#define NUMBER_THEORY_H

#include <stdbool.h>
#include <stdint.h>

uint64_t gcd(uint64_t a, uint64_t b);
bool is_prime(uint32_t n);
uint32_t random_prime(uint32_t min, uint32_t max);
uint64_t mod_inverse(uint64_t a, uint64_t modulus);
uint64_t mod_pow(uint64_t base, uint64_t exponent, uint64_t modulus);

#endif