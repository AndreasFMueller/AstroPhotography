/*
 * PerturbedPlanets.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

#define	ADD(perturber, perturbed_i, perturber_i, T_exponent, 		\
	 dl_cos, dl_sin, dr_cos, dr_sin, db_cos, db_sin) 		\
perturber->add(perturbed_i, perturber_i, T_exponent, 			\
	dl_cos, dl_sin,	dr_cos, dr_sin, db_cos, db_sin)

/**
 * \brief Set up perturbation series coefficients for Mercury
 */
MercuryPerturbed::MercuryPerturbed() : PerturbedPlanetoid(Mercury()) {
	M0(Angle(0.4855407, Angle::Revolutions));
	n(Angle(415.2014314, Angle::Revolutions));

	venus = PerturbationSeriesPtr(new PerturbationSeries(*this, Venus()));
	venus->perturber().M0(Angle(0.1394222, Angle::Revolutions));
	venus->perturber().n(Angle(162.5490444, Angle::Revolutions));
	ADD(venus,  1, 0,0, 259.74,84547.39,-78342.34, 0.01,11683.22,21203.79);
	ADD(venus,  1, 0,1,   2.30,    5.04,    -7.52, 0.02,  138.55,  -71.01);
	ADD(venus,  1, 0,2,   0.01,   -0.01,     0.01, 0.01,   -0.19,   -0.54);
	ADD(venus,  2, 0,0,-549.71,10394.44, -7955.45, 0.00, 2390.29, 4306.79);
	ADD(venus,  2, 0,1,  -4.77,    8.97,    -1.53, 0.00,   28.49,  -14.18);
	ADD(venus,  2, 0,2,   0.00,    0.00,     0.00, 0.00,   -0.04,   -0.11);
	ADD(venus,  3, 0,0,-234.04, 1748.74, -1212.86, 0.00,  535.41,  984.33);
	ADD(venus,  3, 0,1,  -2.03,    3.48,    -0.35, 0.00,    6.56,   -2.91);
	ADD(venus,  4, 0,0, -77.64,  332.63,  -219.23, 0.00,  124.40,  237.03);
	ADD(venus,  4, 0,1,  -0.70,    1.10,    -0.08, 0.00,    1.59,   -0.59);
	ADD(venus,  5, 0,0, -23.59,   67.28,   -43.54, 0.00,   29.44,   58.77);
	ADD(venus,  5, 0,1,  -0.23,    0.32,    -0.02, 0.00,    0.39,   -0.11);
	ADD(venus,  6, 0,0,  -6.86,   14.06,    -9.18, 0.00,    7.03,   14.84);
	ADD(venus,  6, 0,1,  -0.07,    0.09,    -0.01, 0.00,    0.10,   -0.02);
	ADD(venus,  7, 0,0,  -1.94,    2.98,    -2.02, 0.00,    1.69,    3.80);
	ADD(venus,  8, 0,0,  -0.54,    0.63,    -0.46, 0.00,    0.41,    0.98);
	ADD(venus,  9, 0,0,  -0.15,    0.13,    -0.11, 0.00,    0.10,    0.25);
	ADD(venus, -1,-2,0,  -0.17,   -0.06,    -0.05, 0.14,   -0.06,   -0.07);
	ADD(venus,  0,-1,0,   0.24,   -0.16,    -0.11,-0.16,    0.04,   -0.01);
	ADD(venus,  0,-2,0,  -0.68,   -0.25,    -0.26, 0.73,   -0.16,   -0.18);
	ADD(venus,  0,-5,0,   0.37,    0.08,     0.06,-0.28,    0.13,    0.12);
	ADD(venus,  1,-1,0,   0.58,   -0.41,     0.26, 0.36,    0.01,   -0.01);
	ADD(venus,  1,-2,0,  -3.51,   -1.23,     0.23,-0.63,   -0.05,   -0.06);
	ADD(venus,  1,-3,0,   0.08,    0.53,    -0.11, 0.04,    0.02,   -0.09);
	ADD(venus,  1,-5,0,   1.44,    0.31,     0.30,-1.39,    0.34,    0.29);
	ADD(venus,  2,-1,0,   0.15,   -0.11,     0.09, 0.12,    0.02,   -0.04);
	ADD(venus,  2,-2,0,  -1.99,   -0.68,     0.65,-1.91,   -0.20,    0.03);
	ADD(venus,  2,-3,0,  -0.34,   -1.28,     0.97,-0.26,    0.03,    0.03);
	ADD(venus,  2,-4,0,  -0.33,    0.35,    -0.13,-0.13,   -0.01,    0.00);
	ADD(venus,  2,-5,0,   7.19,    1.56,    -0.05, 0.12,    0.06,    0.05);
	ADD(venus,  3,-2,0,  -0.52,   -0.18,     0.13,-0.39,   -0.16,    0.03);
	ADD(venus,  3,-3,0,  -0.11,   -0.42,     0.36,-0.10,   -0.05,   -0.05);
	ADD(venus,  3,-4,0,  -0.19,    0.22,    -0.23,-0.20,   -0.01,    0.02);
	ADD(venus,  3,-5,0,   2.77,    0.49,    -0.45, 2.56,    0.40,   -0.12);
	ADD(venus,  4,-5,0,   0.67,    0.12,    -0.09, 0.47,    0.24,   -0.08);
	ADD(venus,  5,-5,0,   0.18,    0.03,    -0.02, 0.12,    0.09,   -0.03);
	add(venus);

	earth = PerturbationSeriesPtr(new PerturbationSeries(*this, Earth()));
	earth->perturber().M0(Angle(0.9937861, Angle::Revolutions));
	earth->perturber().n(Angle(99.9978139, Angle::Revolutions));
	ADD(earth,  0,-4,0,  -0.11,   -0.07,    -0.08, 0.11,   -0.02,   -0.04);
	ADD(earth,  1,-1,0,   0.10,   -0.20,     0.15, 0.07,    0.00,    0.00);
	ADD(earth,  1,-2,0,  -0.35,    0.28,    -0.13,-0.17,   -0.01,    0.00);
	ADD(earth,  1,-4,0,  -0.67,   -0.45,     0.00, 0.01,   -0.01,   -0.01);
	ADD(earth,  2,-2,0,  -0.20,    0.16,    -0.16,-0.20,   -0.01,    0.02);
	ADD(earth,  2,-3,0,   0.13,   -0.02,     0.02, 0.14,    0.01,    0.00);
	ADD(earth,  2,-4,0,  -0.33,   -0.18,     0.17,-0.31,   -0.04,    0.00);
	add(earth);

	jupiter = PerturbationSeriesPtr(new PerturbationSeries(*this, Jupiter()));
	jupiter->perturber().M0(Angle(0.0558417, Angle::Revolutions));
	jupiter->perturber().n(Angle(8.4298417, Angle::Revolutions));
	ADD(jupiter, -1,-1,0,  -0.08,    0.16,     0.15, 0.08,   -0.04,    0.01);
	ADD(jupiter, -1,-2,0,   0.10,   -0.06,    -0.07,-0.12,    0.07,   -0.01);
	ADD(jupiter,  0,-1,0,  -0.31,    0.48,    -0.02, 0.13,   -0.03,   -0.02);
	ADD(jupiter,  0,-2,0,   0.42,   -0.26,    -0.38,-0.50,    0.20,   -0.03);
	ADD(jupiter,  1,-1,0,  -0.70,    0.01,    -0.02,-0.63,    0.00,    0.03);
	ADD(jupiter,  1,-2,0,   2.61,   -1.97,     1.74, 2.32,    0.01,    0.01);
	ADD(jupiter,  1,-3,0,   0.32,   -0.15,     0.13, 0.28,    0.00,    0.00);
	ADD(jupiter,  2,-1,0,  -0.18,    0.01,     0.00,-0.13,   -0.03,    0.03);
	ADD(jupiter,  2,-2,0,   0.75,   -0.56,     0.45, 0.60,    0.08,   -0.17);
	ADD(jupiter,  3,-2,0,   0.20,   -0.15,     0.10, 0.14,    0.04,   -0.08);
	add(jupiter);

	saturn = PerturbationSeriesPtr(new PerturbationSeries(*this, Saturn()));
	saturn->perturber().M0(Angle(0.8823333, Angle::Revolutions));
	saturn->perturber().n(Angle(3.3943333, Angle::Revolutions));
	ADD(saturn, 1,-2,0,  -0.19,    0.33,     0.00, 0.00,    0.00,    0.00);
	add(saturn);
}

/**
 * \brief Compute the unperturbed position of mercury
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	MercuryPerturbed::position(const JulianCenturies& T) const {
	Angle	lt0(5601.7, Angle::ArcSeconds);
	Angle	lt1(1.1, Angle::ArcSeconds);
	Angle	l = Angle(0.2151379, Angle::Revolutions) + M(T) + (lt0 + lt1 * T) * T;
	double	r = 0.3952829 + 0.0000016 * T;
	Angle	bt0(-2522.15, Angle::ArcSeconds);
	Angle	bt1(-30.18, Angle::ArcSeconds);
	Angle	bt2(-0.04, Angle::ArcSeconds);
	Angle	b = bt0 + (bt1 + bt2 * T) * T;
	return EclipticalCoordinates(l, r, b);
}

/**
 * \brief Set up perturbation series coefficients for Venus
 */
