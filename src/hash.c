#include "prng.h"

void fract_hash_random(fract_hash *hash, fract_lcg *lcg, int modulus){
	hash->multiplier = (fract_lcg_int(lcg, ((modulus - 2) / 4 - 1) / 2) * 2 + 1) * 4 + 1;
	hash->adden = fract_lcg_int(lcg, (modulus - 2) / 2) * 2 + 1;
	hash->modulus = modulus;
}

int fract_hash_digest(fract_hash *hash, int x){
	return (hash->multiplier * x + hash->adden) % hash->modulus;
}