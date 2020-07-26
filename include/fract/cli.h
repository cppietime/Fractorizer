/**
 * YAAKOV SCHECTMAN
 * cli.h
 * Command line usage functions
 */

#ifndef _H_FRACT_CLI
#define _H_FRACT_CLI

/**
 * CLI generate/save fractal noise image ("fractal")
 *
 * -l: no. layers
 * -o: output filename
 * -c: colors as r,g,b,r,g,b...
 * -r: noise resolutions ,-separated
 * -x: offsets as x,y,x,y...
 * -f: flags ,-separated
 * -s: image width/height (image always square)
 */
void
fract_save_noise(int argc, char **argv);

/**
 * CLI generate/save swirl image ("swirl")
 *
 * -o: output filename
 * -n: noise resolution
 * -r: swirl resolution
 * -w: swirl strength
 * -x: x offset
 * -y: y offset
 * -b: blur radius (box)
 * -l: no. colors
 * -c: colors as r,g,b,r,g,b...
 */
void
fract_save_swirl(int argc, char **argv);

/**
 * CLI generate/save fractal flame ("flame")
 *
 * -o: output filename
 * -l: palette size
 * -c: colors as r,b,g,r,g,b,...
 * -x, -y: top-left coords
 * -X, -Y: width/height
 * -i: iterations
 * -g: gamma
 * -k: super-sampling
 * -s: image size
 */
void
fract_save_flame(int argc, char **argv);

/**
 * CLI create/save track ("track")
 *
 * -i: input image
 * -o: output name
 * -n: notes per measure
 * -m: length in measures
 * -u: unique measures
 * -t: tracks
 * -g: generator (0 - random, 1 - linear, 2 - hilbert)
 */
void
fract_save_track_record(int argc, char **argv);

/**
 * CLI create/save midi ("midi")
 *
 * -i: input track
 * -o: output filename
 * -d: percussion track
 * -p: programs ,-separated
 * -b: tempo in notes-per-second
 * -l, -h: min/max MIDI notes
 */
void
fract_save_midi(int argc, char **argv);

#endif