VenusPerturbed::VenusPerturbed() : PerturbedPlanetoid(Venus()) {
	M0(Angle(0.1400197, Angle::Revolutions));
	n(Angle(162.5494552, Angle::Revolutions));

	mercury = PerturbationSeriesPtr(new PerturbationSeries(*this, Mercury()));
	mercury->perturber().M0(Angle(0.4861431, Angle::Revolutions));
	mercury->perturber().n(Angle(415.2018375, Angle::Revolutions));
	ADD(mercury, 1,-1,0,   0.00,   0.00,    0.06, -0.09,   0.01,   0.00);
	ADD(mercury, 2,-1,0,   0.25,  -0.09,   -0.09, -0.27,   0.00,   0.00);
	ADD(mercury, 4,-2,0,  -0.07,  -0.08,   -0.14,  0.14,  -0.01,  -0.01);
	ADD(mercury, 5,-2,0,  -0.35,   0.08,    0.02,  0.09,   0.00,   0.00);
	add(mercury);

	earth = PerturbationSeriesPtr(new PerturbationSeries(*this, Earth()));
	earth->perturber().M0(Angle(0.9944153, Angle::Revolutions));
	earth->perturber().n(Angle(99.9982208, Angle::Revolutions));
	ADD(earth, 1, 0,0,   2.37,2793.23,-4899.07,  0.11,9995.27,7027.22);
	ADD(earth, 1, 0,1,   0.10, -19.65,   34.40,  0.22,  64.95, -86.10);
	ADD(earth, 1, 0,2,   0.06,   0.04,   -0.07,  0.11,  -0.55,  -0.07);
	ADD(earth, 2, 0,0,-170.42,  73.13,  -16.59,  0.00,  67.71,  47.56);
	ADD(earth, 2, 0,1,   0.93,   2.91,    0.23,  0.00,  -0.03,  -0.92);
	ADD(earth, 3, 0,0,  -2.31,   0.90,   -0.08,  0.00,   0.04,   2.09);
	ADD(earth, 1,-1,0,  -2.38,  -4.27,    3.27, -1.82,   0.00,   0.00);
	ADD(earth, 1,-2,0,   0.09,   0.00,   -0.08,  0.05,  -0.02,  -0.25);
	ADD(earth, 2,-2,0,  -9.57,  -5.93,    8.57,-13.83,  -0.01,  -0.01);
	ADD(earth, 2,-3,0,  -2.47,  -2.40,    0.83, -0.95,   0.16,   0.24);
	ADD(earth, 3,-2,0,  -0.09,  -0.05,    0.08, -0.13,  -0.28,   0.12);
	ADD(earth, 3,-3,0,   7.12,   0.32,   -0.62, 13.76,  -0.07,   0.01);
	ADD(earth, 3,-4,0,  -0.65,  -0.17,    0.18, -0.73,   0.10,   0.05);
	ADD(earth, 3,-5,0,  -1.08,  -0.95,   -0.17,  0.22,  -0.03,  -0.03);
	ADD(earth, 4,-3,0,   0.06,   0.00,   -0.01,  0.08,   0.14,  -0.18);
	ADD(earth, 4,-4,0,   0.93,  -0.46,    1.06,  2.13,  -0.01,   0.01);
	ADD(earth, 4,-5,0,  -1.53,   0.38,   -0.64, -2.54,   0.27,   0.00);
	ADD(earth, 4,-6,0,  -0.17,  -0.05,    0.03, -0.11,   0.02,   0.00);
	ADD(earth, 5,-5,0,   0.18,  -0.28,    0.71,  0.47,  -0.02,   0.04);
	ADD(earth, 5,-6,0,   0.15,  -0.14,    0.30,  0.31,  -0.04,   0.03);
	ADD(earth, 5,-7,0,  -0.08,   0.02,   -0.03, -0.11,   0.01,   0.00);
	ADD(earth, 5,-8,0,  -0.23,   0.00,    0.01, -0.04,   0.00,   0.00);
	ADD(earth, 6,-6,0,   0.01,  -0.14,    0.39,  0.04,   0.00,  -0.01);
	ADD(earth, 6,-7,0,   0.02,  -0.05,    0.12,  0.04,  -0.01,   0.01);
	ADD(earth, 6,-8,0,   0.10,  -0.10,    0.19,  0.19,  -0.02,   0.02);
	ADD(earth, 7,-7,0,  -0.03,  -0.06,    0.18, -0.08,   0.00,   0.00);
	ADD(earth, 8,-8,0,  -0.03,  -0.02,    0.06, -0.08,   0.00,   0.00);
	add(earth);

	mars = PerturbationSeriesPtr(new PerturbationSeries(*this, Mars()));
	mars->perturber().M0(Angle(0.0556297, Angle::Revolutions));
	mars->perturber().n(Angle(53.1674631, Angle::Revolutions));
	ADD(mars, 1,-3,0,  -0.65,   1.02,   -0.04, -0.02,  -0.02,   0.00);
	ADD(mars, 2,-2,0,  -0.05,   0.04,   -0.09, -0.10,   0.00,   0.00);
	ADD(mars, 2,-3,0,  -0.50,   0.45,   -0.79, -0.89,   0.01,   0.03);
	add(mars);

	jupiter = PerturbationSeriesPtr(new PerturbationSeries(*this, Jupiter()));
	jupiter->perturber().M0(Angle(0.0567028, Angle::Revolutions));
	jupiter->perturber().n(Angle(8.4305083, Angle::Revolutions));
	ADD(jupiter, 0,-1,0,  -0.05,   1.56,    0.16,  0.04,  -0.08,  -0.04);
	ADD(jupiter, 1,-1,0,  -2.62,   1.40,   -2.35, -4.40,   0.02,   0.03);
	ADD(jupiter, 1,-2,0,  -0.47,  -0.08,    0.12, -0.76,   0.04,  -0.18);
	ADD(jupiter, 2,-2,0,  -0.73,  -0.51,    1.27, -1.82,  -0.01,   0.01);
	ADD(jupiter, 2,-3,0,  -0.14,  -0.10,    0.25, -0.34,   0.00,   0.00);
	ADD(jupiter, 3,-3,0,  -0.01,   0.04,   -0.11, -0.02,   0.00,   0.00);
	add(jupiter);

	saturn = PerturbationSeriesPtr(new PerturbationSeries(*this, Saturn()));
	saturn->perturber().M0(Angle(0.8830539, Angle::Revolutions));
	saturn->perturber().n(Angle(3.3947206, Angle::Revolutions));
	ADD(saturn, 0,-1,0,   0.00,   0.21,    0.00,  0.00,   0.00,  -0.01);
	ADD(saturn, 1,-1,0,  -0.11,  -0.14,    0.24, -0.20,   0.01,   0.00);
	add(saturn);
}

/**
 * \brief Compute the unperturbed position of venus
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	VenusPerturbed::position(const JulianCenturies& T) const {
	Angle	lt0(5071.2, Angle::ArcSeconds);
	Angle	lt1(1.1, Angle::ArcSeconds);
	Angle	l = Angle(0.3654783 , Angle::Revolutions) + M(T) + (lt0 + lt1 * T) * T;
	double r = 0.7233482 - 0.0000002 * T;
	Angle	bt0(-67.70, Angle::ArcSeconds);
	Angle	bt1(0.04, Angle::ArcSeconds);
	Angle	bt2(0.01, Angle::ArcSeconds);
	Angle	b = bt0 + (bt1 + bt2 * T) * T;
	return EclipticalCoordinates(l, r, b);
}

/**
 * \brief Additional Corrections to the orbit of Venus
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	VenusPerturbed::corrections(const JulianCenturies& T) const {
	Angle	dl = Angle(2.74, Angle::ArcSeconds)
			* sin(Angle(0.0764, Angle::Revolutions)
				+ Angle(0.4174, Angle::Revolutions) * T);
	dl = dl + Angle(0.27, Angle::ArcSeconds)
			* sin(Angle(0.9201, Angle::Revolutions)
				+ Angle(0.3307, Angle::Revolutions) * T);
	dl = dl + Angle(1.9, Angle::ArcSeconds) + Angle(1.8, Angle::ArcSeconds) * T;
	return EclipticalCoordinates(dl, 0, Angle(0));
}

/**
 * \brief Set up perturbation series coefficients for Earth
 */
EarthPerturbed::EarthPerturbed() : PerturbedPlanetoid(Earth()) {
	M0(Angle(0.9931266, Angle::Revolutions));
	n(Angle(99.9973604, Angle::Revolutions));

	venus = PerturbationSeriesPtr(new PerturbationSeries(*this, Venus()));
	venus->perturber().M0(Angle(0.1387306, Angle::Revolutions));
	venus->perturber().n(Angle(162.5485917, Angle::Revolutions));
	ADD(venus,  1, 0,0,-0.22,6892.76,-16707.37, -0.54, 0.00, 0.00);
	ADD(venus,  1, 0,1,-0.06, -17.35,    42.04, -0.15, 0.00, 0.00);
	ADD(venus,  1, 0,2,-0.01,  -0.05,     0.13, -0.02, 0.00, 0.00);
	ADD(venus,  2, 0,0, 0.00,  71.98,  -139.57,  0.00, 0.00, 0.00);
	ADD(venus,  2, 0,1, 0.00,  -0.36,     0.70,  0.00, 0.00, 0.00);
	ADD(venus,  3, 0,0, 0.00,   1.04,    -1.75,  0.00, 0.00, 0.00);
	ADD(venus,  0,-1,0, 0.03,  -0.07,    -0.16, -0.07, 0.02,-0.02);
	ADD(venus,  1,-1,0, 2.35,  -4.23,    -4.75, -2.64, 0.00, 0.00);
	ADD(venus,  1,-2,0,-0.10,   0.06,     0.12,  0.20, 0.02, 0.00);
	ADD(venus,  2,-1,0,-0.06,  -0.03,     0.20, -0.01, 0.01,-0.09);
	ADD(venus,  2,-2,0,-4.70,   2.90,     8.28, 13.42, 0.01,-0.01);
	ADD(venus,  3,-2,0, 1.80,  -1.74,    -1.44, -1.57, 0.04,-0.06);
	ADD(venus,  3,-3,0,-0.67,   0.03,     0.11,  2.43, 0.01, 0.00);
	ADD(venus,  4,-2,0, 0.03,  -0.03,     0.10,  0.09, 0.01,-0.01);
	ADD(venus,  4,-3,0, 1.51,  -0.40,    -0.88, -3.36, 0.18,-0.10);
	ADD(venus,  4,-4,0,-0.19,  -0.09,    -0.38,  0.77, 0.00, 0.00);
	ADD(venus,  5,-3,0, 0.76,  -0.68,     0.30,  0.37, 0.01, 0.00);
	ADD(venus,  5,-4,0,-0.14,  -0.04,    -0.11,  0.43,-0.03, 0.00);
	ADD(venus,  5,-5,0,-0.05,  -0.07,    -0.31,  0.21, 0.00, 0.00);
	ADD(venus,  6,-4,0, 0.15,  -0.04,    -0.06, -0.21, 0.01, 0.00);
	ADD(venus,  6,-5,0,-0.03,  -0.03,    -0.09,  0.09,-0.01, 0.00);
	ADD(venus,  6,-6,0, 0.00,  -0.04,    -0.18,  0.02, 0.00, 0.00);
	ADD(venus,  7,-5,0,-0.12,  -0.03,    -0.08,  0.31,-0.02,-0.01);
	add(venus);

	mars = PerturbationSeriesPtr(new PerturbationSeries(*this, Mars()));
	mars->perturber().M0(Angle(0.0543250, Angle::Revolutions));
	mars->perturber().n(Angle(53.1666028, Angle::Revolutions));
	ADD(mars,  1,-1,0,-0.22,   0.17,    -0.21, -0.27, 0.00, 0.00);
	ADD(mars,  1,-2,0,-1.66,   0.62,     0.16,  0.28, 0.00, 0.00);
	ADD(mars,  2,-2,0, 1.96,   0.57,    -1.32,  4.55, 0.00, 0.01);
	ADD(mars,  2,-3,0, 0.40,   0.15,    -0.17,  0.46, 0.00, 0.00);
	ADD(mars,  2,-4,0, 0.53,   0.26,     0.09, -0.22, 0.00, 0.00);
	ADD(mars,  3,-3,0, 0.05,   0.12,    -0.35,  0.15, 0.00, 0.00);
	ADD(mars,  3,-4,0,-0.13,  -0.48,     1.06, -0.29, 0.01, 0.00);
	ADD(mars,  3,-5,0,-0.04,  -0.20,     0.20, -0.04, 0.00, 0.00);
	ADD(mars,  4,-4,0, 0.00,  -0.03,     0.10,  0.04, 0.00, 0.00);
	ADD(mars,  4,-5,0, 0.05,  -0.07,     0.20,  0.14, 0.00, 0.00);
	ADD(mars,  4,-6,0,-0.10,   0.11,    -0.23, -0.22, 0.00, 0.00);
	ADD(mars,  5,-7,0,-0.05,   0.00,     0.01, -0.14, 0.00, 0.00);
	ADD(mars,  5,-8,0, 0.05,   0.01,    -0.02,  0.10, 0.00, 0.00);
	add(mars);

	jupiter = PerturbationSeriesPtr(new PerturbationSeries(*this, Jupiter()));
	jupiter->perturber().M0(Angle(0.0551750, Angle::Revolutions));
	jupiter->perturber().n(Angle(8.4293972, Angle::Revolutions));
	ADD(jupiter, -1,-1,0, 0.01,   0.07,     0.18, -0.02, 0.00,-0.02);
	ADD(jupiter,  0,-1,0,-0.31,   2.58,     0.52,  0.34, 0.02, 0.00);
	ADD(jupiter,  1,-1,0,-7.21,  -0.06,     0.13,-16.27, 0.00,-0.02);
	ADD(jupiter,  1,-2,0,-0.54,  -1.52,     3.09, -1.12, 0.01,-0.17);
	ADD(jupiter,  1,-3,0,-0.03,  -0.21,     0.38, -0.06, 0.00,-0.02);
	ADD(jupiter,  2,-1,0,-0.16,   0.05,    -0.18, -0.31, 0.01, 0.00);
	ADD(jupiter,  2,-2,0, 0.14,  -2.73,     9.23,  0.48, 0.00, 0.00);
	ADD(jupiter,  2,-3,0, 0.07,  -0.55,     1.83,  0.25, 0.01, 0.00);
	ADD(jupiter,  2,-4,0, 0.02,  -0.08,     0.25,  0.06, 0.00, 0.00);
	ADD(jupiter,  3,-2,0, 0.01,  -0.07,     0.16,  0.04, 0.00, 0.00);
	ADD(jupiter,  3,-3,0,-0.16,  -0.03,     0.08, -0.64, 0.00, 0.00);
	ADD(jupiter,  3,-4,0,-0.04,  -0.01,     0.03, -0.17, 0.00, 0.00);
	add(jupiter);

	saturn = PerturbationSeriesPtr(new PerturbationSeries(*this, Saturn()));
	saturn->perturber().M0(Angle(0.8816500, Angle::Revolutions));
	saturn->perturber().n(Angle(3.3938722, Angle::Revolutions));
	ADD(saturn,  0,-1,0, 0.00,   0.32,     0.01,  0.00, 0.00, 0.00);
	ADD(saturn,  1,-1,0,-0.08,  -0.41,     0.97, -0.18, 0.00,-0.01);
	ADD(saturn,  1,-2,0, 0.04,   0.10,    -0.23,  0.10, 0.00, 0.00);
	ADD(saturn,  2,-2,0, 0.04,   0.10,    -0.35,  0.13, 0.00, 0.00);
	add(saturn);
}

