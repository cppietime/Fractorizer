/**
 * YAAKOV SCHECTMAN 2020
 * utils.c
 */

#include <stdint.h>
#include "utils.h"

int
fract_nlz(uint64_t x){
	int n = 64;
	for(int b = 32; b > 0; b >>= 1){
		uint64_t y = x >> b;
		if(y){
			n -= b;
			x = y;
		}
	}
	return n - x;
}