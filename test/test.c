#include <stdio.h>
#include <stdint.h>
#include <SLAV/slavio.h>
#include "prng.h"
#include "ifs.h"
#include "music.h"

void test_fractal_noise(fract_lcg *lcg){
	fract_perlin perlin;
	fract_perlin_init(&perlin, lcg, 256);
	
	uint8_t rgbs[] = {0xff, 0, 0, 0, 0xff, 0, 0, 0, 0xff};
	double res[] = {5, 10, 20};
	double offsets[] = {0, 0, 2, 0, 0, 2};
	uint8_t flags[] = {0, 0, 0};
	
	Bitmap *img = Bmp_empty(256, 256, 24, 0);
	fract_perlin_fractal(&perlin, rgbs, res, offsets, flags, img, 3);
	
	FILE *file = fopen("fractal.bmp", "wb");
	Bmp_save(img, file);
	fclose(file);
	Bmp_free(img);
	fract_perlin_destroy(&perlin);
}

void test_perlin_swirl(fract_lcg *lcg){
	fract_perlin perlin;
	fract_perlin_init(&perlin, lcg, 256);
	
	fract_perlin colors;
	fract_perlin_init(&colors, lcg, 4);
	uint32_t palette[] = {0, 1, 2, 3};
	
	Bitmap *img = Bmp_empty(256, 256, 8, 4);
	img->palette[0] = 0xffffff;
	img->palette[1] = 0xff0000;
	img->palette[2] = 0x00ffff;
	img->palette[3] = 0xffff00;
	fract_perlin_swirl(&perlin, &colors, palette, 10, 4, 10, 2, 10, img);
	
	FILE *file = fopen("swirl.bmp", "wb");
	Bmp_save(img, file);
	fclose(file);
	Bmp_free(img);
	fract_perlin_destroy(&perlin);
	fract_perlin_destroy(&colors);
}

void test_fractal_flame(fract_lcg *lcg){
	uint8_t colors[] = {0xff, 0xff, 0xff,
		0xff, 0, 0,
		0, 0xff, 0,
		0, 0, 0xff,
		0xff, 0xff, 0,
		0xff, 0, 0xff,
		0, 0xff, 0xff};
	fract_ifs ifs;
	fract_ifs_init(&ifs, lcg, colors, 7);
	
	Bitmap *img = Bmp_empty(512, 512, 24, 0);
	fract_ifs_flame(&ifs, lcg, -2, -2, 4, 4, img, 10000000L, 2, 3);
	fract_ifs_destroy(&ifs);
	
	FILE *file = fopen("flame.bmp", "wb");
	Bmp_save(img, file);
	fclose(file);
	Bmp_free(img);
}

void test_sequence(const char *fname){
	FILE *file = fopen(fname, "rb");
	Bitmap *img = Bmp_load(file);
	fclose(file);
	
	fract_load_bitmap(img);
	fract_track track = {
		4, 4, 8, 1
	};
	fract_track_generate(&track, fract_sequence_linear);
	
	Bmp_free(img);
}

int main(int argc, char **argv){
	
	fract_lcg lcg;
	fract_lcg_default(&lcg);
	
	// test_fractal_noise(&lcg);
	// test_perlin_swirl(&lcg);
	// test_fractal_flame(&lcg);
	test_sequence("flame.bmp");
	
	printf("Complete\n");
	
	return 0;
}