EclipticalCoordinates	EarthPerturbed::corrections(const JulianCenturies& T) const {
	// extract relevant angles
	Angle	M3 = M(T);
	Angle	D = Angle(0.8274, Angle::Revolutions)
			+ Angle(1236.8531, Angle::Revolutions) * T;
	Angle	A = Angle(0.3749, Angle::Revolutions)
			+ Angle(1325.5524, Angle::Revolutions) * T;
	Angle	U = Angle(0.2591, Angle::Revolutions)
			+ Angle(1342.2278, Angle::Revolutions) * T;
	
	// lunar corrections
	Angle	dl = 	  Angle(6.45, Angle::ArcSeconds) * sin(D)
			- Angle(0.42, Angle::ArcSeconds) * sin(D-A)
			+ Angle(0.18, Angle::ArcSeconds) * sin(D+A)
			+ Angle(0.17, Angle::ArcSeconds) * sin(D-M3)
			- Angle(0.06, Angle::ArcSeconds) * sin(D+M3);
	double	dr = 30.76 * cos(D) - 3.06 * cos(D - A) + 0.85 * cos(D + A)
        		- 0.58 * cos(D + M3) + 0.57 * cos(D - M3);
	Angle	db = Angle(0.576, Angle::ArcSeconds) * sin(U);

	// longperiod perturbations
	dl = dl + Angle(6.40, Angle::ArcSeconds)
			* sin(Angle(0.6983, Angle::Revolutions)
				+ Angle(0.0561, Angle::Revolutions) * T)
		+ Angle(1.87, Angle::ArcSeconds)
			* sin(Angle(0.5764, Angle::Revolutions)
				+ Angle(0.4174, Angle::Revolutions) * T)
		+ Angle(0.27, Angle::ArcSeconds)
			* sin(Angle(0.4189, Angle::Revolutions)
				+ Angle(0.3306, Angle::Revolutions) * T)
		+ Angle(0.20, Angle::ArcSeconds)
			* sin(Angle(0.3581, Angle::Revolutions)
				+ Angle(2.4814, Angle::Revolutions) * T);

	EclipticalCoordinates	result(dl, dr * 1e-6, db);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "earth corrections: %s",
		result.toString().c_str());
	return result;
}

/**
 * \brief Determine the position of Earth
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	EarthPerturbed::position(const JulianCenturies& T) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "computing perturbed position of earth: T = %f",
		(double)T);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "earth M = %f", M(T).degrees());
	Angle	lt0(6191.2, Angle::ArcSeconds);
	Angle	lt1(1.1, Angle::ArcSeconds);
	Angle	l = Angle(0.7859453, Angle::Revolutions) + M(T) + (lt0 + lt1 * T) * T;
	double	r = 1.0001398 - 0.0000007 * T;
	EclipticalCoordinates	result(l, r, 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unperturbed earth position: %s",
		result.toString().c_str());
	return result;
}

/**
 * \brief Compute the position of the earth
 *
 * The perturbation series actually computes the relative position of the sun,
 * we have to change the sign to actually get the position of earth
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	EarthPerturbed::ecliptical(const JulianCenturies& T) const {
	EclipticalCoordinates	sunpos = PerturbedPlanetoid::ecliptical(T);
	EclipticalCoordinates	result(sunpos.l() + Angle(M_PI, Angle::Radians),
		sunpos.r(), -sunpos.b());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "earth ecliptic coordinates: %s",
		result.toString().c_str());
	return result;
}

/**
 * \brief Set up perturbation series coefficients for Mars
 */
