/**
 * YAAKOV SCHECTMAN 2020
 * signal.c
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "music.h"
#include "signal.h"
#include "constants.h"
#include "utils.h"

enum{
	MAX_DRY = 16
};

static const float hertz_0 = 8.1758;

void
fract_iir_destroy(fract_iir *iir)
{
	if(iir->buffer)free(iir->buffer);
	if(iir->coefs)free(iir->coefs);
}

void
fract_iir_normalize(fract_iir *iir)
{
	switch(iir->type){
		case IIR_ZERO:
			for(size_t i = 0; i < iir->points; i++){
				float sum = fabs(iir->coefs[i * 3])
					+ fabs(iir->coefs[i * 3 + 1])
					+ fabs(iir->coefs[i * 3 + 2]);
				iir->coefs[i * 3] /= sum;
				iir->coefs[i * 3 + 1] /= sum;
				iir->coefs[i * 3 + 2] /= sum;
			}
			break;
		case IIR_POLE:
			for(size_t i = 0; i < iir->points; i++){
				if(iir->coefs[i * 3 + 2] != 0){
					float mag = sqrt(iir->coefs[i * 3 + 2] / iir->coefs[i * 3]);
					float cosarg = -.5 * iir->coefs[i * 3 + 1] / mag
						/ iir->coefs[i * 3];
					float arg = acos(cosarg);
					float norm = hypot(1 + cosarg * mag, sin(arg) * mag);
					norm *= norm;
					iir->coefs[i * 3] = norm;
				}else{
					iir->coefs[i * 3 + 1] /= iir->coefs[i * 3];
					iir->coefs[i * 3] = 1 + (iir->coefs[i * 3 + 1]);
				}
			}
	}
}

void
fract_iir_butterworth(
	datam_darr *iirs, size_t order,
	float *cutoff, size_t points, char type)
{
	float real, imag;
	for(size_t k = 0; k < (order + 1) / 2; k++){
		float angle = (2 * k + order + 1) * M_PI / (2 * order);
		fract_iir *iir = malloc(sizeof(fract_iir));
		iir->buffer = malloc(sizeof(float) * 3);
		iir->coefs = malloc(sizeof(float) * 3 * points);
		iir->points = points;
		iir->type = IIR_POLE;
		iir->stride = 1;
		iir->stride_dev = 0;
		iir->buf_len = 3;
		for(size_t i = 0; i < points; i++){
			float warped = tan(cutoff[i] / 2) * 2;
			printf("%f warps -> %f\n", cutoff[i], warped);
			float sigma = warped * cos(angle);
			float omega = warped * sin(angle);
			printf("S poles at %f +- %fi\n", sigma, omega);
			fract_blt(sigma, omega, &real, &imag);
			printf("Z poles at %f +- %fi\n", real, imag);
			if((order & 1) && k == order / 2){
				iir->coefs[i * 3] = 1;
				iir->coefs[i * 3 + 1] = -exp(sigma);
				iir->coefs[i * 3 + 2] = 0;
			}else{
				iir->coefs[i * 3] = 1;
				iir->coefs[i * 3 + 1] = -2 * real;
				iir->coefs[i * 3 + 2] = real * real + imag * imag;
			}
			if(type == IIR_ZERO)
				iir->coefs[i * 3 + 1] *= -1;
		}
		fract_iir_normalize(iir);
		printf("Coefs %f %f %f\n", iir->coefs[0], iir->coefs[1], iir->coefs[2]);
		datam_darr_push(iirs, iir);
	}
}

void
fract_iir_delayline(
	datam_darr *iirs, size_t delay_min, size_t delay_dev,
	float *gains, size_t points, char type,
	float *mix, size_t mix_pts)
{
	fract_iir *iir = malloc(sizeof(fract_iir));
	iir->points = points;
	iir->stride = delay_min;
	iir->stride_dev = delay_dev;
	iir->buf_len = (delay_min + delay_dev) + 1;
	iir->type = type;
	iir->buffer = malloc(sizeof(float) * iir->buf_len);
	iir->coefs = malloc(sizeof(float) * 3 * points);
	for(size_t i = 0; i < points; i++){
		iir->coefs[i * 3] = 1.0;
		iir->coefs[i * 3 + 1] = gains[i] * ((type == IIR_POLE) ? -1 : 1);
		iir->coefs[i * 3 + 2] = 0;
	}
	datam_darr_push(iirs, iir);
	if(mix_pts){
		iir = malloc(sizeof(fract_iir));
		iir->buffer = NULL;
		iir->buf_len = 0;
		iir->points = mix_pts;
		iir->type = IIR_MIX;
		iir->coefs = malloc(sizeof(float) * 3 * mix_pts);
		for(size_t i = 0; i < mix_pts; i++){
			iir->coefs[i * 3] = gains[i];
			iir->coefs[i * 3 + 1] = 1 - gains[i];
			iir->coefs[i * 3 + 2] = 0;
		}
		datam_darr_push(iirs, iir);
	}
}

void
fract_iir_reverb(
	datam_darr *iirs, size_t *delays, size_t delay_num,
	float *gains, float mix)
{
	for(size_t i = 0; i < delay_num; i++){
		fract_iir_delayline(iirs, delays[i], 1, gains + i, 1, IIR_ZERO,
			&mix, 1);
	}
}

void
fract_iir_ready(fract_iir *iir)
{
	memset(iir->buffer, 0, sizeof(float) * iir->buf_len);
	iir->buf_ptr = 0;
}

float
fract_iir_apply(fract_iir *iir, float sample, float dry, float lfo)
{
	size_t stride = (size_t)(iir->stride + iir->stride_dev * lfo);
	float coefs[3];
	lfo *= iir->points - 1;
	fflush(stdout);
	size_t lo = (int)(lfo);
	memcpy(coefs, iir->coefs + lo * 3, sizeof(float) * 3);
	float frac = lfo - lo;
	if(frac > 0 && iir->points > lo + 1){
		float next[3];
		memcpy(next, iir->coefs + lo * 3 + 3, sizeof(float) * 3);
		for(int i = 0; i < 3; i++)
			coefs[i] += frac * (next[i] - coefs[i]);
	}
	switch(iir->type){
		case IIR_ZERO:{
			iir->buffer[iir->buf_ptr] = sample;
			float out = sample * coefs[0] +
				iir->buffer[
				(iir->buf_ptr + iir->buf_len - stride) % iir->buf_len
				] * coefs[1] +
				iir->buffer[
				(iir->buf_ptr + 2 * iir->buf_len - stride * 2) % iir->buf_len
				] * coefs[2];
			iir->buf_ptr =(iir->buf_ptr + 1) % iir->buf_len;
			return out;
		}
		case IIR_POLE:{
			sample *= coefs[0];
			float out = sample -
				iir->buffer[
				(iir->buf_ptr + iir->buf_len - stride) % iir->buf_len
				] * coefs[1] -
				iir->buffer[
				(iir->buf_ptr + 2 * (iir->buf_len - stride)) % iir->buf_len
				] * coefs[2];
			iir->buffer[iir->buf_ptr] = out;
			iir->buf_ptr =(iir->buf_ptr + 1) % iir->buf_len;
			return out;
		}
		case IIR_MIX:
			return dry * coefs[0] + sample * coefs[1];
		default:
			return dry;
	}
}

void
fract_oscillator_init(fract_oscillator *synth)
{
	synth->generator = wavgen_sin;
	synth->post_proc = NULL;
	synth->filters = datam_darr_new(sizeof(fract_iir));
	synth->pm_strength = 0;
	synth->attack = 0;
	synth->decay = 0;
	synth->sustain = 1;
	synth->release = 0;
	synth->ring_buf = malloc(sizeof(float) * RINGBUF_SIZE);
	synth->ring_gain = .999;
	synth->ring_ptr = 0;
}

void
fract_oscillator_destroy(fract_oscillator *synth)
{
	free(synth->ring_buf);
	if(synth->filters != NULL){
		fract_iir *iirs = synth->filters->data;
		for(size_t i = 0; i < synth->filters->n; i++){
			fract_iir_destroy(iirs + i);
		}
		datam_darr_delete(synth->filters);
	}
}

void
fract_oscillator_ready(fract_oscillator *synth)
{
	synth->ring_ptr = 0;
	synth->phase = 0;
	fract_iir *filters = synth->filters->data;
	for(size_t i = 0; i < synth->filters->n; i++){
		fract_iir_ready(filters + i);
	}
	synth->filter_phase = 0;
}

void
fract_oscillator_synth(
	fract_oscillator *synth,
	int32_t *target, int32_t *limit,
	float omega, float sample_rate,
	float gain, size_t samples,
	float elapsed, char note_ends)
{
	float dry[MAX_DRY];
	float delta = 1.0 / sample_rate;
	size_t total_duration = samples + synth->release * note_ends * sample_rate;
	if(samples > limit - target){
		samples = limit - target;
		total_duration = samples;
	}
	fflush(stdout);
	float attack = (synth->attack == 0) ? 0 :
		1 / synth->attack * delta;
	float decay = (synth->decay == 0) ? 0 :
		(synth->sustain - 1) / synth->decay * delta;
	float release = (synth->release == 0) ? 0 :
		-synth->sustain / synth->release * delta;
	float envelope;
	if(elapsed < synth->attack)
		envelope = elapsed * attack * sample_rate;
	else if(elapsed < synth->attack + synth->decay)
		envelope = 1 + (elapsed - synth->attack) * decay * sample_rate;
	else envelope = synth->sustain;
	for(size_t index = 0; index < total_duration; index ++){
		float phase = synth->phase;
		synth->phase += omega;
		if(synth->pm_strength != 0){
			float pm_phase = phase * synth->pm_ratio;
			phase += synth->pm_strength * sin(pm_phase);
		}
		dry[0] = synth->generator(phase, synth->ring_buf, RINGBUF_SIZE,
			synth->ring_ptr);
		size_t dryptr = 0;
		synth->ring_buf[synth->ring_ptr] = dry[0] * synth->ring_gain;
		synth->ring_ptr = (synth->ring_ptr + 1) % (int)(2 * M_PI / omega);
		float wet = dry[0];
		fract_iir *filters = synth->filters->data;
		float lfo = sin(synth->filter_phase) * .5 + .5;
		synth->filter_phase += synth->filter_arg;
		for(size_t i = 0; i < synth->filters->n; i++){
			wet = fract_iir_apply(filters + i, wet, dry[dryptr], lfo);
			switch(filters[i].type){
				case IIR_PUSH:
					if(dryptr + 1 < MAX_DRY)
						dry[++dryptr] = wet; break;
				case IIR_POP:
					dryptr--; break;
				case IIR_EXTRACT:
					wet = dry[(size_t)(filters[i].coefs[0] + .4)]; break;
				case IIR_FLATTEN:{
					size_t layers = filters[i].coefs[0] + .4;
					float gain = 1.0 / layers;
					wet *= gain;
					for(size_t i = 1; i < layers && dryptr < 16U; i ++){
						wet += gain * dry[dryptr--];
					}
					break;
				}
			}
		}
		wet *= gain * envelope;
		float t = (float)(index) / sample_rate;
		if(elapsed + t < synth->attack)
			envelope += attack;
		else if(elapsed + t < synth->attack + synth->decay)
			envelope += decay;
		else if(index > samples){
			envelope += release;
		}
		int32_t samp = (int32_t)(wet * envelope * 32767);
		target[index] += samp;
		if(target + index >= limit)
			break;
	}
}

void
fract_signal_normalize(int32_t *signal, size_t len, int32_t gain, char amp)
{
	int32_t max = 0;
	for(size_t i = 0; i < len; i++){
		if(signal[i] > max)
			max = signal[i];
		else if(-signal[i] > max)
			max = -signal[i];
	}
	if(max <= gain && !amp)
		return;
	float ratio = (float)gain / max;
	for(size_t i = 0; i < len; i++){
		signal[i] = (signal[i] * ratio);
		if(signal[i] > gain)
			signal[i] = gain;
		else if(-signal[i] > gain)
			signal[i] = -gain;
	}
}

float
note_to_omega(int note, float sample_rate)
{
	return pow(2.0, note / 12.0) * hertz_0 * 2 * M_PI / sample_rate;
}

void
fract_signal_from_track(
	fract_track *track, fract_oscillator *instruments,
	double tempo, char *scale,
	double sample_rate, int32_t *samples, size_t samp_len)
{
	size_t scale_size;
	for(scale_size = 1; scale[scale_size]; scale_size++);
	int *last_notes = malloc(sizeof(int) * track->num_tracks);
	for(size_t k = 0; k < track->num_tracks; k++)
		last_notes[k] = -1;
	float *durations = calloc(track->num_tracks, sizeof(float));
	int32_t *end = samples + samp_len;
	size_t per_measure = track->notes_per_measure * sample_rate / tempo;
	size_t m_start = 0;
	size_t per_note = sample_rate / tempo;
	for(size_t m = 0; m < track->num_measures; m++){
		fflush(stdout);
		size_t measure = track->measures[m];
		for(size_t k = 0; k < track->num_tracks; k++){
			fflush(stdout);
			size_t offset = measure * track->notes_per_measure +
				k * track->notes_per_measure * track->unique_measures;
			size_t index = m_start;
			for(size_t n = 0; n < track->notes_per_measure; n++){
				int note = track->notes[offset + n];
				note = note ? fract_note_to_midi(note, scale, scale_size,
					scale[0], scale[scale_size + 1]) : -1;
				if(note >= 0){
					float omega = note_to_omega(note, sample_rate);
					char last_note = ((k + 1 < track->num_tracks)
						|| (n + 1 < track->notes_per_measure)) ?
						track->notes[offset + n] != track->notes[offset + n + 1]
						: 0;
					if(note != last_notes[k]){
						durations[k] = 0;
						fract_oscillator_ready(
							instruments + track->programs[k]);
					}
					size_t nind = m_start + (size_t)(n * sample_rate / tempo);
					fract_oscillator_synth(instruments + track->programs[k],
						samples + index,
						end,
						omega, sample_rate, 1.0, per_note,
						durations[k], last_note);
					durations[k] += per_note / sample_rate;
				}
				last_notes[k] = note;
				index += per_note;
			}
		}
		m_start += per_measure;
	}
}
