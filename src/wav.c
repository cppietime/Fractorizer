/**
 * YAAKOV SCHECTMAN 2020
 * wav.c
 */

#include <stdio.h>
#include <stdint.h>
#include <SLAV/slavio.h>
#include "wav.h"

void
fract_write_wavhead(FILE *file, fract_wavhead *header)
{
	fputs("fmt ", file);
	safe_write(16, 4, SAFE_LITTLE_ENDIAN, file);
	safe_write(1, 2, SAFE_LITTLE_ENDIAN, file);
	safe_write(header->channels, 2, SAFE_LITTLE_ENDIAN, file);
	safe_write(header->sample_rate, 4, SAFE_LITTLE_ENDIAN, file);
	uint32_t bytes = header->sample_rate * header->channels * header->bps;
	safe_write(bytes, 4, SAFE_LITTLE_ENDIAN, file);
	uint16_t block = header->channels * header->bps;
	safe_write(block, 2, SAFE_LITTLE_ENDIAN, file);
	safe_write(header->bps * 8, 2, SAFE_LITTLE_ENDIAN, file);
}

void
fract_write_wav(FILE *file, fract_wavhead *head, int32_t *samples, size_t len)
{
	fputs("RIFF", file);
	safe_write(len * head->bps + 36, 4, SAFE_LITTLE_ENDIAN, file);
	fputs("WAVE", file);
	fract_write_wavhead(file, head);
	fputs("data", file);
	safe_write(len * head->bps, 4, SAFE_LITTLE_ENDIAN, file);
	for(size_t i = 0; i < len; i++){
		int32_t samp = samples[i];
		safe_write(samp, head->bps, SAFE_LITTLE_ENDIAN, file);
	}
}

void
fract_i32_unsign(int32_t *samps, size_t len)
{
	for(size_t i = 0; i < len; i++){
		int32_t samp = samps[i];
		samp = (int)(samp / 256 + 128);
		if(samp > 255)samp = 255;
		else if(samp < 0)samp = 0;
		samps[i] = samp;
	}
}