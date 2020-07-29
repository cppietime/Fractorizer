#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <SLAV/slavio.h>
#include "prng.h"
#include "ifs.h"
#include "music.h"
#include "utils.h"
#include "wav.h"
#include "signal.h"
#include "constants.h"

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
	int m = 16;
	int u = 7;
	int n = 16;
	int t = 1;
	double tempo = 16.0;
	fract_track track = {
		m, u, n, t
	};
	long rate = 44100;
	fract_track_generate(&track, fract_sequence_hilbert);
	for(int i = 0; i < track.num_tracks; i++)
		track.programs[i] = 0;
	
	FILE *midi = fopen(out, "wb");
	char scale[] = {40, 2, 2, 1, 2, 2, 2, 1, 0, 80};
	fract_oscillator synth;
	fract_oscillator_init(&synth);
	float cutoffs[] = {1000 * 2 * M_PI / rate, 50 * 2 * M_PI / rate};
	fract_iir_butterworth(synth.filters, 4, cutoffs, 1, IIR_POLE);
	fract_iir_butterworth(synth.filters, 2, cutoffs + 1, 1, IIR_ZERO);
	float g = .5;
	fract_iir_delayline(synth.filters, rate * .1, rate * .2, &g, 1, IIR_POLE,
		&g, 1);
	synth.generator = wavgen_hsin;
	synth.pm_strength = 1;
	synth.pm_ratio = 4;
	synth.filter_arg = M_PI / rate;
	fflush(stdout);
	int32_t *samples = calloc(m * n * rate / tempo, sizeof(int32_t));
	fflush(stdout);
	fract_signal_from_track(&track, &synth, tempo, scale, rate, samples,
		m * n * rate / tempo);
	fract_oscillator_destroy(&synth);
	fflush(stdout);
	fract_signal_normalize(samples, m * n * rate / tempo, 32767, 0);
	fract_wavhead head = {rate, 1, 2};
	fract_write_wav(midi, &head, samples, m * n * rate / tempo);
	free(samples);
	// fract_write_midi_file(midi, &track, 8.0, 2, scale, 0, -1);
	fclose(midi);
	
	fract_track_destroy(&track);
	Bmp_free(img);
}

void test_instr(const char *in, const char *out, fract_lcg *lcg,
	fract_oscillator *instr){
	FILE *file = fopen(in, "rb");
	Bitmap *img = Bmp_load(file);
	fclose(file);
	
	fract_load_bitmap(img);
	int m = 16;
	int u = 7;
	int n = 16;
	int t = 3;
	double tempo = 8.0;
	fract_track track = {
		m, u, n, t
	};
	long rate = 44100;
	fract_track_generate(&track, fract_sequence_hilbert);
	for(int i = 0; i < track.num_tracks; i++)
		track.programs[i] = 0;
	
	char scale[] = {40, 2, 2, 1, 2, 2, 2, 1, 0, 64};
	FILE *wav = fopen(out, "wb");
	int32_t *samples = calloc(m * n * rate / tempo, sizeof(int32_t));
	fract_signal_from_track(&track, instr, tempo, scale, rate, samples,
		m * n * rate / tempo);
	fract_signal_normalize(samples, n * m * rate / tempo, 32767, 1);
	fract_wavhead head = {rate, 1, 2};
	fract_write_wav(wav, &head, samples, m * n * rate / tempo);
	fclose(wav);
	fract_track_destroy(&track);
	Bmp_free(img);
}

void test_instr_file(const char *img, const char *instrs, fract_lcg *lcg){
	FILE *src = fopen(instrs, "r");
	fseek(src, 0, SEEK_END);
	long end = ftell(src);
	fseek(src, 0, SEEK_SET);
	size_t index = 0;
	static char name[64];
	while(ftell(src) < end){
		fract_oscillator instr = fract_oscillator_from_file(src);
		sprintf(name, "test_i%d.wav", index);
		test_instr(img, name, lcg, &instr);
		fract_oscillator_destroy(&instr);
		index++;
	}
	fclose(src);
}

void test_sequence(
	const char *fractal, const char *swirl, const char *flame, fract_lcg *lcg){
	
	// img_to_sequence(fractal, "fractal.wav", lcg);
	img_to_sequence(swirl, "swirl.wav", lcg);
	// img_to_sequence(flame, "flame.wav", lcg);
}

void test_hsv(uint32_t c){
	float r, g, b;
	RGB2HSV(c, &r, &g, &b);
	printf("%06x -> %f %f %f\n", c, r, g, b);
}

int main(int argc, char **argv){
	
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	srand(now.tv_nsec);
	
	fract_lcg lcg;
	fract_lcg_default(&lcg);
	fract_load_lcg(&lcg);
	
	// test_fractal_noise(&lcg);
	// test_perlin_swirl(&lcg);
	// test_fractal_flame(&lcg);
	// test_sequence("fractal.bmp",
		// "swirl.bmp",
		// "flame.bmp",
		// &lcg);
	
	test_instr_file("C:/users/sellar.king/documents/github/fractorizer/swirl.bmp",
		"C:/users/sellar.king/documents/github/fractorizer/instr.txt", &lcg);
	
	printf("Complete\n");
	
	return 0;
}