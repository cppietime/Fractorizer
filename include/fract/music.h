/**
 * YAAKOV SCHECTMAN 2020
 * music.h
 * Datatypes and functions for symbolic music generation and manipulation
 */

#ifndef _H_FRACT_MUSIC
#define _H_FRACT_MUSIC

#include <stdio.h>
#include <stdint.h>
#include <SLAV/datam.h>
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
 * programs: instrument per track
 */
typedef struct _fract_track {
	uint8_t notes_per_measure;
	size_t unique_measures;
	size_t num_measures;
	size_t num_tracks;
	uint8_t *notes;
	size_t *measures;
	int *programs;
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

/**
 * Print track to file in human-readable format
 */
void
fract_track_print(fract_track *track, FILE *file);

/**
 * Write a MIDI header chunk to file
 * 'MThd' 6 (4 bytes) format (2 bytes) tracks (2 bytes) ticks (2 bytes)
 *
 * format: 0, 1, or 2
 *     0: single-track
 *     1: synchronous multi-track
 *     2: asynchronous multi_track
 */
void
fract_write_midi_header(
	FILE *file,
	int format, int num_tracks, int ticks_per_quarter_note
);

/**
 * Return a list of bytes for messages that make up a MIDI track
 *
 * tempo: 0 to ignore, otherwise, note ticks per second
 * scale: array of form:
 *     min_note, intervals..., 0, max_note
 */
datam_darr*
fract_create_midi_track(
	fract_track *track,
	double tempo,
	int ticks_per_quarter_note,
	char *scale,
	int percussion
);

/**
 * Convert a scale-indexed note to MIDI
 */
int
fract_note_to_midi(int note, char *scale, size_t scale_size, int zero, int max);

/**
 * Write a value in the variable-length format MIDI expects
 * Split the value into 7-bit groups, with the MSb of the last being 0,
 * and of all preceeding it being 1
 */
void
fract_write_midi_var(datam_darr *list, uint64_t val);

/**
 * Write a MIDI track chunk
 */
void
fract_write_midi_track(FILE *file, datam_darr *msg_bytes);

/**
 * Write to MIDI file, currently just format 0
 *
 * percussion: track for percussion
 */
void
fract_write_midi_file(
	FILE *file, fract_track *track,
	double tempo, int ticks_per_quarter_note,
	char *scale, int format,
	int percussion;
);

/**
 * Save a track to a file
 */
void
fract_save_track(FILE *file, fract_track *track);

/**
 * Load track from file; allocates memory
 */
void
fract_load_track(FILE *file, fract_track *track);

#endif