MarsPerturbed::MarsPerturbed() : PerturbedPlanetoid(Mars()) {
	M0(Angle(0.0538553, Angle::Revolutions));
	n(Angle(53.1662736, Angle::Revolutions));

	venus = PerturbationSeriesPtr(new PerturbationSeries(*this, Venus()));
	venus->perturber().M0(Angle(0.1382208, Angle::Revolutions));
	venus->perturber().n(Angle(162.5482542, Angle::Revolutions));
	ADD(venus,  0,-1,0, -0.01,   -0.03,      0.10, -0.04,    0.00,   0.00);
	ADD(venus,  1,-1,0,  0.05,    0.10,     -2.08,  0.75,    0.00,   0.00);
	ADD(venus,  2,-1,0, -0.25,   -0.57,     -2.58,  1.18,    0.05,  -0.04);
	ADD(venus,  2,-2,0,  0.02,    0.02,      0.13, -0.14,    0.00,   0.00);
	ADD(venus,  3,-1,0,  3.41,    5.38,      1.87, -1.15,    0.01,  -0.01);
	ADD(venus,  3,-2,0,  0.02,    0.02,      0.11, -0.13,    0.00,   0.00);
	ADD(venus,  4,-1,0,  0.32,    0.49,     -1.88,  1.21,   -0.07,   0.07);
	ADD(venus,  4,-2,0,  0.03,    0.03,      0.12, -0.14,    0.00,   0.00);
	ADD(venus,  5,-1,0,  0.04,    0.06,     -0.17,  0.11,   -0.01,   0.01);
	ADD(venus,  5,-2,0,  0.11,    0.09,      0.35, -0.43,   -0.01,   0.01);
	ADD(venus,  6,-2,0, -0.36,   -0.28,     -0.20,  0.25,    0.00,   0.00);
	ADD(venus,  7,-2,0, -0.03,   -0.03,      0.11, -0.13,    0.00,  -0.01);
	add(venus);

	earth = PerturbationSeriesPtr(new PerturbationSeries(*this, Earth()));
	earth->perturber().M0(Angle(0.9926208, Angle::Revolutions));
	earth->perturber().n(Angle(99.9970236, Angle::Revolutions));
	ADD(earth,  1, 0,0, -5.32,38481.97,-141856.04,  0.40,-6321.67,1876.89);
	ADD(earth,  1, 0,1, -1.12,   37.98,   -138.67, -2.93,   37.28, 117.48);
	ADD(earth,  1, 0,2, -0.32,   -0.03,      0.12, -1.19,    1.04,  -0.40);
	ADD(earth,  2, 0,0, 28.28, 2285.80,  -6608.37,  0.00, -589.35, 174.81);
	ADD(earth,  2, 0,1,  1.64,    3.37,    -12.93,  0.00,    2.89,  11.10);
	ADD(earth,  2, 0,2,  0.00,    0.00,      0.00,  0.00,    0.10,  -0.03);
	ADD(earth,  3, 0,0,  5.31,  189.29,   -461.81,  0.00,  -61.98,  18.53);
	ADD(earth,  3, 0,1,  0.31,    0.35,     -1.36,  0.00,    0.25,   1.19);
	ADD(earth,  4, 0,0,  0.81,   17.96,    -38.26,  0.00,   -6.88,   2.08);
	ADD(earth,  4, 0,1,  0.05,    0.04,     -0.15,  0.00,    0.02,   0.14);
	ADD(earth,  5, 0,0,  0.11,    1.83,     -3.48,  0.00,   -0.79,   0.24);
	ADD(earth,  6, 0,0,  0.02,    0.20,     -0.34,  0.00,   -0.09,   0.03);
	ADD(earth, -1,-1,0,  0.09,    0.06,      0.14, -0.22,    0.02,  -0.02);
	ADD(earth,  0,-1,0,  0.72,    0.49,      1.55, -2.31,    0.12,  -0.10);
	ADD(earth,  1,-1,0,  7.00,    4.92,     13.93,-20.48,    0.08,  -0.13);
	ADD(earth,  2,-1,0, 13.08,    4.89,     -4.53, 10.01,   -0.05,   0.13);
	ADD(earth,  2,-2,0,  0.14,    0.05,     -0.48, -2.66,    0.01,   0.14);
	ADD(earth,  3,-1,0,  1.38,    0.56,     -2.00,  4.85,   -0.01,   0.19);
	ADD(earth,  3,-2,0, -6.85,    2.68,      8.38, 21.42,    0.00,   0.03);
	ADD(earth,  3,-3,0, -0.08,    0.20,      1.20,  0.46,    0.00,   0.00);
	ADD(earth,  4,-1,0,  0.16,    0.07,     -0.19,  0.47,   -0.01,   0.05);
	ADD(earth,  4,-2,0, -4.41,    2.14,     -3.33, -7.21,   -0.07,  -0.09);
	ADD(earth,  4,-3,0, -0.12,    0.33,      2.22,  0.72,   -0.03,  -0.02);
	ADD(earth,  4,-4,0, -0.04,   -0.06,     -0.36,  0.23,    0.00,   0.00);
	ADD(earth,  5,-2,0, -0.44,    0.21,     -0.70, -1.46,   -0.06,  -0.07);
	ADD(earth,  5,-3,0,  0.48,   -2.60,     -7.25, -1.37,    0.00,   0.00);
	ADD(earth,  5,-4,0, -0.09,   -0.12,     -0.66,  0.50,    0.00,   0.00);
	ADD(earth,  5,-5,0,  0.03,    0.00,      0.01, -0.17,    0.00,   0.00);
	ADD(earth,  6,-2,0, -0.05,    0.03,     -0.07, -0.15,   -0.01,  -0.01);
	ADD(earth,  6,-3,0,  0.10,   -0.96,      2.36,  0.30,    0.04,   0.00);
	ADD(earth,  6,-4,0, -0.17,   -0.20,     -1.09,  0.94,    0.02,  -0.02);
	ADD(earth,  6,-5,0,  0.05,    0.00,      0.00, -0.30,    0.00,   0.00);
	ADD(earth,  7,-3,0,  0.01,   -0.10,      0.32,  0.04,    0.02,   0.00);
	ADD(earth,  7,-4,0,  0.86,    0.77,      1.86, -2.01,    0.01,  -0.01);
	ADD(earth,  7,-5,0,  0.09,   -0.01,     -0.05, -0.44,    0.00,   0.00);
	ADD(earth,  7,-6,0, -0.01,    0.02,      0.10,  0.08,    0.00,   0.00);
	ADD(earth,  8,-4,0,  0.20,    0.16,     -0.53,  0.64,   -0.01,   0.02);
	ADD(earth,  8,-5,0,  0.17,   -0.03,     -0.14, -0.84,    0.00,   0.01);
	ADD(earth,  8,-6,0, -0.02,    0.03,      0.16,  0.09,    0.00,   0.00);
	ADD(earth,  9,-5,0, -0.55,    0.15,      0.30,  1.10,    0.00,   0.00);
	ADD(earth,  9,-6,0, -0.02,    0.04,      0.20,  0.10,    0.00,   0.00);
	ADD(earth, 10,-5,0, -0.09,    0.03,     -0.10, -0.33,    0.00,  -0.01);
	ADD(earth, 10,-6,0, -0.05,    0.11,      0.48,  0.21,   -0.01,   0.00);
	ADD(earth, 11,-6,0,  0.10,   -0.35,     -0.52, -0.15,    0.00,   0.00);
	ADD(earth, 11,-7,0, -0.01,   -0.02,     -0.10,  0.07,    0.00,   0.00);
	ADD(earth, 12,-6,0,  0.01,   -0.04,      0.18,  0.04,    0.01,   0.00);
	ADD(earth, 12,-7,0, -0.05,   -0.07,     -0.29,  0.20,    0.01,   0.00);
	ADD(earth, 13,-7,0,  0.23,    0.27,      0.25, -0.21,    0.00,   0.00);
	ADD(earth, 14,-7,0,  0.02,    0.03,     -0.10,  0.09,    0.00,   0.00);
	ADD(earth, 14,-8,0,  0.05,    0.01,      0.03, -0.23,    0.00,   0.03);
	ADD(earth, 15,-8,0, -1.53,    0.27,      0.06,  0.42,    0.00,   0.00);
	ADD(earth, 16,-8,0, -0.14,    0.02,     -0.10, -0.55,   -0.01,  -0.02);
	ADD(earth, 16,-9,0,  0.03,   -0.06,     -0.25, -0.11,    0.00,   0.00);
	add(earth);

	jupiter = PerturbationSeriesPtr(new PerturbationSeries(*this, Jupiter()));
	jupiter->perturber().M0(Angle(0.0548944, Angle::Revolutions));
	jupiter->perturber().n(Angle(8.4290611, Angle::Revolutions));
	ADD(jupiter, -2,-1,0,  0.05,    0.03,      0.08, -0.14,    0.01,  -0.01);
	ADD(jupiter, -1,-1,0,  0.39,    0.27,      0.92, -1.50,   -0.03,  -0.06);
	ADD(jupiter, -1,-2,0, -0.16,    0.03,      0.13,  0.67,   -0.01,   0.06);
	ADD(jupiter, -1,-3,0, -0.02,    0.01,      0.05,  0.09,    0.00,   0.01);
	ADD(jupiter,  0,-1,0,  3.56,    1.13,     -5.41, -7.18,   -0.25,  -0.24);
	ADD(jupiter,  0,-2,0, -1.44,    0.25,      1.24,  7.96,    0.02,   0.31);
	ADD(jupiter,  0,-3,0, -0.21,    0.11,      0.55,  1.04,    0.01,   0.05);
	ADD(jupiter,  0,-4,0, -0.02,    0.02,      0.11,  0.11,    0.00,   0.01);
	ADD(jupiter,  1,-1,0, 16.67,  -19.15,     61.00, 53.36,   -0.06,  -0.07);
	ADD(jupiter,  1,-2,0,-21.64,    3.18,     -7.77,-54.64,   -0.31,   0.50);
	ADD(jupiter,  1,-3,0, -2.82,    1.45,     -2.53, -5.73,    0.01,   0.07);
	ADD(jupiter,  1,-4,0, -0.31,    0.28,     -0.34, -0.51,    0.00,   0.00);
	ADD(jupiter,  2,-1,0,  2.15,   -2.29,      7.04,  6.94,    0.33,   0.19);
	ADD(jupiter,  2,-2,0,-15.69,    3.31,    -15.70,-73.17,   -0.17,  -0.25);
	ADD(jupiter,  2,-3,0, -1.73,    1.95,     -9.19, -7.20,    0.02,  -0.03);
	ADD(jupiter,  2,-4,0, -0.01,    0.33,     -1.42,  0.08,    0.01,  -0.01);
	ADD(jupiter,  2,-5,0,  0.03,    0.03,     -0.13,  0.12,    0.00,   0.00);
	ADD(jupiter,  3,-1,0,  0.26,   -0.28,      0.73,  0.71,    0.08,   0.04);
	ADD(jupiter,  3,-2,0, -2.06,    0.46,     -1.61, -6.72,   -0.13,  -0.25);
	ADD(jupiter,  3,-3,0, -1.28,   -0.27,      2.21, -6.90,   -0.04,  -0.02);
	ADD(jupiter,  3,-4,0, -0.22,    0.08,     -0.44, -1.25,    0.00,   0.01);
	ADD(jupiter,  3,-5,0, -0.02,    0.03,     -0.15, -0.08,    0.00,   0.00);
	ADD(jupiter,  4,-1,0,  0.03,   -0.03,      0.08,  0.08,    0.01,   0.01);
	ADD(jupiter,  4,-2,0, -0.26,    0.06,     -0.17, -0.70,   -0.03,  -0.05);
	ADD(jupiter,  4,-3,0, -0.20,   -0.05,      0.22, -0.79,   -0.01,  -0.02);
	ADD(jupiter,  4,-4,0, -0.11,   -0.14,      0.93, -0.60,    0.00,   0.00);
	ADD(jupiter,  4,-5,0, -0.04,   -0.02,      0.09, -0.23,    0.00,   0.00);
	ADD(jupiter,  5,-4,0, -0.02,   -0.03,      0.13, -0.09,    0.00,   0.00);
	ADD(jupiter,  5,-5,0,  0.00,   -0.03,      0.21,  0.01,    0.00,   0.00);
	add(jupiter);

	saturn = PerturbationSeriesPtr(new PerturbationSeries(*this, Saturn()));
	saturn->perturber().M0(Angle(0.8811167, Angle::Revolutions));
	saturn->perturber().n(Angle(3.3935250, Angle::Revolutions));
	ADD(saturn, -1,-1,0,  0.03,    0.13,      0.48, -0.13,    0.02,   0.00);
	ADD(saturn,  0,-1,0,  0.27,    0.84,      0.40, -0.43,    0.01,  -0.01);
	ADD(saturn,  0,-2,0,  0.12,   -0.04,     -0.33, -0.55,   -0.01,  -0.02);
	ADD(saturn,  0,-3,0,  0.02,   -0.01,     -0.07, -0.08,    0.00,   0.00);
	ADD(saturn,  1,-1,0,  1.12,    0.76,     -2.66,  3.91,   -0.01,   0.01);
	ADD(saturn,  1,-2,0,  1.49,   -0.95,      3.07,  4.83,    0.04,  -0.05);
	ADD(saturn,  1,-3,0,  0.21,   -0.18,      0.55,  0.64,    0.00,   0.00);
	ADD(saturn,  2,-1,0,  0.12,    0.10,     -0.29,  0.34,   -0.01,   0.02);
	ADD(saturn,  2,-2,0,  0.51,   -0.36,      1.61,  2.25,    0.03,   0.01);
	ADD(saturn,  2,-3,0,  0.10,   -0.10,      0.50,  0.43,    0.00,   0.00);
	ADD(saturn,  2,-4,0,  0.01,   -0.02,      0.11,  0.05,    0.00,   0.00);
	ADD(saturn,  3,-2,0,  0.07,   -0.05,      0.16,  0.22,    0.01,   0.01);
	add(saturn);

}

/**
 * \brief Compute the position of Mars
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	MarsPerturbed::position(const JulianCenturies& T) const {
	Angle	lt0(6615.5, Angle::ArcSeconds);
	Angle	lt1(1.1, Angle::ArcSeconds);
	Angle	l = Angle(0.9334591 , Angle::Revolutions) + M(T) + (lt0 + lt1 * T) * T;
	double	r = 1.5303352 + 0.0000131 * T;
	Angle	bt0(596.32, Angle::ArcSeconds);
	Angle	bt1(-2.92, Angle::ArcSeconds);
	Angle	bt2(-0.10, Angle::ArcSeconds);
	Angle	b = bt0 + (bt1 + bt2 * T) * T;
	return EclipticalCoordinates(l, r, b);
}

/**
 * \brief Additional Corrections to the orbit of Mars
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	MarsPerturbed::corrections(const JulianCenturies& T) const {
	Angle	dl = Angle(52.49, Angle::ArcSeconds)
			* sin(Angle(0.1868, Angle::Revolutions)
				+ Angle(0.0549, Angle::Revolutions) * T);
        dl = dl + Angle(0.61, Angle::ArcSeconds)
			* sin(Angle(0.9220, Angle::Revolutions)
				+ Angle(0.3307, Angle::Revolutions) * T);
        dl = dl + Angle(0.32, Angle::ArcSeconds)
			* sin(Angle(0.4731, Angle::Revolutions)
				+ Angle(2.1485, Angle::Revolutions) * T);
        dl = dl + Angle(0.28, Angle::ArcSeconds)
			* sin(Angle(0.9467, Angle::Revolutions)
				+ Angle(0.1133, Angle::Revolutions) * T);
	dl = dl + Angle(0.14, Angle::ArcSeconds)
		+ (Angle(0.87, Angle::ArcSeconds)
			- Angle(0.11, Angle::ArcSeconds) * T) * T;
	return EclipticalCoordinates(dl, 0, Angle(0));
}

/**
 * \brief Set up perturbation series coefficients for Jupiter
 */
