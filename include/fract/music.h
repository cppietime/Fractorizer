/**
 * YAAKOV SCHECTMAN 2020
 * music.h
 * Datatypes and functions for symbolic music generation and manipulation
 */

#ifndef _H_FRACT_MUSIC
#define _H_FRACT_MUSIC

#include <stdio.h>
#include <stdint.h>
#include "prng.h"

/**
 * Datatype that holds a generated track of music
 * Composed of an number of "measures" of notes in time-order
 *
 * notes_per_measure: number of discrete note points per measure
 * unique_measures: number of unique measures
 * num_measures: length in measures
 * num_tracks: number of tracks/voices
 * notes: array of size num_tracks * notes_per_measure * unique_measures
 * measures: measure indices
 */
typedef struct _fract_track {
	uint8_t notes_per_measure;
	size_t unique_measures;
	size_t num_measures;
	size_t num_tracks;
	uint8_t *notes;
	size_t *measures;
} fract_track;

/**
 * Generates notes and measures for the fract_track provided
 *
 * base: specifies parameters by its fields
 * generator: value generator for PRNG in generation
 */
void
fract_track_generate(fract_track *track, fract_igen generator);

/**
 * Free memory allocated for a track
 */
void
fract_track_destroy(fract_track *track);

#endif