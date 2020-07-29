/**
 * YAAKOV SCHECTMAN 2020
 * perlin.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <SLAV/slavio.h>
#include "prng.h"
#include "utils.h"
#include "constants.h"

void
fract_perlin_init(fract_perlin *perl, fract_lcg *lcg, size_t size)
{
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

void
fract_perlin_destroy(fract_perlin *perl)
{
	free(perl->permutation);
	free(perl->gradients);
}

int
fract_perlin_index(fract_perlin *perl, int x, int y)
{
	int x_digest = fract_hash_digest(&perl->digest, x);
	int inner = perl->permutation[x_digest]
		+ fract_hash_digest(&perl->digest, y);
	return perl->permutation[inner % perl->size];
}

void
fract_perlin_gradient(fract_perlin *perl, int x, int y, double *dx, double *dy)
{
	int index = fract_perlin_index(perl, x, y);
	*dx = perl->gradients[index * 2];
	*dy = perl->gradients[index * 2 + 1];
}

double
fract_perlin_at(fract_perlin *perl, double x, double y)
{
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

void
fract_perlin_best(fract_perlin *perl, double x, double y, int *ox, int *oy)
{
	int x0 = floor(x), y0 = floor(y);
	int x1 = x0 + 1, y1 = y0 + 1;
	double dx = x - x0, dy = y - y0;
	double ix = smooth3(dx);
	double grads[8];
	fract_perlin_gradient(perl, x0, y0, grads + 0, grads + 1);
	fract_perlin_gradient(perl, x1, y0, grads + 2, grads + 3);
	fract_perlin_gradient(perl, x0, y1, grads + 4, grads + 5);
	fract_perlin_gradient(perl, x1, y1, grads + 6, grads + 7);
	double corrs[] = {
		dot2(dx		, dy	, grads[0], grads[1]),
		dot2(dx - 1	, dy	, grads[2], grads[3]),
		dot2(dx		, dy - 1, grads[4], grads[5]),
		dot2(dx - 1	, dy - 1, grads[6], grads[7])
	};
	int index = 0;
	for(int i = 1; i < 4; i++){
		if(corrs[i] > corrs[index])
			index = i;
	}
	*ox = x0 + (index & 1);
	*oy = y0 + ((index & 2 ) >> 1);
}

void
fract_perlin_swirl(
	fract_perlin *perl,
	fract_perlin *pal_indices,
	uint32_t *palette,
	double noise_resolution,
	double swirl_resolution,
	double swirl,
	double offset_x,
	double offset_y,
	Bitmap *img)
{
	int cx, cy;
	for(int y = 0; y < img->height; y++){
		double dy = y * noise_resolution / img->height;
		double ny = y * swirl_resolution / img->height;
		for(int x = 0; x < img->width; x++){
			double dx = x * noise_resolution / img->width;
			double nx = x * swirl_resolution / img->width;
			double disp_x = fract_perlin_at(perl, nx + offset_x, ny + offset_y)
				* swirl;
			double disp_y = fract_perlin_at(perl, nx + offset_y, ny + offset_x)
				* swirl;
			fract_perlin_best(perl, dx + disp_x, dy + disp_y, &cx, &cy);
			int index = fract_perlin_index(pal_indices, cx, cy);
			uint32_t color = palette[index];
			set_pixel(img, x, y, color);
		}
	}
}

void
fract_perlin_fractal(
	fract_perlin *perl,
	uint8_t *colors,
	double *resolutions,
	double *offsets,
	uint8_t *flags,
	Bitmap *img,
	size_t layers)
{
	for(int y = 0; y < img->height; y++){
		for(int x = 0; x < img->width; x++){
			float r = 0, g = 0, b = 0;
			for(int i = 0; i < layers; i++){
				float dx = x * resolutions[i] / img->width;
				float dy = y * resolutions[i] / img->height;
				double sample = fract_perlin_at(
					perl, dx + offsets[i * 2], dy + offsets[i * 2 + 1]);
				uint8_t flag = flags[i];
				sample = (flag & 1) ? fabs(sample) * M_SQRT2
					: (sample + M_SQRT1_2) * M_SQRT1_2;
				r += sample * colors[i * 3];
				g += sample * colors[i * 3 + 1];
				b += sample * colors[i * 3 + 2];
			}
			uint32_t color = (((int)(r) & 0xff) << 16) |
				(((int)(g) & 0xff) << 8) |
				((int)(b) & 0xff);
			set_pixel(img, x, y, color);
		}
	}
}
