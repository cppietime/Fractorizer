/**
 * YAAKOV SCHECTMAN 2020
 * parser.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <SLAV/datam.h>
#include "signal.h"
#include "utils.h"

static wavgen_func generators[] = {
	wavgen_sin,
	wavgen_hsin,
	wavgen_qsin,
	wavgen_rsin,
	wavgen_saw,
	wavgen_sqr,
	wavgen_tri,
	wavgen_noise,
	wavgen_ring
};

fract_oscillator
fract_oscillator_from_file(FILE *file)
{
	static char line[1024];
	char *ptr;
	fract_oscillator osc;
	fract_oscillator_init(&osc);
	while(fgets(line, 1024, file) != NULL){
		if(*line == '#')
			continue;
		if(!strncmp(line, "BRK", 3))
			break;
		if(!strncmp(line, "GEN", 3)){
			osc.generator = generators[strtol(line + 3, NULL, 10)];
		}
		else if(!strncmp(line, "PM", 2)){
			osc.pm_ratio = strtod(line + 2, &ptr);
			osc.pm_strength = strtod(ptr + 1, NULL);
		}
		else if(!strncmp(line, "BW", 2)){
			int order = strtol(line + 2, &ptr, 10);
			float cutoff = strtod(ptr + 1, &ptr);
			char type = strtol(ptr + 1, NULL, 10);
			fract_iir_butterworth(osc.filters, order, &cutoff, 1, type);
		}
		else if(!strncmp(line, "EXEC", 4)){
			fract_iir filter;
			filter.coefs = malloc(sizeof(float));
			filter.buffer = NULL;
			filter.buf_len = 0;
			filter.type = strtol(line + 4, &ptr, 10);
			filter.coefs[0] = strtod(ptr + 1, NULL);
			datam_darr_push(osc.filters, &filter);
		}
		else if(!strncmp(line, "DEL", 3)){
			size_t del_min = strtol(line + 3, &ptr, 10);
			size_t del_dev = strtol(ptr + 1, &ptr, 10);
			float gain = strtod(ptr + 1, &ptr);
			float mix = strtod(ptr + 1, &ptr);
			char type = strtol(ptr + 1, NULL, 10);
			fract_iir_delayline(osc.filters, del_min, del_dev, &gain, 1, type,
				&mix, 1);
		}
		else if(!strncmp(line, "RG", 2)){
			float gain = strtod(line + 2, NULL);
			osc.ring_gain = gain;
		}
		else if(!strncmp(line, "ENV", 3)){
			osc.attack = strtod(line + 3, &ptr);
			osc.decay = strtod(ptr + 1, &ptr);
			osc.sustain = strtod(ptr + 1, &ptr);
			osc.release = strtod(ptr + 1, NULL);
		}
		else if(!strncmp(line, "CP", 2)){
			float arg = strtod(line + 2, &ptr);
			float mag = strtod(ptr + 1, &ptr);
			float real, imag;
			real = mag * cos(arg);
			imag = mag * sin(arg);
			char type = strtol(ptr + 1, NULL, 10);
			fract_iir filter = {
				malloc(sizeof(float) * 3),
				malloc(sizeof(float) * 3),
				1, 1, 0, 3, 0, type
			};
			filter.coefs[0] = 1;
			filter.coefs[1] = -2 * real;
			filter.coefs[2] = real * real + imag * imag;
			fract_iir_normalize(&filter);
			printf("Pair of %f %f %f\n", filter.coefs[0], filter.coefs[1], filter.coefs[2]);
			datam_darr_push(osc.filters, &filter);
		}
		else if(!strncmp(line, "MIX", 3)){
			fract_iir filter = {
				malloc(sizeof(float) * 3),
				NULL,
				1, 0, 0, 0, 0, IIR_MIX
			};
			filter.coefs[0] = strtod(line + 3, &ptr);
			filter.coefs[1] = strtod(ptr + 1, NULL);
			datam_darr_push(osc.filters, &filter);
		}
	}
	return osc;
}