#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "prng.h"
#include "utils.h"

void fract_perlin_init(fract_perlin *perl, fract_lcg *lcg, size_t size){
	perl->size = size;
	perl->permutation = malloc(sizeof(int) * size);
	perl->gradients = malloc(sizeof(double) * size * 2);
	for(int i = 0; i < size; i++){
		perl->permutation[i] = i;
		double theta = 2 * M_PI * fract_lcg_uniform(lcg);
		perl->gradients[i * 2] = cos(theta);
		perl->gradients[i * 2 + 1] = sin(theta);
	}
	fract_lcg_permute(lcg, perl->permutation, size);
	fract_hash_random(&perl->digest, lcg, size);
}

void fract_perlin_destroy(fract_perlin *perl){
	free(perl->permutation);
	free(perl->gradients);
}

void fract_perlin_gradient(fract_perlin *perl, int x, int y, double *dx, double *dy){
	int index = perl->permutation[(perl->permutation[fract_hash_digest(&perl->digest, x)] + fract_hash_digest(&perl->digest, y)) % perl->size];
	*dx = perl->gradients[index * 2];
	*dy = perl->gradients[index * 2 + 1];
}

double fract_perlin_at(fract_perlin *perl, double x, double y){
	int x0 = floor(x), y0 = floor(y);
	int x1 = x0 + 1, y1 = y0 + 1;
	double dx = x - x0, dy = y - y0;
	double ix = smooth3(dx);
	double grads[8];
	fract_perlin_gradient(perl, x0, y0, grads + 0, grads + 1);
	fract_perlin_gradient(perl, x1, y0, grads + 2, grads + 3);
	fract_perlin_gradient(perl, x0, y1, grads + 4, grads + 5);
	fract_perlin_gradient(perl, x1, y1, grads + 6, grads + 7);
	double vx0 = dot2(dx    , dy    , grads[0], grads[1]);
	double vx1 = dot2(dx - 1, dy    , grads[2], grads[3]);
	double vy0 = lin_interp(vx0, vx1, ix);
	vx0        = dot2(dx    , dy - 1, grads[4], grads[5]);
	vx1        = dot2(dx - 1, dy - 1, grads[6], grads[7]);
	double vy1 = lin_interp(vx0, vx1, ix);
	return lin_interp(vy0, vy1, smooth3(dy));
}