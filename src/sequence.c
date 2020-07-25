/**
 * YAAKOV SCHECTMAN 2020
 * sequence.c
 */

#include <stdint.h>
#include <SLAV/slavio.h>
#include "prng.h"
#include "utils.h"

fract_lcg *sampling_lcg = NULL;
Bitmap *sampling_bitmap = NULL;
static int bmp_x = 0;
static int bmp_y = 0;

void
fract_load_bitmap(Bitmap *bmp)
{
	sampling_bitmap = bmp;
	bmp_x = bmp_y = 0;
}

void
fract_load_lcg(fract_lcg *lcg)
{
	sampling_lcg = lcg;
}

uint32_t
fract_sequence_random(int detail)
{
	if(sampling_lcg == NULL)
		return 0;
	return fract_lcg_next(sampling_lcg);
}

uint32_t
fract_sequence_linear(int detail)
{
	if(sampling_bitmap == NULL)
		return 0;
	uint32_t ret = get_pixel(sampling_bitmap,
		bmp_x, bmp_y);
	int incr = 63 - fract_nlz(sampling_bitmap->width) - detail - 1;
	if(incr < 0)
		incr = 0;
	incr = 1 << incr;
	bmp_x += incr;
	if(bmp_x >= sampling_bitmap->width){
		bmp_y += (bmp_x / sampling_bitmap->width) * incr;
		bmp_x %= sampling_bitmap->width;
		bmp_y %= sampling_bitmap->height;
	}
	return ret;
}