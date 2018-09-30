/*
 * LocalSiderealTime.h -- Widget to continuously display LMST
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
class LocalSiderealTime : public QLabel {
	Q_OBJECT

	astro::LongLat	_position;
	QTimer	*_timer;
public:
	void	position(const astro::LongLat& p);
	const astro::LongLat&	position() const { return _position; }
private:
	long	_offset;
public:
	long	offset() const { return _offset; }
	void	offset(long o) { _offset = o; }

	LocalSiderealTime(QWidget *parent = NULL);
	~LocalSiderealTime();
	
public slots:
	void	update();
};

} // 
