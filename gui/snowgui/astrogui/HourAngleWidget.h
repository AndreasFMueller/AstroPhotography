/*
 * HourAngleWidget.h -- Widget to continuously display hour angle
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <QLabel>
#include <QTimer>
#include <AstroCoordinates.h>

namespace snowgui {

/**
 * \brief Display class to display the current local sidereal time
 */
class HourAngleWidget : public QLabel {
	Q_OBJECT

	astro::LongLat	_position;
	astro::Angle	_ra;
	QTimer	_timer;
public:
	void	position(const astro::LongLat& p);
	const astro::LongLat&	position() const { return _position; }
	void	ra(const astro::Angle& a);
	const astro::Angle&	ra() const { return _ra; }
	
private:
	long	_offset;
public:
	long	offset() const { return _offset; }
	void	offset(long o) { _offset = o; }

	HourAngleWidget(QWidget *parent = NULL);
	~HourAngleWidget();
	
private:
	void	updateCommon(time_t);
public slots:
	void	update();
	void	update(time_t);
};

} // 