JupiterPerturbed::JupiterPerturbed() : PerturbedPlanetoid(Jupiter()) {
	M0(Angle(0.0565314, Angle::Revolutions));
	n(Angle(8.4302963, Angle::Revolutions));

	// perturbation series for Saturn perturbations
	saturn = PerturbationSeriesPtr(new PerturbationSeries(*this, Saturn()));
	saturn->perturber().M0(Angle(0.8829867, Angle::Revolutions));
	saturn->perturber().n(Angle(3.3947688, Angle::Revolutions));
	ADD(saturn,-1, -1,0,  -0.2,    1.4,     20.,    6.,    0.1, -0.2);
	ADD(saturn, 0, -1,0,   9.4,    8.9,     39.,  -83.,   -0.4, -1.4);
	ADD(saturn, 0, -2,0,   5.6,   -3.0,    -54.,  -57.,   -2.0,  0.0);
	ADD(saturn, 0, -3,0,  -4.0,   -0.1,      0.,   55.,    0.0,  0.0);
	ADD(saturn, 0, -5,0,   3.3,   -1.6,    -16.,  -31.,   -0.5, -1.2);
	ADD(saturn, 1,  0,0,-113.1,19998.6,-252082.,-1422.,-4670.7,288.9);
	ADD(saturn, 1,  0,1, -76.1,   66.9,   -842., -958.,   21.6, 29.4);
	ADD(saturn, 1,  0,2,  -0.5,   -0.3,      4.,  -07.,    0.1, -0.1);
	ADD(saturn, 1, -1,0,  78.8,  -14.5,    115.,  644.,   -0.2,  0.2);
	ADD(saturn, 1, -2,0,  -2.0, -132.4,    288.,   43.,   -1.7,  0.4);
	ADD(saturn, 1, -2,1,  -1.1,   -0.7,      2.,   -3.,    0.0,  0.0);
	ADD(saturn, 1, -3,0,  -7.5,   -6.8,     -4.,  -11.,    0.6, -0.9);
	ADD(saturn, 1, -4,0,   0.7,    0.7,      6.,  -11.,    0.0, -0.2);
	ADD(saturn, 1, -5,0,  51.5,  -26.0,   -325., -644.,   -4.9,-12.4);
	ADD(saturn, 1, -5,1,  -1.2,   -2.2,    -27.,   15.,   -0.4,  0.3);
	ADD(saturn, 2,  0,0,  -3.4,  632.0,  -6106.,  -65., -226.8, 12.7);
	ADD(saturn, 2,  0,1,  -4.2,    3.8,    -41.,  -45.,    0.2,  0.6);
	ADD(saturn, 2, -1,0,   5.3,   -0.7,      7.,   61.,    0.2,  1.1);
	ADD(saturn, 2, -2,0, -76.4, -185.1,   2602.,-1080.,    1.6,  0.0);
	ADD(saturn, 2, -3,0,  66.7,   47.8,   -514.,  698.,    0.9,  0.3);
	ADD(saturn, 2, -3,1,   0.6,   -1.0,     10.,    6.,    0.0,  0.0);
	ADD(saturn, 2, -4,0,  17.0,    1.4,    -18.,   96.,    0.0, -0.1);
	ADD(saturn, 2, -5,0,1066.2, -518.3,    -13., -239.,    1.8, -0.3);
	ADD(saturn, 2, -5,1, -25.4,  -40.3,     -9.,    3.,    0.0,  0.0);
	ADD(saturn, 2, -5,2,  -0.7,    0.5,      0.,    0.,    0.0,  0.0);
	ADD(saturn, 3,  0,0,  -0.1,   28.0,   -221.,   -2.,  -12.5,  0.7);
	ADD(saturn, 3, -2,0,  -5.0,  -11.5,    117.,  -54.,    2.1, -1.0);
	ADD(saturn, 3, -3,0,  16.9,   -6.4,    134.,  269.,   -0.5,  0.8);
	ADD(saturn, 3, -4,0,   7.2,  -13.3,    209.,  105.,    0.1, -0.1);
	ADD(saturn, 3, -5,0,  68.5,  134.3,  -1669.,  865.,    7.1, 15.2);
	ADD(saturn, 3, -5,1,   3.5,   -2.7,     34.,   43.,    0.5, -0.4);
	ADD(saturn, 3, -6,0,   0.6,    1.0,     -9.,    5.,    0.0,  0.0);
	ADD(saturn, 3, -7,0,  -1.1,    1.7,     -4.,   -2.,    0.0,  0.0);
	ADD(saturn, 4,  0,0,   0.0,    1.4,    -10.,    0.,   -0.6,  0.0);
	ADD(saturn, 4, -2,0,  -0.3,   -0.7,      4.,   -2.,    0.2, -0.1);
	ADD(saturn, 4, -3,0,   1.1,   -0.6,      9.,   12.,    0.1,  0.2);
	ADD(saturn, 4, -4,0,   3.2,    1.7,    -41.,   58.,    0.2,  0.1);
	ADD(saturn, 4, -5,0,   6.7,    8.7,    -93.,   87.,   -1.1,  1.6);
	ADD(saturn, 4, -6,0,   1.5,   -0.3,      6.,   24.,    0.0,  0.0);
	ADD(saturn, 4, -7,0,  -1.9,    2.3,    -32.,  -27.,    0.0, -0.1);
	ADD(saturn, 4, -8,0,   0.4,   -1.8,     19.,    5.,    0.0,  0.0);
	ADD(saturn, 4, -9,0,  -0.2,   -0.5,      3.,   -1.,    0.0,  0.0);
	ADD(saturn, 4,-10,0,  -8.6,   -6.8,     -4.,    1.,    0.0,  0.0);
	ADD(saturn, 4,-10,1,  -0.5,    0.6,      0.,    0.,    0.0,  0.0);
	ADD(saturn, 5, -5,0,  -0.1,    1.5,    -25.,   -8.,   -0.1,  0.1);
	ADD(saturn, 5, -6,0,   0.1,    0.8,    -16.,    1.,    0.0,  0.0);
	ADD(saturn, 5, -9,0,  -0.5,   -0.1,      1.,   -8.,    0.0,  0.0);
	ADD(saturn, 5,-10,0,   2.5,   -2.2,     28.,   31.,    0.1, -0.2);
	add(saturn);

	// perturbation series for Uranus perturbations
	uranus = PerturbationSeriesPtr(new PerturbationSeries(*this, Uranus()));
	uranus->perturber().M0(Angle(0.3969537, Angle::Revolutions));
	uranus->perturber().n(Angle(1.1902586, Angle::Revolutions));
	ADD(uranus, 1, -1, 0, 0.4, 0.9,  0., 0., 0.0, 0.0);
	ADD(uranus, 1, -2, 0, 0.4, 0.4, -4., 3., 0.0, 0.0);
	add(uranus);
}

/**
 * \brief Determine the position of Jupiter
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	JupiterPerturbed::position(const JulianCenturies& T) const {
	Angle	lt0(5025.2, Angle::ArcSeconds);
	Angle	lt1(0.8, Angle::ArcSeconds);
	Angle	l = Angle(0.0388910 , Angle::Revolutions) + M(T) + (lt0 + lt1 * T) * T;
	double	r = 5.208873 + 0.000041 * T;
	Angle	bt0(227.3, Angle::ArcSeconds);
	Angle	bt1(-0.3, Angle::ArcSeconds);
	Angle	b = bt0 + bt1 * T;
	return EclipticalCoordinates(l, r, b);
}

/**
 * \brief Additional Corrections to the orbit of Jupiter
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	JupiterPerturbed::corrections(const JulianCenturies& T) const {
	Angle	M5 = M(T);
	Angle	M6 = saturn->perturber().M(T);
	Angle	M7 = uranus->perturber().M(T);

	Angle	phi = 2 * M5 - 6 * M6 + 3 * M7;
	double	c = cos(phi);
	double	s = sin(phi);

	Angle	dl = Angle(-0.8, Angle::ArcSeconds) * c
			+ Angle(8.5, Angle::ArcSeconds) * s;
	double	dr = -0.1 * c;

	phi = 3 * M5 - 6 * M6 + 3 * M7;
	c = cos(phi);
	s = sin(phi);

	dl = dl + Angle(0.4, Angle::ArcSeconds) * c + Angle(0.5, Angle::ArcSeconds) * s;
	dr = dr + -0.7 * c + 0.5 * s;
	Angle	db = Angle(-0.1, Angle::ArcSeconds) * c;

	return EclipticalCoordinates(dl, dr * 1e-5, db);
}

/**
 * \brief Set up perturbation series coefficients for Saturn
 */
