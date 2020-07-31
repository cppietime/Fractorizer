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
####Options:
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
####Options:
 - -o: output file name (default to stdout)
 - -l: number of colors in palette, also number of noise layers
 - -c: colors, specifies as r,g,b,r,g,b..., each in [0, 255]
 - -r: comma-separated list of noise resolutions
 - -x: comma-separated list of offsets: x,y,x,y...
 - -f: comma-separated list of flags (1 to take `abs` of noise, else 0)
 - -s: image side length (always square)

### flame
`cli flame` generates a BMP using an IFS fractal flame.
####Options:
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
####Options:
 - -o: output file name (default stdout)
 - -i: input BMP file name
 - -g: generator (0 - random, 1 - linear, 2 - Hilbert Curve)
 - -t: number of tracks/voices
 - -m: length in measures
 - -n: notes per measure
 - -u: number of unique measures to choose from

### midi
`cli track` renders a track to MIDI
####Options:
 - -o: output file name(default stdout)
 - -i: input file name(default stdin)
 - -b: tempo (recommended 4.0 to 8.0)
 - -p: instrument id's, comma separated-list, assigned to voices in-order
 - -d: percussion voice number, or -1 for no percussion
 - -l: minimum MIDI note
 - -h: maximum MIDI note

### wav
`cli wav` renders a WAV from a track
 - -o: output file name(default stdout)
 - -i: input file name(default stdin)
 - -p: instruments file name(leave out to use default)
 - -b: tempo (notes per second)
 - -r: sample rate
 - -l: minimum MIDI-style note
 - -h: maximum MIDI-style note