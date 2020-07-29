/**
 * YAAKOV SCHECTMAN 2020
 * signal.h
 * Datatypes and functions for digital signal generation/processing
 */

#ifndef _H_FRACT_SIGNAL
#define _H_FRACT_SIGNAL

#include <stdio.h>
#include <stdint.h>
#include <SLAV/datam.h>
#include "music.h"

#define RINGBUF_SIZE 44100

/**
 * A function that generates wav samples
 */
typedef float (*wavgen_func)(float, float*, size_t, size_t);

float wavgen_sin(float phase, float *buffer, size_t buf_len, size_t buf_ptr);
float wavgen_hsin(float phase, float *buffer, size_t buf_len, size_t buf_ptr);
float wavgen_qsin(float phase, float *buffer, size_t buf_len, size_t buf_ptr);
float wavgen_rsin(float phase, float *buffer, size_t buf_len, size_t buf_ptr);
float wavgen_saw(float phase, float *buffer, size_t buf_len, size_t buf_ptr);
float wavgen_sqr(float phase, float *buffer, size_t buf_len, size_t buf_ptr);
float wavgen_tri(float phase, float *buffer, size_t buf_len, size_t buf_ptr);
float wavgen_noise(float phase, float *buffer, size_t buf_len, size_t buf_ptr);
float wavgen_ring(float phase, float *buffer, size_t buf_len, size_t buf_ptr);

/**
 * 1:1 float transformation
 */
typedef float (*float_func)(float);

enum {
	IIR_ZERO = 0,
	IIR_POLE = 1,
	IIR_MIX = 2,
	IIR_POP = 3,
	IIR_PUSH = 4,
	IIR_EXTRACT = 5,
	IIR_FLATTEN = 6
};

/**
 * Infinite Impulse Response fitler
 * Represented individually using order-2 real-valued filters
 *
 * coefs: 3 coefficients per time-point
 * buffer: buffer of samples used
 * buf_ptr: pointer into buffer
 * stride: how far back each sample
 */
typedef struct _fract_iir{
	float *coefs;
	float *buffer;
	size_t points;
	size_t stride;
	size_t stride_dev;
	size_t buf_len;
	size_t buf_ptr;
	char type;
} fract_iir;

/**
 * Free memory allocated by iir
 */
void
fract_iir_destroy(fract_iir *iir);

/**
 * Normalize IIR coefs
 */
void
fract_iir_normalize(fract_iir *iir);

/**
 * Generates a new Butterworth filter
 * Allocates memory for each filter generated
 *
 * order: number of poles/zeros
 * cutoffs: cutoff angular frequencies (should be in [0, pi])
 * points: size of cutoffs
 * type: IIR_ZERO or IIR_POLE
 */
void
fract_iir_butterworth(
	datam_darr *iirs, size_t order,
	float *cutoff, size_t points, char type
);

/**
 * Generates a new comb filter
 *
 * gains: array of gain coefficients for comb filters
 * points: size of gains
 * type: IIR_ZERO for feedforward, IIR_POLE for feedback
 * mix: dry/wet mix coefficients (just give dry)
 * mix_pts: size of mix (zero to not mix)
 */
void
fract_iir_delayline(
	datam_darr *iirs, size_t delay_min, size_t delay_dev,
	float *gains, size_t points, char type,
	float *mix, size_t mix_pts
);

/**
 * Reverb of many delay lines
 */
void
fract_iir_reverb(
	datam_darr *iirs, size_t *delays, size_t delay_num,
	float *gains, float mix
);

/**
 * Set the buffer and buf_ptr to 0
 */
void
fract_iir_ready(fract_iir *iir);

/**
 * Pass a sample throgh, process, and return the output
 *
 * dry: pre-processing dry value for use in effects
 * lfo: normalized to [0, 1]
 */
float
fract_iir_apply(fract_iir *iir, float sample, float dry, float lfo);

/**
 * An oscillator datatype
 *
 * generator: function to derive samples
 * post_proc: function to post-process output samples
 * filters: list of IIR filters to apply to instrument output
 * filter_arg: used for time-varying filters
 * ring_buf: used for ring-buffer synthesis (strings)
 * ring_gain: gain in ring-buffer synthesis
 * pm_ratio: ratio of PM frequency to note frequency
 * pm_strength: amplitude of PM
 * phase: current phase in [0, 2PI]
 */
typedef struct _fract_oscillator{
	wavgen_func generator;
	float_func post_proc;
	datam_darr *filters;
	float *ring_buf;
	float ring_gain;
	size_t ring_ptr;
	float filter_phase;
	float filter_arg;
	float pm_ratio;
	float pm_strength;
	float phase;
	float attack;
	float decay;
	float sustain;
	float release;
} fract_oscillator;

/**
 * Generate a new empty oscillator
 */
void
fract_oscillator_init(fract_oscillator *synth);

/**
 * Frees memory allocated by synth
 */
void
fract_oscillator_destroy(fract_oscillator *synth);

/**
 * Prepare synth for new note
 */
void
fract_oscillator_ready(fract_oscillator *synth);

/**
 * Synthesize a note
 *
 * target: pointer to destination (start of note)
 * limit: pointer to end of samples (used by release)
 * omega: angular frequency of note in [0, pi], pi being nyquist
 * duration: length of note in seconds
 * elapsed: seconds of this note already played
 * note_ends: 1 if release should be written after note, 0 if more note to come
 */
void
fract_oscillator_synth(
	fract_oscillator *synth,
	int32_t *target, int32_t *limit,
	float omega, float sample_rate,
	float gain, size_t duration,
	float elapsed, char note_ends
);

/**
 * Normalize signal to be at most gain
 *
 * amp: if 1, increase to match gain, otherwise, only decrease if needed
 */
void
fract_signal_normalize(int32_t *signal, size_t len, int32_t gain, char amp);

/**
 * Play a fract_track to samples, each in [-32767, 32767]
 */
void
fract_signal_from_track(
	fract_track *track, fract_oscillator *instruments,
	double tempo, char *scale,
	double sample_rate, int32_t *samples, size_t samp_len
);

/**
 * Return a new instrument read from a file
 */
fract_oscillator
fract_oscillator_from_file(FILE *file);

#endif