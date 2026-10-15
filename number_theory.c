#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "number_theory.h"

uint64_t gcd(uint64_t a, uint64_t b) {
    while (b) {
        uint64_t reste = a % b;
        a = b;
        b = reste;
    }
    return a;
}

bool is_prime(uint32_t n) {
    if (n < 2) return false;
    
    for (uint64_t div = 2; div * div <= n; ++div) {
        if (n % div == 0) return false;
    }
    
    return true;
}

uint32_t random_prime(uint32_t min, uint32_t max) {
    while (1) {
        uint32_t candidat = min + (uint32_t)rand() % (max - min);
        if (is_prime(candidat)) return candidat;
    }
}

uint64_t mod_inverse(uint64_t a, uint64_t modulus) {
    int64_t ancien_r = (int64_t)a;
    int64_t r = (int64_t)modulus;
    int64_t ancien_c = 1;
    int64_t c = 0;
    
    while (r) {
        int64_t q = ancien_r / r;
        
        int64_t tmp_r = ancien_r - q * r;
        ancien_r = r;
        r = tmp_r;
        
        int64_t tmp_c = ancien_c - q * c;
        ancien_c = c;
        c = tmp_c;
    }
    
    if (ancien_c < 0) ancien_c += (int64_t)modulus;
    
    return (uint64_t)ancien_c;
}

uint64_t mod_pow(uint64_t base, uint64_t exponent, uint64_t modulus) {
    uint64_t resultat = 1;
    base %= modulus;
    
    // Algorithme d'exponentiation modulaire rapide : évite le débordement de capacité (overflow) et termine le calcul en temps logarithmique O(log(e)) plutôt que linéaire
    while (exponent > 0) {
        if (exponent % 2 == 1) {
            resultat = (resultat * base) % modulus;
        }
        
        base = (base * base) % modulus;
        exponent /= 2;
    }
    
    return resultat;
}