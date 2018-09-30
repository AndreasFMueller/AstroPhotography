/*
 * ClockWidget.h -- display the current time
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <QLabel>
#include <QTimer>

namespace snowgui {

/**
 * \brief Clock display class
 */
class ClockWidget : public QLabel {
	Q_OBJECT
	QTimer	*_timer;
	long	_offset;
public:
	long	offset() const { return _offset; }
	void	offset(long l) { _offset = l; }

	explicit ClockWidget(QWidget *parent = NULL);
	~ClockWidget();
	
public slots:
	void	update();
};

}
