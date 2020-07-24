#include <stdio.h>
#include <stdint.h>
#include <SLAV/slavio.h>
#include "prng.h"

int main(){
	fract_lcg lcg;
	fract_lcg_default(&lcg);
	
	fract_perlin perlin;
	fract_perlin_init(&perlin, &lcg, 256);
	
	Bitmap *img = Bmp_empty(256, 256, 24, 0);
	img->R = img->G = img->B = 0;
	for(int x = 0; x < 256; x++){
		for(int y = 0; y < 256; y++){
			double v = fract_perlin_at(&perlin, x / 29.0, y / 29.0);
			int gray = (v + 0.707) * 256 * 0.707;
			uint32_t color = (gray << 16) | (gray << 8) | gray;
			set_pixel(img, x, y, color);
		}
	}
	
	FILE *file = fopen("image.bmp", "wb");
	Bmp_save(img, file);
	fclose(file);
	Bmp_free(img);
	fract_perlin_destroy(&perlin);
	
	return 0;
}