SaturnPerturbed::SaturnPerturbed() : PerturbedPlanetoid(Saturn()) {
	M0(Angle(0.8829867, Angle::Revolutions));
	n(Angle(3.3947688, Angle::Revolutions));

	jupiter = PerturbationSeriesPtr(new PerturbationSeries(*this, Jupiter()));
	jupiter->perturber().M0(Angle(0.0565314, Angle::Revolutions));
	jupiter->perturber().n(Angle(8.4302963, Angle::Revolutions));
	ADD(jupiter, 0,-1,0,   12.0,   -1.4,   -139.,    64.,    1.2,  -1.8);
	ADD(jupiter, 0,-2,0,    0.0,   -0.2,    - 9.,    10.,    0.0,  -0.1);
	ADD(jupiter, 1, 1,0,    0.9,    0.4,    -18.,    19.,    0.2,   0.2);
	ADD(jupiter, 1, 0,0, -348.3,22907.7,-529155., -7522.,-3266.5,8314.4);
	ADD(jupiter, 1, 0,1, -225.2, -146.2,   3377., -5213.,   79.6,  17.4);
	ADD(jupiter, 1, 0,2,    1.3,   -1.4,     32.,    29.,    0.1,  -0.4);
	ADD(jupiter, 1,-1,0,   -1.0,  -30.7,   1086., -8150.,   -3.6,  -9.3);
	ADD(jupiter, 1,-2,0,   -2.0,   -2.7,    -21.,  -119.,   -0.1,  -0.4);
	ADD(jupiter, 2, 1,0,    0.1,    0.2,    -10.,     3.,    0.0,   0.0);
	ADD(jupiter, 2, 0,0,   44.2,  724.0, -14643.,  -347., -188.7, 459.1);
	ADD(jupiter, 2, 0,1,  -17.0,  -11.3,    189.,  -286.,    1.0,  -3.7);
	ADD(jupiter, 2,-1,0,   -3.5, -426.6,  -5465.,  -265.,   -1.6,  -2.7);
	ADD(jupiter, 2,-1,1,    3.5,   -2.2,    -26.,   -43.,    0.0,   0.0);
	ADD(jupiter, 2,-2,0,   10.5,  -30.9,  -1305.,  -523.,   -1.9,   0.2);
	ADD(jupiter, 2,-3,0,   -0.2,   -0.4,    -12.,    -1.,   -0.1,   0.0);
	ADD(jupiter, 3, 0,0,    6.5,   30.5,   -611.,     4.,  -11.6,  28.1);
	ADD(jupiter, 3, 0,1,   -1.2,   -0.7,     11.,   -18.,   -0.2,  -0.6);
	ADD(jupiter, 3,-1,0,   29.0,  -40.2,    982.,   453.,    3.2,  -9.4);
	ADD(jupiter, 3,-1,1,    0.6,    0.6,    -10.,    13.,    0.0,   0.0);
	ADD(jupiter, 3,-2,0,  -27.0,  -21.1,   -685.,    81.,  -19.8,   5.4);
	ADD(jupiter, 3,-2,1,    0.9,   -0.5,     -4.,   -20.,   -0.1,  -0.8);
	ADD(jupiter, 3,-3,0,   -5.4,   -4.1,   -191.,   262.,   -0.1,  -0.1);
	ADD(jupiter, 4, 0,0,    0.6,    1.4,    -30.,    -2.,   -0.6,   1.6);
	ADD(jupiter, 4,-1,0,    1.5,   -2.5,    124.,    47.,    1.0,  -1.1);
	ADD(jupiter, 4,-2,0, -821.9,   -9.6,   -260., 18736.,  -70.5,  -4.4);
	ADD(jupiter, 4,-2,1,    4.1,  -21.9,   -503.,   -99.,    0.7,  -3.0);
	ADD(jupiter, 4,-3,0,   -2.0,   -4.7,   -193.,    82.,   -0.1,  -0.3);
	ADD(jupiter, 4,-4,0,   -1.5,    1.3,     65.,    73.,    0.0,   0.0);
	ADD(jupiter, 5,-2,0,-2627.6,-1277.3,   1174., -3441.,  -13.8,  -4.3);
	ADD(jupiter, 5,-2,1,   63.0,  -98.6,    127.,    67.,    0.1,  -0.2);
	ADD(jupiter, 5,-2,2,    1.7,    1.2,     -2.,     3.,    0.0,   0.0);
	ADD(jupiter, 5,-3,0,    0.4,   -3.6,   -113.,   -16.,    0.0,  -0.3);
	ADD(jupiter, 5,-4,0,   -1.4,    0.3,     15.,    63.,   -0.1,   0.0);
	ADD(jupiter, 5,-5,0,    0.3,    0.6,     30.,   -17.,    0.0,   0.0);
	ADD(jupiter, 6,-2,0, -146.7,  -73.7,   1664., -3343.,  -43.6, -46.7);
	ADD(jupiter, 6,-2,1,    5.2,   -6.8,    151.,   114.,    1.7,  -1.0);
	ADD(jupiter, 6,-3,0,    1.5,   -2.9,    -22.,   -13.,    0.1,  -0.1);
	ADD(jupiter, 6,-4,0,   -0.7,   -0.2,     -7.,    28.,    0.0,   0.0);
	ADD(jupiter, 6,-5,0,    0.0,    0.5,     25.,    -1.,    0.0,   0.0);
	ADD(jupiter, 6,-6,0,    0.3,   -0.1,     -3.,   -12.,    0.0,   0.0);
	ADD(jupiter, 7,-2,0,   -9.6,   -3.9,     96.,  -186.,   -4.7,  -5.3);
	ADD(jupiter, 7,-2,1,    0.4,   -0.5,     10.,     9.,    0.3,  -0.1);
	ADD(jupiter, 7,-3,0,    3.0,    5.3,     75.,   -35.,    0.0,   0.0);
	ADD(jupiter, 7,-4,0,    0.2,    0.4,     16.,   -13.,    0.0,   0.0);
	ADD(jupiter, 7,-5,0,   -0.1,    0.2,     10.,     5.,    0.0,   0.0);
	ADD(jupiter, 7,-6,0,    0.2,    0.0,      2.,   -10.,    0.0,   0.0);
	ADD(jupiter, 8,-2,0,   -0.7,   -0.2,      6.,   -12.,   -0.4,  -0.4);
	ADD(jupiter, 8,-3,0,    0.5,    1.0,    -20.,    15.,    0.1,   0.2);
	ADD(jupiter, 8,-4,0,    0.4,    1.3,     36.,    -9.,    0.0,  -0.1);
	ADD(jupiter, 9,-4,0,    4.0,   -8.7,   -199.,   -99.,    0.2,  -0.4);
	ADD(jupiter, 9,-4,1,    0.5,    0.3,      8.,   -18.,    0.0,   0.0);
	ADD(jupiter,10,-4,0,   21.3,  -16.8,     33.,    33.,    0.2,  -0.2);
	ADD(jupiter,10,-4,1,    1.0,    1.7,     -4.,     4.,    0.0,   0.0);
	ADD(jupiter,11,-4,0,    1.6,   -1.3,     30.,    37.,    0.8,  -0.2);
	add(jupiter);

	uranus = PerturbationSeriesPtr(new PerturbationSeries(*this, Uranus()));
	uranus->perturber().M0(Angle(0.3969537, Angle::Revolutions));
	uranus->perturber().n(Angle(1.1902586, Angle::Revolutions));
	ADD(uranus,  0,-1,0,    1.0,    0.7,      4.,   -15.,    0.1,   0.0);
	ADD(uranus,  0,-2,0,    0.0,   -0.4,    -11.,     1.,   -0.1,  -0.1);
	ADD(uranus,  0,-3,0,   -0.9,   -1.2,    -27.,    21.,   -0.5,  -0.3);
	ADD(uranus,  1,-1,0,    7.8,   -1.5,     23.,   127.,    0.0,   0.0);
	ADD(uranus,  1,-2,0,   -1.1,   -8.1,     52.,    -3.,   -0.3,  -0.3);
	ADD(uranus,  1,-3,0,  -16.4,  -21.0,    -21.,     0.,    0.4,   0.0);
	ADD(uranus,  2,-1,0,    0.6,   -0.1,      1.,    12.,    0.1,   0.0);
	ADD(uranus,  2,-2,0,   -4.9,  -11.7,    315.,  -133.,    0.0,  -0.2);
	ADD(uranus,  2,-3,0,   19.1,   10.0,   -221.,   421.,    0.1,  -1.1);
	ADD(uranus,  2,-4,0,    0.9,   -0.1,      1.,    14.,    0.0,   0.0);
	ADD(uranus,  3,-2,0,   -0.4,   -0.9,     17.,    -8.,    0.0,  -0.3);
	ADD(uranus,  3,-3,0,    2.3,    0.0,     10.,    57.,    0.3,   0.3);
	ADD(uranus,  3,-4,0,    0.3,   -0.7,     20.,     7.,    0.0,   0.0);
	ADD(uranus,  3,-5,0,   -0.1,   -0.4,     11.,   - 3.,    0.0,   0.0);
	add(uranus);

	neptune = PerturbationSeriesPtr(new PerturbationSeries(*this, Neptune()));
	neptune->perturber().M0(Angle(0.7208473, Angle::Revolutions));
	neptune->perturber().n(Angle(0.6068623, Angle::Revolutions));
	ADD(neptune,  1,-1,0,   -1.3,   -1.2,     23.,   -25.,    0.0,   0.0);
	ADD(neptune,  1,-2,0,    1.0,   -0.1,      1.,    14.,    0.0,   0.0);
	ADD(neptune,  2,-2,0,    1.1,   -0.1,      2.,    33.,    0.0,   0.0);
	add(neptune);
}

/**
 * \brief Determine the position of Saturn
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	SaturnPerturbed::position(const JulianCenturies& T) const {
	Angle	lt0(5018.6, Angle::ArcSeconds);
	Angle	lt1(1.9, Angle::ArcSeconds);
	Angle	l = Angle(0.2561136 , Angle::Revolutions) + M(T) + (lt0 + lt1 * T) * T;
	double	r = 9.557584 - 0.000186 * T;
	Angle	bt0(175.1, Angle::ArcSeconds);
	Angle	bt1(-10.2, Angle::ArcSeconds);
	Angle	b = bt0 + bt1 * T;
	return EclipticalCoordinates(l, r, b);
}

/**
 * \brief Additional Corrections to the orbit of Saturn
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	SaturnPerturbed::corrections(const JulianCenturies& T) const {
	Angle	M5 = jupiter->perturber().M(T);
	Angle	M6 = M(T);
	Angle	M7 = uranus->perturber().M(T);

	Angle	phi = -2 * M5 + 5 * M6 - 3 * M7;
	double	c = cos(phi);
	double	s = sin(phi);

	Angle	dl = Angle(-0.8, Angle::ArcSeconds) * c
			+ Angle(-0.1, Angle::ArcSeconds) * s;
	double	dr = -0.2 * c + 1.8 * s;
	Angle	db = Angle(0.3, Angle::ArcSeconds) * c
			+ Angle(0.5, Angle::ArcSeconds) * s;

	phi = -2 * M5 + 6 * M6 - 3 * M7;
	c = cos(phi);
	s = sin(phi);
	dl = dl + (Angle(2.4, Angle::ArcSeconds)
			- Angle(0.7, Angle::ArcSeconds) * T) * c
		+ (Angle(27.8, Angle::ArcSeconds)
			- Angle(0.4, Angle::ArcSeconds) * T) * s;
	dr = 2.1 * c - 0.2 * s;

	phi = -2 * M5 + 7 * M6 - 3 * M7;
	c = cos(phi);
	s = sin(phi);
	dl = dl + Angle(0.1, Angle::ArcSeconds) * c
		+ Angle(1.6, Angle::ArcSeconds) * s;
	dr = dr - 3.6 * c + 0.3 * s;
	db = db + Angle(-0.2, Angle::ArcSeconds) * c
		+ Angle(0.6, Angle::ArcSeconds) * s;

	return EclipticalCoordinates(dl, dr * 1e-5, db);
}

/**
 * \brief Set up perturbation series coefficients for Uranus
 */
