/**
 * YAAKOV SCHECTMAN 2020
 * waves.c
 */

#include <math.h>
#include <stdlib.h>
#include "signal.h"
#include "constants.h"

float wavgen_sin(float phase, float *buffer, size_t buf_len, size_t buf_ptr)
{
	return sin(phase);
}

float wavgen_hsin(float phase, float *buffer, size_t buf_len, size_t buf_ptr)
{
	return (fmod(phase, M_PI * 2) < M_PI) ? sin(phase) : 0;
}

float wavgen_qsin(float phase, float *buffer, size_t buf_len, size_t buf_ptr)
{
	return (fmod(phase, M_PI * 2)  * 2 < M_PI) ? sin(phase) : 0;
}

float wavgen_rsin(float phase, float *buffer, size_t buf_len, size_t buf_ptr)
{
	return fabs(sin(phase));
}

float wavgen_saw(float phase, float *buffer, size_t buf_len, size_t buf_ptr)
{
	return fmod(phase, 2 * M_PI) / M_PI - 1;
}

float wavgen_sqr(float phase, float *buffer, size_t buf_len, size_t buf_ptr)
{
	return (fmod(phase, M_PI * 2)  < M_PI) ? 1 : -1;
}

float wavgen_tri(float phase, float *buffer, size_t buf_len, size_t buf_ptr)
{
	return ((fmod(phase, M_PI * 2)  < M_PI) ? fmod(phase, M_PI)
		: M_PI - fmod(phase, M_PI)) * 2 / M_PI - 1;
}

float wavgen_noise(float phase, float *buffer, size_t buf_len, size_t buf_ptr)
{
	return (float)rand() / RAND_MAX * 2 - 1;
}

float wavgen_ring(float phase, float *buffer, size_t buf_len, size_t buf_ptr)
{
	if(phase < 2 * M_PI)
		return (float)rand() / RAND_MAX * 2 - 1;
	int index = buf_ptr;
	return (buffer[index] + buffer[(index + buf_len - 1) % buf_len]) / 2;
}