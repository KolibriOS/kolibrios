
#include <stdint.h>

void TEA_encrypt (uint32_t block[2], uint32_t key[4])
{
    /* set up */
    uint32_t v0 = block[0];
    uint32_t v1 = block[1];
    uint32_t sum = 0;
    uint32_t i;

    /* a key schedule constant */
    uint32_t delta = 0x9e3779b9;

    /* cache key */
    uint32_t k0 = key[0];
    uint32_t k1 = key[1];
    uint32_t k2 = key[2];
    uint32_t k3 = key[3];

    /* basic cycle start */
    for (i = 0; i < 32; i++)
    {
        sum += delta;
        v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
    }
    /* end cycle */

    block[0] = v0;
    block[1] = v1;
}

void TEA_decrypt (uint32_t* block, uint32_t* key)
{
    /* set up */
    uint32_t v0 = block[0];
    uint32_t v1 = block[1];
    uint32_t sum = 0xC6EF3720;
    uint32_t i;

    /* a key schedule constant */
    uint32_t delta = 0x9e3779b9;

    /* cache key */
    uint32_t k0 = key[0];
    uint32_t k1 = key[1];
    uint32_t k2 = key[2];
    uint32_t k3 = key[3];        

    /* basic cycle start */
    for (i = 0; i < 32; i++)
    {                              
        v1 -= ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
        v0 -= ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        sum -= delta;                                   
    }
    /* end cycle */

    block[0] = v0;
    block[1] = v1;
} 
