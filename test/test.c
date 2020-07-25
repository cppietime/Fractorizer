#include <stdio.h>
#include <stdint.h>
#include <SLAV/slavio.h>
#include "prng.h"

int main(int argc, char **argv){
	
	fract_lcg lcg;
	fract_lcg_default(&lcg);
	
	fract_perlin perlin;
	fract_perlin_init(&perlin, &lcg, 256);
	
	fract_perlin colors;
	fract_perlin_init(&colors, &lcg, 4);
	uint32_t palette[] = {0xffffff, 0xff0000, 0x00ff00, 0x0000ff};
	
	uint8_t rgbs[] = {0xff, 0, 0, 0, 0xff, 0, 0, 0, 0xff};
	double res[] = {5, 10, 20};
	double offsets[] = {0, 0, 2, 0, 0, 2};
	uint8_t flags[] = {0, 0, 0};
	
	Bitmap *img = Bmp_empty(256, 256, 24, 0);
	img->R = img->G = img->B = 0;
	fract_perlin_fractal(&perlin, rgbs, res, offsets, flags, img, 3);
	
	FILE *file = fopen("image.bmp", "wb");
	Bmp_save(img, file);
	fclose(file);
	Bmp_free(img);
	fract_perlin_destroy(&perlin);
	fract_perlin_destroy(&colors);
	
	printf("Complete\n");
	
	return 0;
}