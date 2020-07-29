#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "wav.h"
#include "signal.h"
#include "constants.h"

int sample_rate = 44100;
float duration = 1.0;
float frequency = 440;

float sawtooth(float phase){
	return (32767 * (fmod(phase, 2 * M_PI) / M_PI - 1));
}

float filter(float *coefs, float *buffer, size_t *buf_ptr, float input){
	float dc = 1 / (coefs[0] + coefs[1] + coefs[2]);
	input /= dc;
	float output = input - coefs[1] * buffer[(*buf_ptr + 3 - 1) % 3]
		- coefs[2] * buffer[(*buf_ptr + 3 - 2) % 3];
	output /= coefs[0];
	buffer[*buf_ptr] = output;
	(*buf_ptr)++;
	(*buf_ptr) %= 3;
	return output;
}

int main(){
	float buffer[3] = {0, 0, 0};
	size_t buf_ptr = 0;
	float pole_mag = 0.9;
	float pole_arg = 2000 * 2 * M_PI / sample_rate;
	float real = pole_mag * cos(pole_arg);
	float coefs[3] = {1, -2 * real, pole_mag * pole_mag};
	float phase = 0;
	size_t len = duration * sample_rate;
	int32_t *samples = malloc(sizeof(int32_t) * len);
	for(size_t i = 0; i < len; i++){
		float sample = sawtooth(phase);
		phase += frequency * 2 * M_PI / sample_rate;
		sample = filter(coefs, buffer, &buf_ptr, sample);
		samples[i] = (int32_t)sample;
	}
	
	// IO code to write to a WAV file
	fract_wavhead head = {sample_rate, 1, 2};
	fract_signal_normalize(samples, len, 32767, 0);
	FILE *file = fopen("test.wav", "wb");
	fract_write_wav(file, &head, samples, len);
	fclose(file);
	free(samples);
}