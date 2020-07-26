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

#define MAX_HILBERT_DEPTH (32)

static char hilbert_stack[MAX_HILBERT_DEPTH * 3];
static int hilbert_ptr = 0;
static char hilbert_dir = 0; // Positive X
static char x_dir[] = {1, 0, -1, 0};
static char y_dir[] = {0, 1, 0, -1};
static int hilbert_detail;

#define HILBERT_PUSH(depth, angle, step) do{\
	hilbert_stack[hilbert_ptr * 3] = (depth);\
	hilbert_stack[hilbert_ptr * 3 + 1] = (angle);\
	hilbert_stack[hilbert_ptr * 3 + 2] = (step);\
	hilbert_ptr++;}while(0)

#define HILBERT_POP(depth, angle, step) do{\
	hilbert_ptr--;\
	depth = hilbert_stack[hilbert_ptr * 3];\
	angle = hilbert_stack[hilbert_ptr * 3 + 1];\
	step = hilbert_stack[hilbert_ptr * 3 + 2];}while(0)

void
fract_load_bitmap(Bitmap *bmp)
{
	sampling_bitmap = bmp;
	bmp_x = bmp_y = 0;
	hilbert_ptr = 0;
	hilbert_dir = 0;
	hilbert_detail = 0;
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

void
rot(int side, int *x, int *y, int qx, int qy);
size_t
xy2d(int power, int x, int y);
void
d2xy(int power, size_t index, int *x, int *y);

uint32_t
fract_sequence_hilbert(int detail)
{
	if(sampling_bitmap == NULL)
		return 0;
	uint32_t ret = get_pixel(sampling_bitmap,
		bmp_x, bmp_y);
	int power = 63 - fract_nlz(sampling_bitmap->width);
	int incr = power - detail - 1;
	if(incr < 0)
		incr = 0;
	incr = 1 << incr;
	if(power >= 0){
		size_t index = xy2d(power, bmp_x, bmp_y);
		index += incr;
		if(index >= sampling_bitmap->width * sampling_bitmap->height)
			index %= sampling_bitmap->width * sampling_bitmap->height;
		d2xy(power, index, &bmp_x, &bmp_y);
	}else{
		if(detail != hilbert_detail){
			bmp_x = bmp_y = 0;
			hilbert_dir = 0;
			hilbert_ptr = 0;
			printf("Detail to %d\n", detail);
			hilbert_detail = detail;
		}
		if(detail == 2)printf("at %d %d\n", bmp_x, bmp_y);
		if(hilbert_ptr == 0){
			bmp_x = bmp_y = 0;
			hilbert_dir = 0;
			HILBERT_PUSH(power, 1, 0);
		}
		char depth, angle, step;
		while(hilbert_ptr > 0){
			HILBERT_POP(depth, angle, step);
			if(depth <= 0)
				continue;
			switch(step){
				case 0:
					hilbert_dir = (hilbert_dir + 4 + angle) & 3;
					HILBERT_PUSH(depth, angle, 1);
					HILBERT_PUSH(depth - 1, -angle, 0);
					break;
				case 1:
				case 3:
					HILBERT_PUSH(depth, angle, step + 1);
					bmp_x += x_dir[hilbert_dir] * incr;
					bmp_y += y_dir[hilbert_dir] * incr;
					goto afterloop;
				case 2:
					HILBERT_PUSH(depth, angle, 3);
					HILBERT_PUSH(depth - 1, angle, 0);
					hilbert_dir = (hilbert_dir + 4 - angle) & 3;
					break;
				case 4:
					HILBERT_PUSH(depth, angle, 5);
					HILBERT_PUSH(depth - 1, angle, 0);
					break;
				case 5:
					HILBERT_PUSH(depth, angle, 6);
					HILBERT_PUSH(depth - 1, -angle, 0);
					hilbert_dir = (hilbert_dir + 4 - angle) & 3;
					bmp_x += x_dir[hilbert_dir] * incr;
					bmp_y += y_dir[hilbert_dir] * incr;
					goto afterloop;
				case 6:
					hilbert_dir = (hilbert_dir + 4 + angle) & 3;
					break;
			}
		}
		afterloop:;
	}
	return ret;
}

void
rot(int side, int *x, int *y, int qx, int qy)
{
	if(!qy){
		if(qx){
			*x = side - 1 - *x;
			*y = side - 1 - *y;
		}
		int t = *x;
		*x = *y;
		*y = t;
	}
}

size_t
xy2d(int power, int x, int y)
{
	int qx, qy;
	size_t index = 0;
	for(int p = power - 1; p >= 0; p--){
		qx = (x >> p) & 1;
		qy = (y >> p) & 1;
		index += (1 << (p * 2)) * ((3 * qx) ^ qy);
		rot(1 << power, &x, &y, qx, qy);
	}
	return index;
}

void
d2xy(int power, size_t index, int *x, int *y)
{
	*x = *y = 0;
	int qx, qy;
	for(int p = 0; p < power; p++){
		qx = (index >> 1) & 1;
		qy = (index ^ qx) & 1;
		rot(1 << p, x, y, qx, qy);
		*x += qx << p;
		*y += qy << p;
		index >>= 2;
	}
}