UranusPerturbed::UranusPerturbed() : PerturbedPlanetoid(Uranus()) {
	M0(Angle(0.3967117, Angle::Revolutions));
	n(Angle(1.1902849, Angle::Revolutions));

	jupiter = PerturbationSeriesPtr(new PerturbationSeries(*this, Jupiter()));
	jupiter->perturber().M0(Angle(0.0564472, Angle::Revolutions));
	jupiter->perturber().n(Angle(8.4302889, Angle::Revolutions));
	ADD(jupiter, -1,-1,0,  0.0,    0.0,     -1.,   17.,  -0.1,   0.0);
	ADD(jupiter,  0,-1,0,  0.5,   -1.2,    189.,   91.,  -0.9,   0.1);
	ADD(jupiter,  1,-1,0,-21.2,   48.7,  -4555.,-1988.,   0.0,   0.0);
	ADD(jupiter,  1,-2,0, -0.5,    1.2,   -109.,  -48.,   0.0,   0.0);
	ADD(jupiter,  2,-1,0, -1.3,    3.2,   -232., -111.,   0.3,   0.1);
	ADD(jupiter,  2,-2,0, -0.2,    0.2,     11.,   15.,   0.0,   0.0);
	ADD(jupiter,  3,-1,0,  0.0,    0.2,    -18.,    4.,   0.0,   0.0);
	add(jupiter);

	saturn = PerturbationSeriesPtr(new PerturbationSeries(*this, Saturn()));
	saturn->perturber().M0(Angle(0.8829611, Angle::Revolutions));
	saturn->perturber().n(Angle(3.3947583, Angle::Revolutions));
	ADD(saturn,  0,-1,0,  1.4,   -0.5,    -64.,   90.,  -0.4,  -0.8);
	ADD(saturn,  1,-1,0,-18.6,  -12.6,    367.,-3368.,   1.0,   0.3);
	ADD(saturn,  1,-2,0, -0.7,   -0.3,      5.,  -75.,   0.1,   0.0);
	ADD(saturn,  2,-1,0, 20.0, -141.6,  -5871.,-1070.,   3.1,  -0.8);
	ADD(saturn,  2,-1,1,  1.0,    1.4,     58.,  -40.,   0.0,   0.0);
	ADD(saturn,  2,-2,0,  1.6,   -3.8,   -356., -160.,   0.0,   0.0);
	ADD(saturn,  3,-1,0, 75.3, -100.9,   1289.,  775.,  -0.8,   0.1);
	ADD(saturn,  3,-1,1,  0.2,    1.8,    -19.,    3.,   0.0,   0.0);
	ADD(saturn,  3,-2,0,  2.3,   -1.3,    -95., -179.,   0.0,   0.1);
	ADD(saturn,  3,-3,0, -0.7,   -0.5,    -49.,   68.,   0.0,   0.0);
	ADD(saturn,  4,-1,0,  3.4,   -5.0,    216.,  143.,  -0.8,  -0.5);
	ADD(saturn,  4,-2,0,  1.9,    0.1,     12., -121.,   0.0,   0.0);
	ADD(saturn,  4,-3,0, -0.1,   -0.4,    -39.,   12.,   0.0,   0.0);
	ADD(saturn,  4,-4,0, -0.2,    0.1,     16.,   18.,   0.0,   0.0);
	ADD(saturn,  5,-1,0,  0.2,   -0.3,     10.,    6.,  -0.1,   0.0);
	ADD(saturn,  5,-2,0, -2.2,   -2.2,    -77.,   85.,   0.0,   0.0);
	ADD(saturn,  5,-3,0,  0.1,   -0.2,    -14.,   -4.,   0.0,   0.0);
	ADD(saturn,  5,-4,0, -0.1,    0.0,      1.,   12.,   0.0,   0.0);
	ADD(saturn,  6,-2,0, -0.2,   -0.6,     14.,   -7.,   0.0,   0.0);
	add(saturn);

	neptune = PerturbationSeriesPtr(new PerturbationSeries(*this, Neptune()));
	neptune->perturber().M0(Angle(0.7216833, Angle::Revolutions));
	neptune->perturber().n(Angle(0.6068528, Angle::Revolutions));
	ADD(neptune,  1, 0,0,-78.1,19518.1,-907182.,-3347.,2759.5,-311.9);
	ADD(neptune,  1, 0,1,-81.6,  107.7,  -4974.,-3795.,  -2.8, -43.7);
	ADD(neptune,  1, 0,2, -6.6,   -3.1,    144., -306.,  -0.4,  -0.5);
	ADD(neptune,  1, 0,3,  0.0,   -0.5,     24.,    0.,   0.0,   0.0);
	ADD(neptune,  2, 0,0, -2.4,  586.1, -21452., -153., 130.6, -14.3);
	ADD(neptune,  2, 0,1, -4.5,    6.6,   -242., -178.,   0.7,  -1.6);
	ADD(neptune,  2, 0,2, -0.4,    0.0,      1.,  -14.,   0.0,   0.0);
	ADD(neptune,  3, 0,0,  0.0,   24.5,   -762.,   -6.,   7.0,  -0.7);
	ADD(neptune,  3, 0,1, -0.2,    0.4,    -14.,   -8.,   0.1,  -0.1);
	ADD(neptune,  4, 0,0,  0.0,    1.1,    -30.,    1.,   0.4,   0.0);
	ADD(neptune, -1,-1,0, -0.2,    0.2,      7.,    7.,  -0.1,   0.0);
	ADD(neptune,  0,-1,0, -2.8,    2.5,     87.,  105.,  -0.4,  -0.1);
	ADD(neptune,  1,-1,0,-28.4,   20.3,   -514., -720.,   0.0,   0.0);
	ADD(neptune,  1,-2,0, -0.6,   -0.1,     42., -146.,   0.2,   0.4);
	ADD(neptune,  1,-3,0,  0.2,    0.5,     34.,  -16.,  -0.1,   0.1);
	ADD(neptune,  2,-1,0, -1.8,    1.3,    -55.,  -77.,   0.0,   0.3);
	ADD(neptune,  2,-2,0, 29.4,   10.2,   -290.,  832.,   0.0,   0.0);
	ADD(neptune,  2,-3,0,  8.8,   17.8,   -419.,  215.,  -0.1,  -0.3);
	ADD(neptune,  2,-4,0,  0.0,    0.1,    -21.,   -9.,   0.1,   0.0);
	ADD(neptune,  3,-2,0,  1.5,    0.5,    -17.,   51.,   0.1,  -0.2);
	ADD(neptune,  3,-3,0,  4.4,   14.6,   -843.,  252.,   0.1,  -0.1);
	ADD(neptune,  3,-4,0,  2.4,   -4.5,    120.,   62.,   0.0,   0.0);
	ADD(neptune,  3,-5,0,  2.9,   -0.9,     21.,   62.,   0.0,   0.0);
	ADD(neptune,  4,-3,0,  0.3,    1.0,    -40.,   11.,   0.1,  -0.1);
	ADD(neptune,  4,-4,0,  2.1,   -2.7,    179.,  140.,   0.0,   0.0);
	ADD(neptune,  4,-5,0,  3.0,   -0.4,     23.,  176.,  -0.1,  -0.1);
	ADD(neptune,  4,-6,0, -0.6,   -0.5,     11.,  -16.,   0.0,   0.0);
	ADD(neptune,  5,-4,0,  0.2,   -0.2,     10.,    8.,   0.0,   0.0);
	ADD(neptune,  5,-5,0, -0.9,   -0.1,     06.,  -71.,   0.0,   0.0);
	ADD(neptune,  5,-6,0, -0.5,   -0.6,     38.,  -36.,   0.0,   0.0);
	ADD(neptune,  5,-7,0,  0.0,   -0.5,     30.,    1.,   0.0,   0.0);
	ADD(neptune,  6,-6,0,  0.2,    0.3,    -27.,   16.,   0.0,   0.0);
	ADD(neptune,  6,-7,0, -0.1,    0.2,    -20.,   -4.,   0.0,   0.0);
	ADD(neptune,  7,-7,0,  0.1,   -0.2,     13.,    5.,   0.0,   0.0);
	ADD(neptune,  7,-8,0,  0.1,    0.0,      4.,    9.,   0.0,   0.0);
	add(neptune);

	jsu = PerturbationSeriesPtr(new PerturbationSeries(*this, Saturn()));
	jsu->perturber().M0(Angle(0.8829611, Angle::Revolutions));
	jsu->perturber().n(Angle(3.3947583, Angle::Revolutions));
	ADD(jsu, -2,-4,0, -0.7,    0.4,    -15.,  -25.,   0.0,   0.0);
	ADD(jsu, -1,-4,0, -0.1,   -0.1,    -22.,   10.,   0.0,   0.0);
	ADD(jsu,  1,-5,0,  0.1,   -0.4,     14.,    2.,   0.0,   0.0);
	ADD(jsu,  1,-6,0,  0.4,    0.5,     -8.,   -8.,   0.0,   0.0);
	ADD(jsu,  2,-6,0,  5.7,    6.3,    285., -255.,   0.0,   0.0);
	ADD(jsu,  2,-6,1,  0.1,   -0.2,    -11.,   -6.,   0.0,   0.0);
	ADD(jsu,  3,-6,0, -1.4,   29.2,   -114.,   11.,   0.0,   0.0);
	ADD(jsu,  3,-6,1,  0.8,   -0.4,      2.,    3.,   0.0,   0.0);
	ADD(jsu,  4,-6,0,  0.0,    1.3,    -60.,   -1.,   0.0,   0.0);
	add(jsu);
}

/**
 * \brief Determine the position of Uranus
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	UranusPerturbed::position(const JulianCenturies& T) const {
	Angle	lt0(5082.3, Angle::ArcSeconds);
	Angle	lt1(34.2, Angle::ArcSeconds);
	Angle	l = Angle(0.4734843 , Angle::Revolutions) + M(T) + (lt0 + lt1 * T) * T;
	double	r = 19.211991 + (-0.000333 - 0.000005 * T) * T;
	Angle	bt0(-130.61, Angle::ArcSeconds);
	Angle	bt1(-0.54, Angle::ArcSeconds);
	Angle	bt2(0.04, Angle::ArcSeconds);
	Angle	b = bt0 + (bt1 + bt2 * T) * T;
	return EclipticalCoordinates(l, r, b);
}

/**
 * \brief Compute the perturbations of Uranus
 *
 * The Uranus perturbations are special because they also need the relative
 * positions of Jupiter and Saturn.
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	UranusPerturbed::perturbations(const JulianCenturies& T) const {
	Angle	M5 = jupiter->perturber().M(T);
	jsu->perturber().phi0(2 * M5);
	return PerturbedPlanetoid::perturbations(T);
}

/**
 * \brief Set up perturbation series coefficients for Neptune
 */
