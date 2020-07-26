/**
 * YAAKOV SCHECTMAN 2020
 * utils.c
 */

#include <stdint.h>
#include <stdlib.h>
#include <SLAV/slavio.h>
#include "utils.h"

int
fract_nlz(uint64_t x)
{
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

void
fract_box_blur(Bitmap *img, int w, int h)
{
	int box = (2 * w + 1) * (2 * h + 1);
	float *buffer = calloc(img->width * img->height * 3, sizeof(float));
	for(int y = 0; y < img->height; y++){
		for(int x = 0 ; x < img->width; x++){
			uint32_t cell = get_pixel(img, x, y);
			float r = (cell >> 16) & 0xff;
			float g = (cell >> 8) & 0xff;
			float b = cell & 0xff;
			for(int dy = -h; dy <= h; dy++){
				if(y + dy < 0 || y + dy >= img->height)
					continue;
				for(int dx = -w; dx <= w; dx++){
					if(x + dx < 0 || x + dx >= img->width)
						continue;
					size_t index = (y + dy) * img->width + x + dx;
					buffer[index * 3] += r / box;
					buffer[index * 3 + 1] += g / box;
					buffer[index * 3 + 2] += b / box;
				}
			}
		}
	}
	for(int y = 0; y < img->height; y++){
		for(int x = 0 ; x < img->width; x++){
			size_t index = y * img->width + x;
			int r = buffer[index * 3];
			int g = buffer[index * 3 + 1];
			int b = buffer[index * 3 + 2];
			uint32_t cell = ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
			set_pixel(img, x, y, cell);
		}
	}
	free(buffer);
}