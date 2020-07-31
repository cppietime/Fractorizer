/**
 * YAAKOV SCHECTMAN 2020
 * music.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <SLAV/slavio.h>
#include "prng.h"
#include "music.h"
#include "utils.h"
#include "constants.h"

enum {
	MUL_H = 5,
	MUL_S = 3,
	MUL_V = 2,
	SATURATION_LEVELS = 5,
	REST_CHANCE = 3,
	DRUM_CHANNEL = 9,
	CHANGE_CHANCE = 3
};

static char drums[] = {
	35, 36, 38, 41, 42, 46, 49, 0
};

void
fract_track_generate(fract_track *track, fract_igen generator)
{
	size_t num_notes = track->notes_per_measure * track->unique_measures
		* track -> num_tracks;
	track->notes = malloc(sizeof(uint8_t) * num_notes);
	track->measures = malloc(sizeof(size_t) * track->num_measures);
	float h, s, v;
	uint8_t lastnote = 0;
	for(size_t i = 0; i < num_notes; i++){
		uint8_t note = lastnote;
		if(i == 0 || generator(5) % CHANGE_CHANCE == 0){
			uint32_t sample = generator(4) & 0xffffff;
			RGB2HSV(sample, &h, &s, &v);
			note = 0;
			if(v > 0 && ((int)(s * SATURATION_LEVELS)) % REST_CHANCE != 0){
				note = (uint8_t)(h * 128 / M_PI);
			}
		}
		lastnote = note;
		track->notes[i] = note;
	}
	for(size_t i = 0; i < track->num_measures; i++){
		uint32_t sample = generator(2) & 0xffffff;
		RGB2HSV(sample, &h, &s, &v);
		track->measures[i] = ((size_t)(h * MUL_H + s * MUL_S + v * MUL_V))
			% track->unique_measures;
	}
	track->programs = calloc(track->num_tracks, sizeof(int));
}

void
fract_track_destroy(fract_track *track)
{
	free(track->notes);
	free(track->measures);
}

void
fract_track_print(fract_track *track, FILE *file)
{
	for(size_t i = 0; i < track->num_measures; i++){
		size_t measure = track->measures[i];
		for(size_t j = 0; j < track->notes_per_measure; j++){
			if(j)fprintf(file, " ");
			for(size_t k = 0; k < track->num_tracks; k++){
				if(k)fprintf(file, "&");
				size_t index = k * track->unique_measures
					* track->notes_per_measure +
					measure * track->notes_per_measure + j;
				fprintf(file, "%d", track->notes[index]);
			}
		}
		fprintf(file, "\n");
	}
}

void
fract_write_midi_header(
	FILE *file,
	int format, int tracks, int ticks_per_quarter_note)
{
	fputs("MThd", file);
	safe_write(6, 4, SAFE_BIG_ENDIAN, file);
	safe_write(format, 2, SAFE_BIG_ENDIAN, file);
	safe_write(tracks, 2, SAFE_BIG_ENDIAN, file);
	safe_write(ticks_per_quarter_note, 2, SAFE_BIG_ENDIAN, file);
}

datam_darr*
fract_create_midi_track(
	fract_track *track,
	double tempo,
	int ticks_per_quarter_note,
	char *scale,
	int percussion,
	size_t offset)
{
	datam_darr *ret = datam_darr_new(1);
	if(tempo != 0){
		datam_darr_pushlit(ret, 0);
		datam_darr_pushlit(ret, 0xff);
		datam_darr_pushlit(ret, 0x51);
		datam_darr_pushlit(ret, 0x03);
		int us_per_qn = ticks_per_quarter_note * 1000000 / tempo;
		datam_darr_pushlit(ret, (us_per_qn >> 16) & 0xff);
		datam_darr_pushlit(ret, (us_per_qn >> 8) & 0xff);
		datam_darr_pushlit(ret, us_per_qn & 0xff);
		datam_darr_pushlit(ret, 0);
		datam_darr_pushlit(ret, 0xff);
		datam_darr_pushlit(ret, 0x58);
		datam_darr_pushlit(ret, 0x04);
		datam_darr_pushlit(ret, 0x04);
		datam_darr_pushlit(ret, 0x04);
		datam_darr_pushlit(ret, 24);
		datam_darr_pushlit(ret, 8);
	}
	if(track != NULL){
		for(size_t i = offset; i < track->num_tracks && i < 16; i++){
			datam_darr_pushlit(ret, 0);
			datam_darr_pushlit(ret, 0xc0 | (i&15));
			datam_darr_pushlit(ret, track->programs[i]);
		}
		size_t scale_size;
		for(scale_size = 1; scale[scale_size] != 0; scale_size++);
		int max = scale[scale_size + 1];
		size_t tracks = track->num_tracks - offset;
		if(tracks > 16)
			tracks = 16;
		char *playing = calloc(tracks * 128, 1);
		char *newplay = calloc(tracks * 128, 1);
		int delta = 0;
		for(size_t i = 0; i < track->num_measures; i++){
			size_t measure = track->measures[i];
			for(size_t j = 0; j < track->notes_per_measure; j++){
				memset(newplay, 0, tracks * 128);
				for(size_t k = 0; k < tracks; k++){
					size_t index = (k + offset) * track->unique_measures
						* track->notes_per_measure +
						measure * track->notes_per_measure + j;
					int note = track->notes[index];
					if(note > 0){
						if(percussion >= 0 && (k + offset) == percussion){
							int drum;
							for(drum = 0; note > 0; note--){
								drum++;
								if(drums[drum] == 0)
									drum = 0;
							}
							note = drums[drum];
							newplay[note + k * 128] = 1;
						}else
							newplay[fract_note_to_midi(
								note - 1, scale, scale_size, scale[0], max)
								+ k * 128] = 1;
					}
				}
				for(size_t k = 0; k < tracks; k++){
					int channel = k;
					if(channel + offset == percussion && percussion >= 0){
						channel = DRUM_CHANNEL;
					}else if(((channel + offset) & 15) == (percussion & 15)
							&& percussion >= 0){
						if(channel > percussion)
							channel--;
						if(channel >= DRUM_CHANNEL)
							channel++;
					}
					for(size_t n = 0; n < 128; n++){
						if(playing[k * 128 + n] && !newplay[k * 128 + n]){
							// Turn note off
							fract_write_midi_var(ret, delta);
							delta = 0;
							datam_darr_pushlit(ret, 0x80 | (channel & 0xf));
							datam_darr_pushlit(ret, n);
							datam_darr_pushlit(ret, 40); //Default velocity
						}
						else if(newplay[k * 128 + n] && !playing[k * 128 + n]){
							// Turn note on
							fract_write_midi_var(ret, delta);
							delta = 0;
							datam_darr_pushlit(ret, 0x90 | (channel & 0xf));
							datam_darr_pushlit(ret, n);
							datam_darr_pushlit(ret, 63);
						}
						playing[k * 128 + n] = newplay[k * 128 + n];
					}
				}
				delta++;
			}
		}
	}
	datam_darr_pushlit(ret, track->notes_per_measure & 0x7f);
	datam_darr_pushlit(ret, 0xff);
	datam_darr_pushlit(ret, 0x2f);
	datam_darr_pushlit(ret, 0);
	return ret;
}

int
fract_note_to_midi(int note, char *scale, size_t scale_size, int zero, int max)
{
	int ret = zero;
	for(int i = 0; i < note; i++){
		ret += scale[i % (scale_size - 1) + 1];
		if(ret > max){
			ret = zero;
			i += (scale_size - 1) - (i % (scale_size - 1)) - 1;
		}
	}
	return ret & 0x7f;
}

void
fract_write_midi_var(datam_darr *list, uint64_t val){
	int bits = 64 - fract_nlz(val);
	int chunks = (bits + 6) / 7;
	uint8_t masked[10];
	uint8_t mask = 0x7f;
	for(size_t i = 0; i < 10; i++){
		masked[i] = val & mask;
		val >>= 7;
	}
	for(int i = chunks - 1; i > 0; i--){
		datam_darr_pushlit(list, 0x80 & masked[i]);
	}
	datam_darr_pushlit(list, masked[0]);
}

void
fract_write_midi_track(FILE *file, datam_darr *msgs)
{
	fputs("MTrk", file);
	safe_write(msgs->n, 4, SAFE_BIG_ENDIAN, file);
	fwrite(msgs->data, 1, msgs->n, file);
}

void
fract_write_midi_file(
	FILE *file, fract_track *track,
	double tempo, int ticks_per_quarter_note,
	char *scale, int format,
	int percussion)
{
	fflush(stdout);
	if((track->num_tracks > 16 && format == 0) || format == 1)
		format = 2;
	fract_write_midi_header(file, format,
		(track->num_tracks + 15) >> 4 + (format & 1),
		ticks_per_quarter_note);
	datam_darr *msgs;
	switch(format){
		case 0:
			msgs = fract_create_midi_track(track, tempo,
				ticks_per_quarter_note, scale, percussion, 0);
			fract_write_midi_track(file, msgs);
			datam_darr_delete(msgs);
			break;
		case 1:
			msgs = fract_create_midi_track(NULL, tempo,
				ticks_per_quarter_note, scale, percussion, 0);
			fract_write_midi_track(file, msgs);
			datam_darr_delete(msgs);
			for(size_t offset = 0; offset < track->num_tracks; offset += 16){
				msgs = fract_create_midi_track(track, 0.0,
					ticks_per_quarter_note, scale, percussion, offset);
				fract_write_midi_track(file, msgs);
				datam_darr_delete(msgs);
			}
			break;
		case 2:
			for(size_t offset = 0; offset < track->num_tracks; offset += 16){
				datam_darr *msgs = fract_create_midi_track(track, 0.0,
					ticks_per_quarter_note, scale, percussion, offset);
				fract_write_midi_track(file, msgs);
				datam_darr_delete(msgs);
			}
			break;
	}
}

void
fract_save_track(FILE *file, fract_track *track){
	safe_write(track->notes_per_measure, 4, SAFE_LITTLE_ENDIAN, file);
	safe_write(track->unique_measures, 4, SAFE_LITTLE_ENDIAN, file);
	safe_write(track->num_measures, 4, SAFE_LITTLE_ENDIAN, file);
	safe_write(track->num_tracks, 4, SAFE_LITTLE_ENDIAN, file);
	for(size_t i = 0; i < track->num_measures; i++){
		safe_write(track->measures[i], 4, SAFE_LITTLE_ENDIAN, file);
	}
	fwrite(track->notes, 1, track->unique_measures * track->num_tracks
			* track->notes_per_measure, file);
}

void
fract_load_track(FILE *file, fract_track *track){
	track->notes_per_measure = safe_read(4, SAFE_LITTLE_ENDIAN, file);
	track->unique_measures = safe_read(4, SAFE_LITTLE_ENDIAN, file);
	track->num_measures = safe_read(4, SAFE_LITTLE_ENDIAN, file);
	track->num_tracks = safe_read(4, SAFE_LITTLE_ENDIAN, file);
	track->measures = malloc(sizeof(size_t) * track->num_measures);
	for(size_t i = 0; i < track->num_measures; i++){
		track->measures[i] = safe_read(4, SAFE_LITTLE_ENDIAN, file);
	}
	size_t num_notes = track->unique_measures *	track->notes_per_measure
		* track->num_tracks;
	track->notes = malloc(num_notes);
	fread(track->notes, 1, track->unique_measures * track->num_tracks
			* track->notes_per_measure, file);
	track->programs = malloc(sizeof(int) * track->num_tracks);
}