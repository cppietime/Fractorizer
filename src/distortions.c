/**
 * YAAKOV SCHECTMAN 2020
 * distortions.c
 */

#include "signal.h"

float
distortion_hard(float samp, float arg)
{
	samp *= arg;
	if(samp > 1)
		samp = 1;
	if(samp < -1)
		samp = -1;
	return samp;
}