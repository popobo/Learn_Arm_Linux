#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QPushButton* push_button;

    QFile file;

    void setBeepStatus();

    bool getBeepStatus();

    Ui::MainWindow *ui;

private slots:
    void pushButtonClicked();
};
#endif // MAINWINDOW_H
