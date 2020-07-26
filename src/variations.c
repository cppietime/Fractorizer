/**
 * YAAKOV SCHECTMAN 2020
 * variations.c
 */

#include <math.h>
#include "prng.h"
#include "ifs.h"

fract_variation fract_variations[] = {
	fract_var_linear,
	fract_var_sin,
	fract_var_sphere,
	fract_var_swirl,
	fract_var_horseshoe,
	fract_var_polar,
	fract_var_kerchief,
	fract_var_heart,
	fract_var_disc,
	fract_var_spiral,
	fract_var_hyperbolic,
	fract_var_diamond,
	fract_var_X,
	fract_var_julia,
	fract_var_bent,
	fract_var_waves,
	fract_var_fisheye,
	fract_var_popcorn,
	// fract_var_exp,
	fract_var_power,
	// fract_var_cos,
	fract_var_rings,
	fract_var_fan,
	fract_var_blob,
	fract_var_pdj,
	NULL
};

void
fract_var_linear(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	*ox = x;
	*oy = y;
}

void
fract_var_sin(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	*ox = sin(x);
	*oy = sin(y);
}

void
fract_var_sphere(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double r = 1.0 / (x * x + y * y);
	*ox = x * r;
	*oy = y * r;
}

void
fract_var_swirl(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double r2 = x * x + y * y;
	double sr = sin(r2), cr = cos(r2);
	*ox = x * sr - y * cr;
	*oy = x * cr + y * sr;
}

void
fract_var_horseshoe(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double r = 1.0 / hypot(x, y);
	*ox = r * (x - y) * (x + y);
	*oy = r * 2 * x * y;
}

void
fract_var_polar(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = hypot(x, y);
	*ox = theta / M_PI;
	*oy = r - 1;
}

void
fract_var_kerchief(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = hypot(x, y);
	*ox = r * sin(theta + r);
	*oy = r * cos(theta - r);
}

void
fract_var_heart(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = hypot(x, y);
	*ox = r * sin(theta * r);
	*oy = -r * cos(theta * r);
}

void
fract_var_disc(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = hypot(x, y);
	*ox = theta / M_PI * sin(M_PI * r);
	*oy = theta / M_PI * cos(M_PI * r);
}

void
fract_var_spiral(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = hypot(x, y);
	*ox = (cos(theta) + sin(r)) / r;
	*oy = (sin(theta) - cos(r)) / r;
}

void
fract_var_hyperbolic(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = hypot(x, y);
	*ox = sin(theta) / r;
	*oy = r * cos(theta);
}

void
fract_var_diamond(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = hypot(x, y);
	*ox = sin(theta) * cos(r);
	*oy = cos(theta) * sin(r);
}

void
fract_var_X(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = hypot(x, y);
	double p = sin(theta + r), q = cos(theta = r);
	p *= p * p;
	q *= q * q;
	*ox = r * (p + q);
	*oy = r * (p - q);
}

void
fract_var_julia(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y) / 2;
	double r = sqrt(hypot(x, y));
	double omega = fract_lcg_int(lcg, 2) * M_PI;
	*ox = r * cos(theta + omega);
	*oy = r * sin(theta + omega);
}

void
fract_var_bent(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	*ox = (x >= 0) ? x : 2 * x;
	*oy = (y >= 0) ? y : y / 2;
}

void
fract_var_waves(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	*ox = x + affine.linear_transform[1]
		* sin(y / (affine.translation[0] * affine.translation[0]));
	*oy = y + affine.linear_transform[3]
		* sin(x / (affine.translation[1] * affine.translation[1]));
}

void
fract_var_fisheye(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double r = 2.0 / (1 + hypot(x, y));
	*ox = r * y;
	*oy = r * x;
}

void
fract_var_popcorn(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	*ox = x + affine.translation[0] * sin(tan(3 * y));
	*oy = y + affine.translation[1] * sin(tan(3 * x));
}

void
fract_var_exp(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double e = exp(x - 1);
	*ox = e * cos(y * M_PI);
	*oy = e * sin(x * M_PI);
}

void
fract_var_power(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = pow(x * x + y * y, sin(theta) / 2);
	*ox = r * cos(theta);
	*oy = r * sin(theta);
}

void
fract_var_cos(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	*ox = cos(M_PI * x) * cosh(y);
	*oy = -sin(M_PI * x) * sinh(y);
}

void
fract_var_rings(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = hypot(x, y);
	double c = affine.translation[0];
	c *= c;
	double d = fmod(r + c, 2 * c) - c + r * (1 - c);
	*ox = d * cos(theta);
	*oy = d * sin(theta);
}

void
fract_var_fan(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = hypot(x, y);
	double t = affine.translation[0];
	t *= M_PI * t;
	double offset = (fmod(theta + affine.translation[0], t) > t/2)
		? -t / 2 : t / 2;
	*ox = r * cos(theta + offset);
	*oy = r * sin(theta + offset);
}

void
fract_var_blob(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	double theta = atan2(x, y);
	double r = hypot(x, y);
	double param = r * (parameters[1] +
		(parameters[0] - parameters[1]) / 2 * (sin(parameters[2] * theta) + 1));
	*ox = param * cos(theta);
	*oy = param * sin(theta);
}

void
fract_var_pdj(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy)
{
	*ox = sin(parameters[0] * x) - cos(parameters[1] * y);
	*oy = sin(parameters[2] * x) - cos(parameters[3] * y);
}