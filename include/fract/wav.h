/**
 * YAAKOV SCHECTMAN 2020
 * wav.h
 * Write wav files (maybe read in future, too)
 */

#ifndef _H_FRACT_WAV
#define _H_FRACT_WAV

#include <stdio.h>
#include <stdint.h>

/**
 * WAVE hader datatype
 *
 * sample_rate: samples per second per channel
 * channels: no. channels
 * bps: bytes per sample per channel (1 or 2)
 */
typedef struct _fract_wavhead{
	unsigned long sample_rate;
	char channels;
	char bps;
} fract_wavhead;

/**
 * Write header to file
 *
 * Data			Size in bytes
 * "fmt "		4
 * 16			4
 * 1			2
 * channels 	2
 * samplerate	4
 * samplerate * channels * bps
 *				4
 * channels * bps
 *				2
 * bps * 8		2
 */
void
fract_write_wavhead(FILE *file, fract_wavhead *header);

/**
 * Write wav file
 */
void
fract_write_wav(FILE *file, fract_wavhead *head, int32_t *samples, size_t len);

/**
 * Convert signed 16-bit to unsigned 8-bit
 */
void
fract_i32_unsign(int32_t *samps, size_t len);

#endif