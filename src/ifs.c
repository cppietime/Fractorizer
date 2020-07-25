/**
 * YAAKOV SCHECTMAN 2020
 * ifs.c
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "prng.h"
#include "ifs.h"

void
fract_affine_transform(
	fract_affine affine,
	double x, double y,
	double *ox, double *oy)
{
	*ox = x * affine.linear_transform[0]
		+ y * affine.linear_transform[1] + affine.translation[0];
	*oy = y * affine.linear_transform[2]
		+ x * affine.linear_transform[3] + affine.translation[1];
}

void
fract_ifs_init(
	fract_ifs *ifs,
	fract_lcg *lcg,
	uint8_t *colors,
	size_t num_colors)
{
	size_t count;
	for(count = 0; fract_variations[count] != NULL; count++);
	ifs->probability = malloc(sizeof(float) * count);
	ifs->affines = malloc(sizeof(fract_affine) * count);
	ifs->color = malloc(sizeof(uint8_t) * count * 3);
	ifs->parameters = malloc(sizeof(double) * count * 4);
	double sum = 0;
	for(int i = 0; i < count; i++){
		ifs->probability[i] = fract_lcg_uniform(lcg);
		sum += ifs->probability[i];
		for(int j = 0; j < 4; j++){
			ifs->affines[i].linear_transform[j] = fract_lcg_gaussian(lcg);
			ifs->parameters[i * 4 + j] = fract_lcg_gaussian(lcg);
		}
		ifs->affines[i].translation[0] = fract_lcg_gaussian(lcg);
		ifs->affines[i].translation[1] = fract_lcg_gaussian(lcg);
		size_t color = fract_lcg_int(lcg, num_colors);
		memcpy(ifs->color + i * 3, colors + color * 3, 3);
	}
	for(int i = 0; i < count; i++)
		ifs->probability[i] /= sum;
}

void
fract_ifs_destroy(fract_ifs *ifs)
{
	free(ifs->probability);
	free(ifs->affines);
	free(ifs->color);
	free(ifs->parameters);
}

void
fract_ifs_iterate(
	fract_ifs *ifs,
	fract_lcg *lcg,
	double x, double y,
	double *ox, double *oy,
	uint8_t *r, uint8_t *g, uint8_t *b)
{
	double cumulative = fract_lcg_uniform(lcg);
	size_t choice;
	for(choice = 0; cumulative >= 0; choice++)
		cumulative -= ifs->probability[choice];
	choice--;
	fract_affine_transform(ifs->affines[choice], x, y, &x, &y);
	fract_variations[choice](
		x, y,
		ifs->affines[choice],
		lcg,
		ifs->parameters + choice * 4,
		ox, oy);
	*r = ifs->color[choice * 3];
	*g = ifs->color[choice * 3 + 1];
	*b = ifs->color[choice * 3 + 2];
}

void
fract_ifs_flame(
	fract_ifs *ifs,
	fract_lcg *lcg,
	double x0, double y0,
	double width, double height,
	Bitmap *img,
	unsigned long iterations,
	double gamma,
	int super_sample)
{
	int ss_incr = super_sample * super_sample;
	unsigned long *colors = calloc(ss_incr * img->width * img->height * 3,
		sizeof(unsigned long));
	unsigned long *freqs = calloc(ss_incr * img->width * img->height,
		sizeof(unsigned long));
	unsigned long freq_max = 0;
	double x = x0 + fract_lcg_uniform(lcg) * width;
	double y = y0 + fract_lcg_uniform(lcg) * height;
	uint8_t r, g, b;
	for(unsigned long i = 0; i < iterations; i++){
		fract_ifs_iterate(ifs, lcg, x, y, &x, &y, &r, &g, &b);
		int px = (x - x0) * super_sample / width * img->width;
		int py = (y - y0) * super_sample / height * img->height;
		if(px >= 0 && px < img->width * super_sample
			&& py >= 0 && py < img->height * super_sample){
			size_t index = py * img->width * super_sample + px;
			colors[index * 3] += r;
			colors[index * 3] /= 2;
			colors[index * 3 + 1] += g;
			colors[index * 3 + 1] /= 2;
			colors[index * 3 + 2] += b;
			colors[index * 3 + 2] /= 2;
			unsigned long freq = freqs[index] + 1;
			if(freq > freq_max)
				freq_max = freq;
			freqs[index] = freq;
		}
	}
	size_t index = 0;
	for(int y = 0; y < img->height; y++){
		for(int x = 0; x < img->width; x++){
			index = y * ss_incr * img->width + x * super_sample;
			float freq = 0;
			int fr = 0, fg = 0, fb = 0;
			for(int sy = 0; sy < super_sample; sy ++){
				for(int sx = 0; sx < super_sample; sx ++){
					size_t super_index =
						index + sy * img->width * super_sample + sx;
					float cell = freqs[super_index];
					freq += cell;
					fr += colors[super_index * 3] * cell;
					fg += colors[super_index * 3 + 1] * cell;
					fb += colors[super_index * 3 + 2] * cell;
					
				}
			}
			float alpha = log(freq / ss_incr) / log(freq_max);
			float denom = pow(alpha, 1.0 / gamma) / freq;
			fr *= denom;
			fg *= denom;
			fb *= denom;
			if(fr > 0xff || fg > 0xff || fb > 0xff){
				fprintf(stderr, "Too big @%d %d = %d %d %d\n", x, y, fr, fg, fb);
			}
			uint32_t color = (fr << 16) | (fg << 8) | fb;
			set_pixel(img, x, y, color);
		}
	}
}