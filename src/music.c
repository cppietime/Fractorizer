/**
 * YAAKOV SCHECTMAN 2020
 * music.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <SLAV/slavio.h>
#include "prng.h"
#include "music.h"

static const float threshold = 0.5f;

enum {
	MUL_H = 5,
	MUL_S = 3,
	MUL_V = 2
};

void
fract_track_generate(fract_track *track, fract_igen generator)
{
	size_t num_notes = track->notes_per_measure * track->unique_measures
		* track -> num_tracks;
	track->notes = malloc(sizeof(uint8_t) * num_notes);
	track->measures = malloc(sizeof(size_t) * track->num_measures);
	float h, s, v;
	for(size_t i = 0; i < num_notes; i++){
		uint32_t sample = generator(4);
		RGB2HSV(sample, &h, &s, &v);
		uint8_t note = 0;
		if(v > 0 && s >= threshold){
			note = (uint8_t)(h * 128 / M_PI);
		}
		track->notes[i] = note;
	}
	for(size_t i = 0; i < track->num_measures; i++){
		uint32_t sample = generator(1);
		RGB2HSV(sample, &h, &s, &v);
		track->measures[i] = ((size_t)(h * MUL_H + s * MUL_S + v * MUL_V))
			% track->unique_measures;
	}
}

void
fract_track_destroy(fract_track *track)
{
	free(track->notes);
	free(track->measures);
}
