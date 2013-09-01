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
};

#endif // FITSVIEWERWINDOW_H
