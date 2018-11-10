/*
 * StarChartLegend.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <QWidget>

namespace snowgui {

class StarChartLegend : public QWidget {
	Q_OBJECT
public:
	explicit StarChartLegend(QWidget *parent = NULL);
	virtual ~StarChartLegend();

private:
	void    draw(QPainter& painter, int& y, QColor color, QString label);
protected:
	void	closeEvent(QCloseEvent *);
public:
	void	paintEvent(QPaintEvent *event);
};

} // namespace snowgui
