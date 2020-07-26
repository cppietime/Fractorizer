#include <stdio.h>
#include <stdint.h>
#include <SLAV/slavio.h>
#include "prng.h"
#include "ifs.h"
#include "music.h"
#include "utils.h"

void test_fractal_noise(fract_lcg *lcg){
	fract_perlin perlin;
	fract_perlin_init(&perlin, lcg, 256);
	
	uint8_t rgbs[] = {0xff, 0, 0, 0, 0xff, 0, 0, 0, 0xff};
	double res[] = {5, 10, 20};
	double offsets[] = {0, 0, 2, 0, 0, 2};
	uint8_t flags[] = {1, 1, 1};
	
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
	fract_perlin_init(&colors, lcg, 8);
	uint32_t palette[] = {0, 0xffffff, 0xff0000, 0x00ff00, 0x0000ff,
		0xffff00, 0xff00ff, 0x00ffff};
	
	Bitmap *img = Bmp_empty(256, 256, 24, 0);
	fract_perlin_swirl(&perlin, &colors, palette, 10, 10, 8, 2, 10, img);
	fract_box_blur(img, 2, 2);
	
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
	
	Bitmap *img = Bmp_empty(256, 256, 24, 0);
	fract_ifs_flame(&ifs, lcg, -.5, -.5, 1, 1, img, 10000000L, 1.5, 1);
	fract_ifs_destroy(&ifs);
	
	FILE *file = fopen("flame.bmp", "wb");
	Bmp_save(img, file);
	fclose(file);
	Bmp_free(img);
}

void img_to_sequence(const char *in, const char *out, fract_lcg *lcg){
	FILE *file = fopen(in, "rb");
	Bitmap *img = Bmp_load(file);
	fclose(file);
	
	fract_load_bitmap(img);
	fract_track track = {
		16, 7, 16, 3
	};
	fract_track_generate(&track, fract_sequence_hilbert);
	for(int i = 0; i < track.num_tracks; i++)
		track.programs[i] = fract_lcg_int(lcg, 112);
	
	FILE *midi = fopen(out, "wb");
	char scale[] = {40, 2, 2, 1, 2, 2, 2, 1, 0, 80};
	fract_write_midi_file(midi, &track, 8.0, 2, scale, 0, -1);
	fclose(midi);
	
	fract_track_destroy(&track);
	Bmp_free(img);
}

void test_sequence(
	const char *fractal, const char *swirl, const char *flame, fract_lcg *lcg){
	
	img_to_sequence(fractal, "fractal.mid", lcg);
	img_to_sequence(swirl, "swirl.mid", lcg);
	img_to_sequence(flame, "flame.mid", lcg);
}

void test_hsv(uint32_t c){
	float r, g, b;
	RGB2HSV(c, &r, &g, &b);
	printf("%06x -> %f %f %f\n", c, r, g, b);
}

int main(int argc, char **argv){
	
	fract_lcg lcg;
	fract_lcg_default(&lcg);
	fract_load_lcg(&lcg);
	
	test_fractal_noise(&lcg);
	test_perlin_swirl(&lcg);
	test_fractal_flame(&lcg);
	test_sequence("fractal.bmp",
		"swirl.bmp",
		"flame.bmp",
		&lcg);
	
	printf("Complete\n");
	
	return 0;
}