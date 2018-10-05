/*
 * skydisplaydialog.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswi
 */
#ifndef SNOWGUI_SKYDISPLAYDIALOG_H
#define SNOWGUI_SKYDISPLAYDIALOG_H

#include <QDialog>
#include <AstroCoordinates.h>

namespace snowgui {

namespace Ui {
	class SkyDisplayDialog;
}

class SkyDisplayDialog : public QDialog {
	Q_OBJECT

public:
	explicit SkyDisplayDialog(QWidget *parent = 0);
	~SkyDisplayDialog();

	void	telescope(const astro::RaDec&);
	const astro::RaDec&	telescope() const;
	void	target(const astro::RaDec&);
	const astro::RaDec&	target() const;

	void	position(const astro::LongLat& p);
	const astro::LongLat&	position() const;


private:
	Ui::SkyDisplayDialog *ui;

protected:
	void	closeEvent(QCloseEvent *e);

public slots:
	void	telescopeChanged(astro::RaDec);
	void	positionChanged(astro::LongLat);
	void	targetSelected(astro::RaDec);

	void	altazChanged(int);
	void	radecChanged(int);
	void	constellationsChanged(int);
	void	telescopeChanged(int);
	void	targetChanged(int);
	void	targetChanged(astro::RaDec);
	void	labelsChanged(int);
	void	update();

signals:
	void	pointSelected(astro::RaDec);
};

} // namespace snowgui
#endif // SNOWGUI_SKYDISPLAYDIALOG_H
