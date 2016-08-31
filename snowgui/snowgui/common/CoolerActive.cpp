/*
 * CoolerActive.pp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CoolerActive.h>
#include <QPainter>
#include <cmath>
#include <AstroDebug.h>

namespace snowgui {

CoolerActive::CoolerActive(QWidget *parent) : QWidget(parent) {
	_value = 0.0;
	_active = false;
}

CoolerActive::~CoolerActive() {
}

void	CoolerActive::setActive(bool b) {
	_active = b;
	draw();
}

void	CoolerActive::setValue(double v) {
	_value = v;
	draw();
}

static double	v = (2. / 30.);

typedef struct point_s { double x; double y; } point_t;
point_t	dir[6] = {
/* 0 */	{  v,      v * 0           },
/* 1 */	{  v / 2,  v * sqrt(3) / 2 },
/* 2 */	{ -v / 2,  v * sqrt(3) / 2 },
/* 3 */	{ -v,      v * 0           },
/* 4 */	{ -v / 2, -v * sqrt(3) / 2 },
/* 5 */	{  v / 2, -v * sqrt(3) / 2 }
};

point_t	outline[16] = {
/* 0 */	{ 1.                         , 0                                  },
/* 1 */	{ 1.    + dir[2].x           , 0            + dir[2].y            },
/* 2 */	{ 2./3. + dir[0].x + dir[1].x, 0            + dir[0].y + dir[1].y },
/* 3 */	{ 5./6. + dir[5].x           , sqrt(3) / 6  + dir[5].y            },
/* 4 */	{ 5./6.                      , sqrt(3) / 6                        },
/* 5 */	{ 5./6. + dir[3].x           , sqrt(3) / 6 + dir[3].y             },
/* 6 */	{ 2./3. + dir[2].x           , 0           + dir[2].y             },
/* 7 */	{ 1./3. + dir[0].x + dir[1].x, 0           + dir[0].y + dir[1].y  },
/* 8 */	{ 3./6. + dir[0].x + dir[1].x, sqrt(3) / 6 + dir[0].x + dir[0].y  },
/* 9 */	{ 1./6. + dir[0].x + dir[1].x, sqrt(3) / 6 + dir[0].y + dir[1].y  },
/*10 */	{ 1./3. + dir[5].x           , sqrt(3) / 3 + dir[5].y             },
/*11 */	{ 2./3. + dir[4].x           , sqrt(3) / 3 + dir[4].y             },
/*12 */	{ 2./3.                      , sqrt(3) / 3                        },
/*13 */	{ 2./3. + dir[2].x           , sqrt(3) / 3 + dir[2].y             },
/*14 */	{ 1./3. + dir[0].x + dir[1].x, sqrt(3) / 3 + dir[0].y + dir[1].y  },
/*15 */	{ 1./2. + dir[5].x           , sqrt(3) / 2 + dir[5].y             }
};

point_t	inside[4] = {
	{ 1./3. + dir[2].x,            0           + dir[2].y               },
	{ 1./2. + dir[3].x + dir[4].x, sqrt(3) / 6 + dir[3].y + dir[4].y    },
	{ 1./6. + dir[4].x,            sqrt(3) / 6 + dir[4].y               },
	{ 0     + dir[0].x + dir[1].x, 0           + dir[0].y + dir[1].y    }
};

void	CoolerActive::draw() {
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// background
	QColor	background(0, 0, 0, 0);
	painter.fillRect(0, 0, width(), height(), background);

	// determin parameters of thermometer
	double	r = 0.07 * height();
	double	R = 0.1 * height();
	double	m = width() * 2 / 3;
	double	w = 2;
	double	aout = asin(r/R) * 180 / M_PI;
	double	ain = asin((r-w)/(R-w)) * 180 / M_PI;
	double	y = _value * 2 * r + (1 - _value) * (height() - 2 * R);

	QPainterPath	outside;
	outside.moveTo(QPointF(m + r, r));
	outside.arcTo(m - r, 0, 2 * r, 2 * r, 0, 180);
	outside.arcTo(m - R, height() - 2 * R, 2 * R, 2 * R, 
		90 + aout, 360 - 2 * aout);
	outside.closeSubpath();
        QColor  black(0, 0, 0);
        painter.fillPath(outside, black);

	QPainterPath	insidetop;
	insidetop.moveTo(QPointF(m + r - w, r));
	insidetop.arcTo(m - r + w, w, 2 * (r - w), 2 * (r - w), 0, 180);
	insidetop.lineTo(m - r + w, y);
	insidetop.lineTo(m + r - w, y);
	insidetop.closeSubpath();
        QColor  white(255, 255, 255);
        painter.fillPath(insidetop, white);

	QPainterPath	insidebottom;
	insidebottom.moveTo(m + r - w, y);
	insidebottom.lineTo(m - r + w, y);
	insidebottom.arcTo(m - R + w, height() - 2 * R + w,
		2 * (R - w), 2 * (R - w), 
		90 + ain, 360 - 2 * ain);
	insidebottom.closeSubpath();
	QColor	medium(_value * 255 + (1 - _value) * 90, (1 - _value) * 90,
		(1 - _value) * 255);
	painter.fillPath(insidebottom, medium);
	
	QColor	flakecolor(128, 128, (_active) ? 255 : 128);
	double	fx, fy;
	fx = width() * 1 / 3;
	fy = height() * 2 / 3 - 5;
	double	fr = height() / 4 + 5;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "snowflake at %.2f, %.2f, r = %.2f",
		fx, fy, fr);

	QPainterPath	flake;
	double	angle = M_PI / 2;
	flake.moveTo(fx, fy + fr);
	int	sectors = 6;
	while (sectors--) {
		double	x, y;
		double	c = cos(angle);
		double	s = sin(angle);
		for (int i = 0; i < 16; i++) {
			x = fx + fr * (c * outline[i].x - s * outline[i].y);
			y = fy + fr * (s * outline[i].x + c * outline[i].y);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "[%d] %.2f, %.2f",
				i, x, y);
			flake.lineTo(x, y);
		}
		angle += M_PI /3;
	}
	flake.closeSubpath();

	sectors = 6;
	angle = M_PI / 2;
	QColor	transparent(0, 0, 0, 0);
	while (sectors--) {
		QPainterPath	opening;
		double	x, y;
		double	c = cos(angle);
		double	s = sin(angle);
		x = fx + fr * (c * inside[0].x - s * inside[0].y);
		y = fy + fr * (s * inside[0].x + c * inside[0].y);
		opening.moveTo(x, y);
		for (int i = 1; i < 4; i++) {
			x = fx + fr * (c * inside[i].x - s * inside[i].y);
			y = fy + fr * (s * inside[i].x + c * inside[i].y);
			opening.lineTo(x, y);
		}
		opening.closeSubpath();
		flake.addPath(opening);
		angle += M_PI /3;
	}
	painter.fillPath(flake, flakecolor);
}

void	CoolerActive::paintEvent(QPaintEvent * /* event */) {
	draw();
}

void	CoolerActive::update() {
	draw();
}

static double	alpha = 0.1;

void	CoolerActive::update(float actualtemp, float settemp, bool active) {
	_active = active;
	double	newvalue = (actualtemp - settemp) / 20.;
	if (newvalue > 1) {
		newvalue = 1;
	}
	if (newvalue < 0) {
		newvalue = 0;
	}
	_value = alpha * newvalue + (1 - alpha) * _value;
	repaint();
}

} // namespace snowgui
