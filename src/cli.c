/**
 * YAAKOV SCHECTMAN 2020
 * cli.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include "prng.h"
#include "ifs.h"
#include "utils.h"
#include "cli.h"
#include "music.h"

void
parse_bytes(char *toks, uint8_t *dst, char delim)
{
	size_t i = 0;
	while(1){
		char *end = strchr(toks, delim);
		if(end == NULL)
			end = toks + strlen(toks);
		dst[i] = strtol(toks, NULL, 10);
		if(*end == 0)
			break;
		toks = end + 1;
		i++;
	}
}

void
parse_doubles(char *toks, double *dst, char delim)
{
	size_t i = 0;
	while(1){
		char *end = strchr(toks, delim);
		if(end == NULL)
			end = toks + strlen(toks);
		dst[i] = strtod(toks, NULL);
		if(*end == 0)
			break;
		toks = end + 1;
		i++;
	}
}

void
fract_save_noise(int argc, char **argv)
{
	fract_lcg lcg;
	fract_lcg_default(&lcg);
	char *filename = NULL;
	uint8_t *colors = NULL;
	double *resolutions = NULL;
	double *offsets = NULL;
	uint8_t *flags = NULL;
	size_t layers = 0;
	int size = 256;
	int c;
	while((c = getopt(argc, argv, "l:o:c:r:x:f:s:")) != -1){
		switch(c){
			case 'l':
				layers = strtol(optarg, NULL, 10);
				if(layers <= 0){
					fprintf(stderr, "-l must be > 0, setting to 1\n");
					layers = 1;
				}
				colors = calloc(3 * layers, 1);
				resolutions = calloc(layers, sizeof(double));
				offsets = calloc(layers * 2, sizeof(double));
				for(size_t i = 0; i < layers; i++){
					colors[i] = fract_lcg_int(&lcg, 256);
					colors[i * 3 + 1] = fract_lcg_int(&lcg, 256);
					colors[i * 3 + 2] = fract_lcg_int(&lcg, 256);
					resolutions[i] = fract_lcg_uniform(&lcg) * 100;
					offsets[i] = fract_lcg_gaussian(&lcg);
				}
				flags = calloc(layers, 1);
				break;
			case 'o':
				filename = strdup(optarg);
				break;
			case 'c':
				if(colors == NULL)
					fprintf(stderr, "Call -l before -%c\n", c);
				else
					parse_bytes(optarg, colors, ',');
				break;
			case 'r':
				if(resolutions == NULL)
					fprintf(stderr, "Call -l before -%c\n", c);
				else
					parse_doubles(optarg, resolutions, ',');
				break;
			case 'x':
				if(offsets == NULL)
					fprintf(stderr, "Call -l before -%c\n", c);
				else
					parse_doubles(optarg, offsets, ',');
				break;
			case 'f':
				if(flags == NULL)
					fprintf(stderr, "Call -l before -%c\n", c);
				else
					parse_bytes(optarg, flags, ',');
				break;
			case 's':
				size = strtol(optarg, NULL, 10);
				break;
		}
	}
	FILE *file = filename ? fopen(filename, "wb") : stdout;
	if(file == NULL){
		fprintf(stderr, "Could not open file %s\n", filename);
	}else if(colors == NULL || flags == NULL || offsets == NULL ||
		resolutions == NULL){
		fprintf(stderr, "You must call -l to specify layers\n");
	}else{
		fract_perlin perlin;
		fract_perlin_init(&perlin, &lcg, 256);
		Bitmap *img = Bmp_empty(size, size, 24, 0);
		fract_perlin_fractal(&perlin, colors, resolutions, offsets, flags,
			img, layers);
		Bmp_save(img, file);
		if(filename)fclose(file);
		Bmp_free(img);
		fract_perlin_destroy(&perlin);
	}
	if(colors)free(colors);
	if(resolutions)free(resolutions);
	if(offsets)free(offsets);
	if(flags)free(flags);
	if(filename)free(filename);
}

void
fract_save_swirl(int argc, char **argv)
{
	fract_lcg lcg;
	fract_lcg_default(&lcg);
	double noise_res = fract_lcg_uniform(&lcg) * 40;
	double swirl_res = fract_lcg_uniform(&lcg) * 40;
	double swirl = fract_lcg_uniform(&lcg) * 20;
	double off_x = fract_lcg_gaussian(&lcg);
	double off_y = fract_lcg_gaussian(&lcg);
	int blur = 0;
	uint8_t *rgb = NULL;
	size_t colors = 0, side = 256;
	char *filename = NULL;
	int c;
	while((c = getopt(argc, argv, "o:n:r:w:x:y:s:b:l:c:")) != -1){
		switch(c){
			case 'o':
				filename = strdup(optarg);
				break;
			case 'n':
				noise_res = strtod(optarg, NULL);
				break;
			case 'r':
				swirl_res = strtod(optarg, NULL);
				break;
			case 'w':
				swirl = strtod(optarg, NULL);
				break;
			case 'x':
				off_x = strtod(optarg, NULL);
				break;
			case 'y':
				off_y = strtod(optarg, NULL);
				break;
			case 's':
				side = strtol(optarg, NULL, 10);
				break;
			case 'b':
				blur = strtol(optarg, NULL, 10);
				break;
			case 'l':
				colors = strtol(optarg, NULL, 10);
				rgb = malloc(3 * colors);
				for(size_t i = 0; i < colors * 3; i++){
					rgb[i] = fract_lcg_int(&lcg, 256);
				}
				break;
			case 'c':
				if(rgb == NULL){
					fprintf(stderr, "Must call -l before -c\n");
				}else
					parse_bytes(optarg, rgb, ',');
				break;
		}
	}
	FILE *file = filename ? fopen(filename, "wb") : stdout;
	if(file == NULL){
		fprintf(stderr, "Could not open file %s\n", filename);
	}else{
		fract_perlin perlin;
		fract_perlin_init(&perlin, &lcg, 256);
		fract_perlin pal;
		fract_perlin_init(&pal, &lcg, colors);
		uint32_t *palette = malloc(sizeof(uint32_t) * colors);
		for(size_t i = 0; i < colors; i++){
			uint32_t color = (rgb[i * 3] << 16) | (rgb[i * 3 + 1] << 8)
				| rgb[i * 3 + 2];
			palette[i] = color;
		}
		Bitmap *img = Bmp_empty(side, side, 24, 0);
		fract_perlin_swirl(&perlin, &pal, palette, noise_res, swirl_res, swirl,
			off_x, off_y, img);
		free(palette);
		if(blur)
			fract_box_blur(img, blur, blur);
		Bmp_save(img, file);
		Bmp_free(img);
		if(filename)fclose(file);
		fract_perlin_destroy(&perlin);
		fract_perlin_destroy(&pal);
	}
	if(rgb)free(rgb);
	if(filename)free(filename);
}

void
fract_save_flame(int argc, char **argv)
{
	fract_lcg lcg;
	fract_lcg_default(&lcg);
	uint8_t *colors = NULL;
	size_t num_colors = 0;
	double x0 = fract_lcg_gaussian(&lcg), y0 = fract_lcg_gaussian(&lcg);
	double width = fract_lcg_uniform(&lcg) * 10;
	double height = fract_lcg_uniform(&lcg) * 10;
	char *filename = NULL;
	unsigned long iters = 1000000;
	double gamma = 1.1;
	int sample = 1;
	size_t side = 256;
	int c;
	while((c = getopt(argc, argv, "l:c:x:y:X:Y:i:g:k:s:o:")) != -1){
		switch(c){
			case 'l':
				num_colors = strtol(optarg, NULL, 10);
				colors = malloc(3 * num_colors);
				break;
			case 'c':
				if(colors == NULL)
					fprintf(stderr, "You must call -l before -c\n");
				else
					parse_bytes(optarg, colors, ',');
				break;
			case 'x':
				x0 = strtod(optarg, NULL);
				break;
			case 'y':
				y0 = strtod(optarg, NULL);
				break;
			case 'X':
				width = strtod(optarg, NULL);
				break;
			case 'Y':
				height = strtod(optarg, NULL);
				break;
			case 'i':
				iters = strtol(optarg, NULL, 10);
				break;
			case 'g':
				gamma = strtod(optarg, NULL);
				break;
			case 'k':
				sample = strtol(optarg, NULL, 10);
				break;
			case 's':
				side = strtol(optarg, NULL, 10);
				break;
			case 'o':
				filename = strdup(optarg);
				break;
		}
	}
	FILE *file = filename ? fopen(filename, "wb") : stdout;
	if(file == NULL){
		fprintf(stderr, "Could not open file %s\n", filename);
	}else{
		fract_ifs ifs;
		fract_ifs_init(&ifs, &lcg, colors, num_colors);
		Bitmap *img = Bmp_empty(side, side, 24, 0);
		fract_ifs_flame(&ifs, &lcg, x0, y0, width, height, img, iters, gamma,
			sample);
		fract_ifs_destroy(&ifs);
		Bmp_save(img, file);
		Bmp_free(img);
	}
	if(colors)free(colors);
	if(filename)free(filename);
}

void
fract_save_track_record(int argc, char **argv)
{
	char *outname = NULL;
	Bitmap *img = NULL;
	fract_lcg lcg;
	fract_lcg_default(&lcg);
	fract_load_lcg(&lcg);
	fract_track track = {
		16, 8, 16, 1, NULL, NULL, NULL
	};
	int c;
	fract_igen generator = fract_sequence_random;
	static fract_igen generators[] = {fract_sequence_random,
		fract_sequence_linear, fract_sequence_hilbert};
	while((c = getopt(argc, argv, "o:i:n:m:u:t:g:")) != -1){
		switch(c){
			case 'o':
				outname = strdup(optarg); break;
			case 'i':{
				FILE *file = fopen(optarg, "rb");
				if(file == NULL){
					fprintf(stderr, "Could not open file %s\n", optarg);
				}else{
					img = Bmp_load(file);
					fclose(file);
					fract_load_bitmap(img);
				}
				break;
			}
			case 'n':
				track.notes_per_measure = strtol(optarg, NULL, 10); break;
			case 'm':
				track.num_measures = strtol(optarg, NULL, 10); break;
			case 'u':
				track.unique_measures = strtol(optarg, NULL, 10); break;
			case 't':
				track.num_tracks = strtol(optarg, NULL, 10); break;
			case 'g':
				generator = generators[strtol(optarg, NULL, 10)]; break;
		}
	}
	FILE *file = outname ? fopen(outname, "wb") : stdout;
	if(file == NULL){
		fprintf(stderr, "Could not open file %s\n", outname);
	}else{
		fract_track_generate(&track, generator);
		fract_save_track(file, &track);
		fclose(file);
		fract_track_destroy(&track);
	}
	if(outname)free(outname);
	if(img)Bmp_free(img);
}

void
fract_save_midi(int argc, char **argv)
{
	char *iname = NULL, *oname = NULL;
	fract_track track;
	int percussion = -1;
	double tempo = 8.0;
	uint8_t programs[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char lo = 40, hi = 80;
	int c;
	while((c = getopt(argc, argv, "i:o:d:p:b:h:l:")) != -1){
		switch(c){
			case 'i':
				iname = strdup(optarg); break;
			case 'o':
				oname = strdup(optarg); break;
			case 'd':
				percussion = strtol(optarg, NULL, 10); break;
			case 'b':
				tempo = strtod(optarg, NULL); break;
			case 'p':
				parse_bytes(optarg, programs, ','); break;
			case 'l':
				lo = strtol(optarg, NULL, 10); break;
			case 'h':
				hi = strtol(optarg, NULL, 10); break;
		}
	}
	FILE *ifile = iname ? fopen(iname, "rb") : stdin;
	if(ifile == NULL)
		fprintf(stderr, "Could not open file %s\n", iname);
	FILE *ofile = oname ? fopen(oname, "wb") : stdout;
	if(ofile == NULL)
		fprintf(stderr, "Could not open file %s\n", oname);
	char scale[] = {lo, 2, 2, 1, 2, 2, 2, 1, 0, hi};
	if(ofile && ifile){
		fract_load_track(ifile, &track);
		for(size_t i = 0; i < track.num_tracks; i++)
			track.programs[i] = programs[i];
		fract_write_midi_file(ofile, &track, tempo, 2, scale, 0, percussion);
		fract_track_destroy(&track);
	}
	if(ifile)fclose(ifile);
	if(ofile)fclose(ofile);
	if(iname)free(iname);
	if(oname)free(oname);
}

int main(int argc, char **argv){
	if(!strcmp(argv[1], "fractal")){
		fract_save_noise(argc - 1, argv + 1);
	}else if(!strcmp(argv[1], "swirl")){
		fract_save_swirl(argc - 1, argv + 1);
	}else if(!strcmp(argv[1], "flame")){
		fract_save_flame(argc - 1, argv + 1);
	}else if(!strcmp(argv[1], "track")){
		fract_save_track_record(argc - 1, argv + 1);
	}else if(!strcmp(argv[1], "midi")){
		fract_save_midi(argc - 1, argv + 1);
	}
}