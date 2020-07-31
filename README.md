# Fractorizer
### Yaakov Schectman 2020

Generate fractal images and MIDI song files with specialized PRNG.

This requires the slavio and datam libraries of the SLAV_parser project: https://github.com/cppietime/SLAV_parser

To make the command line interface, run `make cli`.

Once compiled, run `./cli <command> [OPTIONS]`.
command is one of:
 - swirl
 - fractal
 - flame
 - track
 - midi

### swirl
`cli swirl` generates a BMP using Perlin Noise and displacement noise, producing a swirled appearance.
#### Options:
 - -o: output file name (default to stdout)
 - -n: resolution of Perlin noise
 - -r: resolution of displacement noise
 - -w: strength of displacement
 - -x: displacement noise x offset
 - -y: displacement noise y offset
 - -s: image side length (always square)
 - -b: box blur radius
 - -l: number of colors in palette
 - -c: colors, specifies as r,g,b,r,g,b..., each in [0, 255]

### fractal
`cli fractal` generates a BMP using multiple layers of Perlin Noise.
#### Options:
 - -o: output file name (default to stdout)
 - -l: number of colors in palette, also number of noise layers
 - -c: colors, specifies as r,g,b,r,g,b..., each in [0, 255]
 - -r: comma-separated list of noise resolutions
 - -x: comma-separated list of offsets: x,y,x,y...
 - -f: comma-separated list of flags (1 to take `abs` of noise, else 0)
 - -s: image side length (always square)

### flame
`cli flame` generates a BMP using an IFS fractal flame.
#### Options:
 - -o: output file name (default to stdout)
 - -i: number of iterations
 - -l: number of unique colors to choose from
 - -c: colors, sepcified as r,g,b,r,g,b... each in [0, 255]
 - -x: x coordinate of left of image
 - -y: y coordinate of top of image
 - -X: width of image in XY space
 - -Y: height of image in XY space
 - -s: side length of image (always square)
 - -g: gamma
 - -k: super-sample size

### track
`cli track` generates a music track either randomly or from an image
#### Options:
 - -o: output file name (default stdout)
 - -i: input BMP file name
 - -g: generator (0 - random, 1 - linear, 2 - Hilbert Curve)
 - -t: number of tracks/voices
 - -m: length in measures
 - -n: notes per measure
 - -u: number of unique measures to choose from

### midi
`cli track` renders a track to MIDI
#### Options:
 - -o: output file name(default stdout)
 - -i: input file name(default stdin)
 - -b: tempo (recommended 4.0 to 8.0)
 - -p: instrument id's, comma separated-list, assigned to voices in-order
 - -d: percussion voice number, or -1 for no percussion
 - -l: minimum MIDI note
 - -h: maximum MIDI note

### wav
#### Options:
`cli wav` renders a WAV from a track
 - -o: output file name(default stdout)
 - -i: input file name(default stdin)
 - -p: instruments file name(leave out to use default)
 - -b: tempo (notes per second)
 - -r: sample rate
 - -l: minimum MIDI-style note
 - -h: maximum MIDI-style note

## Instruments
The instruments (or programs) specified for the `wav` command use the following commands, one per line:

`GEN#`: Set generator for this waveform to # (starting from zero) from the following list:
 - sine
 - half-sine
 - quarter-sine
 - rectified-sine
 - sawtooth
 - square
 - triangle
 - white noise
 - ring buffer (string pluck)
`PM# #`: Set phase modulation to have frequency #0 times the note frequency and magnitude #1
`BW# # #`: Apply a butterworth filter of order #0 and angular cutoff frequency #1. #2 of 0 indicates lopass, 1 indicates hipass.
`CP# # #`: Add a conjugate-pair of zeros or poles with angular frequency #0 and magnitude #1, #2 of 0 indicates a zero, 1 indicates a pole.
`RG#`: Set ring-buffer gain to #
`ENV# # # #`: Set the ADSR envelope to: attack = #0, decay = #1, sustain = #2, release = #3, times in seconds
`BRK`: End this instrument's definition and start the next

As an example, the following commands will generate a songfile named "song.wav":

```
./cli swirl -r 10 -n 10 -w 5 -l 10 -o swirl.bmp
./cli track -n 16 -m 16 -u 16 -t 3 -g 1 -i swirl.bmp -o track.trk
./cli wav -b 8 -r 44100 -i track.trk -o song.wav
```
