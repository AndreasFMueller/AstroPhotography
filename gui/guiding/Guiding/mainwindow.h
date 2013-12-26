#include <guider.hh>

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	Astro::Guider_var	guider;

private:
    Ui::MainWindow *ui;

private slots:
        void    startGuider();

};

#endif // MAINWINDOW_H
