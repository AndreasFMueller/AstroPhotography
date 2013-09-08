#ifndef FITSVIEWERWINDOW_H
#define FITSVIEWERWINDOW_H

#include <QMainWindow>
#include <AstroViewer.h>

namespace Ui {
class FITSViewerWindow;
}

using namespace astro::image;

class FITSViewerWindow : public QMainWindow
{
    Q_OBJECT
    Viewer	viewer;
    
    void	update();
public:
    explicit FITSViewerWindow(QWidget *parent, const std::string& filename);
    ~FITSViewerWindow();
    
private:
    Ui::FITSViewerWindow *ui;
private slots:
	void	gammaChanged(int value);
	void	gradientChanged(int state);
	void	backgroundChanged(int state);
	void	doUpdate();
	void	previewupdate();
	void	backgroundupdate();
	void	rangeChanged(int value);
	void	colorcorrectionChanged(double value);
	void	saturationChanged(int value);
};

#endif // FITSVIEWERWINDOW_H