NeptunePerturbed::NeptunePerturbed() : PerturbedPlanetoid(Neptune()) {
	M0(Angle(0.7214906, Angle::Revolutions));
	n(Angle(0.6068526, Angle::Revolutions));

	jupiter = PerturbationSeriesPtr(new PerturbationSeries(*this, Jupiter()));
	jupiter->perturber().M0(Angle(0.0563867, Angle::Revolutions));
	jupiter->perturber().n(Angle(8.4298907, Angle::Revolutions));
	ADD(jupiter,  0,-1,0,  0.1,   0.1,    -30.,   18.,   -0.3, -0.3);
	ADD(jupiter,  1, 0,0,  0.0,   0.0,   -159.,   90.,    0.0,  0.0);
	ADD(jupiter,  1,-1,0,-17.6, -29.3,   4161.,-2500.,    0.0,  0.0);
	ADD(jupiter,  1,-2,0, -0.4,  -0.7,    104.,  -62.,    0.0,  0.0);
	ADD(jupiter,  2,-1,0, -0.2,  -0.4,     24.,  -14.,    0.4, -0.3);
	add(jupiter);

	saturn = PerturbationSeriesPtr(new PerturbationSeries(*this, Saturn()));
	saturn->perturber().M0(Angle(0.8825086, Angle::Revolutions));
	saturn->perturber().n(Angle(3.3957748, Angle::Revolutions));
	ADD(saturn,  0,-1,0, -0.1,   0.0,      2.,  -18.,   -0.1, -0.5);
	ADD(saturn,  1, 0,0,  0.0,   0.0,    -83., -104.,    0.0,  0.0);
	ADD(saturn,  1,-1,0, 13.6, -12.7,   1875., 2011.,    0.0,  0.0);
	ADD(saturn,  1,-2,0,  0.4,  -0.4,     45.,   45.,    0.0,  0.0);
	ADD(saturn,  2,-1,0,  0.4,  -0.1,     17.,  -32.,    0.2,  0.2);
	ADD(saturn,  2,-2,0, -0.1,   0.0,     -2.,   27.,    0.0,  0.0);
	add(saturn);

	uranus = PerturbationSeriesPtr(new PerturbationSeries(*this, Uranus()));
	uranus->perturber().M0(Angle(0.3965358, Angle::Revolutions));
	uranus->perturber().n(Angle(1.1902851, Angle::Revolutions));
	ADD(uranus,  1, 0,0, 32.3,3549.5,-258802., 2358.,-6360.5,374.0);
	ADD(uranus,  1, 0,1, 31.2,  34.4,  -2514., 2274.,   34.9, 29.3);
	ADD(uranus,  1, 0,2, -1.4,   3.9,   -286., -101.,    0.0, -0.9);
	ADD(uranus,  2, 0,0,  6.1,  68.0,  -1114.,   20.,  -54.7,  3.7);
	ADD(uranus,  2, 0,1,  0.8,  -0.2,    -21.,   20.,   -0.2,  0.8);
	ADD(uranus,  3, 0,0,  0.1,   1.0,     -7.,    0.,   -0.8,  0.1);
	ADD(uranus,  0,-1,0, -0.1,  -0.3,    -36.,    0.,    0.0,  0.0);
	ADD(uranus,  1, 0,0,  0.0,   0.0,     55.,  -69.,    0.1,  0.0);
	ADD(uranus,  1,-1,0, -2.2,  -1.6,  -1163., 1636.,    0.0, -0.1);
	ADD(uranus,  1,-2,0,  0.2,   0.1,    -12.,    4.,    0.0, -0.1);
	ADD(uranus,  2,-1,0,  4.2,  -1.1,    -44., -346.,   -0.2,  0.1);
	ADD(uranus,  2,-2,0,  8.6,  -2.9,   -334., -970.,    0.2,  0.1);
	ADD(uranus,  3,-1,0,  0.1,  -0.2,     21.,  -12.,    0.0,  0.1);
	ADD(uranus,  3,-2,0, -4.6,   9.3,    382.,  198.,    0.1,  0.1);
	ADD(uranus,  3,-3,0, -0.5,   1.7,    235.,   70.,    0.0,  0.0);
	ADD(uranus,  4,-2,0,  0.2,   0.8,     33.,  -15.,   -0.2, -0.1);
	ADD(uranus,  4,-3,0,  0.9,   1.7,    179.,  -91.,   -0.1,  0.0);
	ADD(uranus,  4,-4,0, -0.4,  -0.4,    -62.,   48.,    0.0,  0.0);
	ADD(uranus,  5,-3,0, -1.6,  -0.5,    -22.,   70.,    0.0,  0.0);
	ADD(uranus,  5,-4,0, -0.4,  -0.1,    -07.,   55.,    0.0,  0.0);
	ADD(uranus,  5,-5,0,  0.2,   0.0,      0.,  -35.,    0.0,  0.0);
	ADD(uranus,  6,-4,0, -0.3,   0.2,     21.,   27.,    0.0,  0.0);
	ADD(uranus,  6,-5,0,  0.1,  -0.1,    -14.,  -14.,    0.0,  0.0);
	ADD(uranus,  6,-6,0, -0.1,   0.1,     14.,    7.,    0.0,  0.0);
	add(uranus);

}

/**
 * \brief Determine the position of Jupiter
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	NeptunePerturbed::position(const JulianCenturies& T) const {
	Angle	lt0(4982.8, Angle::ArcSeconds);
	Angle	lt1(-21.3, Angle::ArcSeconds);
	Angle	l = Angle(0.1254046 , Angle::Revolutions) + M(T) + (lt0 + lt1 * T) * T;
	double	r = 30.072984 + (0.001234 + 0.000003 * T) * T;
	Angle	bt0(54.77, Angle::ArcSeconds);
	Angle	bt1(0.26, Angle::ArcSeconds);
	Angle	bt2(0.06, Angle::ArcSeconds);
	Angle	b = bt0 + (bt1 + bt2 * T) * T;
	return EclipticalCoordinates(l, r, b);
}

/**
* \brief Set up perturbation series coefficients for Pluto
*/
PlutoPerturbed::PlutoPerturbed() : PerturbedPlanetoid(Pluto()) {
	M0(Angle(0.0385795, Angle::Revolutions));
	n(Angle(0.4026667, Angle::Revolutions));

	jupiter = PerturbationSeriesPtr(new PerturbationSeries(*this, Jupiter()));
	jupiter->perturber().M0(Angle(0.0565314, Angle::Revolutions));
	jupiter->perturber().n(Angle(8.4302963, Angle::Revolutions));
	ADD(jupiter,  1, 0,0,   0.06,100924.08,-9603960.,159651.,51987.68,-24288.76);
	ADD(jupiter,  2, 0,0,3274.74, 17835.12,-1182522., 36324.,12687.49, -6049.72);
	ADD(jupiter,  3, 0,0,1543.52,  4631.99, -214466., 11670., 3504.00, -1853.10);
	ADD(jupiter,  4, 0,0, 688.99,  1227.08,  -48234.,  2135., 1048.19,  -648.26);
	ADD(jupiter,  5, 0,0, 242.27,   415.93,  -10754.,  1406.,  302.33,  -209.76);
	ADD(jupiter,  6, 0,0, 138.41,   110.91,   -3088.,  -553.,  109.52,   -93.82);
	ADD(jupiter,  3,-1,0,  -0.99,     5.06,    -256.,   198.,    1.26,    -1.96);
	ADD(jupiter,  2,-1,0,   7.15,     5.61,    -967.,   572.,    1.64,    -2.16);
	ADD(jupiter,  1,-1,0,  10.79,    23.13,   -3904.,  2364.,   -0.33,     0.86);
	ADD(jupiter,  0, 1,0,  -0.23,     4.43,    1028.,   632.,    3.15,     0.34);
	ADD(jupiter,  1, 1,0,  -1.10,    -0.92,     118.,   -23.,    0.43,     0.14);
	ADD(jupiter,  2, 1,0,   0.62,     0.84,      23.,     7.,    0.05,    -0.04);
	ADD(jupiter,  3, 1,0,  -0.38,    -0.45,      12.,    -8.,    0.04,     0.05);
	ADD(jupiter,  4, 1,0,   0.17,     0.25,       0.,     2.,   -0.01,    -0.01);
	ADD(jupiter,  3,-2,0,   0.06,     0.07,      -6.,     3.,    0.03,    -0.03);
	ADD(jupiter,  2,-2,0,   0.13,     0.20,     -22.,    15.,    0.03,    -0.07);
	ADD(jupiter,  1,-2,0,   0.32,     0.49,     -94.,    57.,   -0.01,     0.03);
	ADD(jupiter,  0,-2,0,  -0.04,    -0.07,      26.,   -15.,    0.07,    -0.02);
	add(jupiter);

	saturn = PerturbationSeriesPtr(new PerturbationSeries(*this, Saturn()));
	saturn->perturber().M0(Angle(0.8829867, Angle::Revolutions));
	saturn->perturber().n(Angle(3.3947688, Angle::Revolutions));
	ADD(saturn,  1,-1,0, -29.47,    75.97,   -1064., -2049.,  -40.71,   -17.55);
	ADD(saturn,  0, 1,0, -13.88,    18.20,     426.,  -461.,    1.13,     0.43);
	ADD(saturn,  1, 1,0,   5.81,   -23.48,     150.,   -68.,   -7.48,     3.07);
	ADD(saturn,  2, 1,0, -10.27,    14.16,     -79.,     4.,    2.43,    -0.09);
	ADD(saturn,  3, 1,0,   6.86,   -10.66,      73.,    -3.,   -2.25,     0.69);
	ADD(saturn,  2,-2,0,   4.32,     2.00,       0.,   -22.,   -0.24,     0.12);
	ADD(saturn,  1,-2,0,  -5.04,    -0.83,     -92.,   -31.,    0.79,    -0.24);
	ADD(saturn,  0,-2,0,   4.25,     2.48,     -59.,   -33.,    0.58,     0.02);
	add(saturn);
}

/**
 * \brief Determine the position of Pluto
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	PlutoPerturbed::position(const JulianCenturies& T) const {
	Angle	l = Angle(0.6232469 , Angle::Revolutions) + M(T);
	double	r = 40.7247248;
  	Angle	b(-3.909434, Angle::Degrees);
	return EclipticalCoordinates(l, r, b);
}

/**
 * \brief Additional Corrections to the orbit of Pluto
 *
 * \param T	time in julian centuries
 */
EclipticalCoordinates	PlutoPerturbed::corrections(const JulianCenturies& T) const {
	Angle	M5 = jupiter->perturber().M(T);
	Angle	M6 = saturn->perturber().M(T);
	Angle	M9 = M(T);

	Angle	phi = M5 - M6;
	double	c = cos(phi);
	double	s = sin(phi);

	Angle	dl = Angle(-9.11, Angle::ArcSeconds) * c
			+ Angle(0.12, Angle::ArcSeconds) * s;
	double	dr = -3.4 * c - 3.3 * s;
	Angle	db = Angle(0.81, Angle::ArcSeconds) * c
			+ Angle(0.78, Angle::ArcSeconds) * s;

	phi = M5 - M6 + M9;
	c = cos(phi);
	s = sin(phi);

	dl = dl + Angle(5.92, Angle::ArcSeconds) * c
		+ Angle(0.25, Angle::ArcSeconds) * s;
	dr = dr + 2.3 * c - 3.8 * s;
	db = db + Angle(-0.67, Angle::ArcSeconds) * c
		- Angle(0.51, Angle::ArcSeconds) * s;

	return EclipticalCoordinates(dl, dr * 1e-5, db);
}

} // namespace solarsystem
} // namespace astro
