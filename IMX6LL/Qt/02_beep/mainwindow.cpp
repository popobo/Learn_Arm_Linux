#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QRect>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    /*
     * get screen resolution in this way, suggested by Qt official,
     * to avoid issues of devices with multi-screen devices.
     * attention, this is to get the resolution of whole desktop system.
     */
    QList<QScreen*> list_screen = QGuiApplication::screens();

    /* if ARM platform, just set the size to the size of screen */
#if __arm__
    this->resize(list_screen.at(0)->geometry().width(), list_screen.at(0)->geometry().height());
#else
    /* or we just set the size of main window to 800x480 */
    this->resize(800, 480);
#endif

    push_button = new QPushButton(this);

    /* centered display */
    push_button->setMinimumSize(200, 50);
    push_button->setGeometry((this->width() - push_button->width()) / 2,
                             (this->height() - push_button->height()) / 2,
                             push_button->width(),
                             push_button->height());

    /* Beep control interface */
    file.setFileName("/sys/devices/platform/leds/leds/beep/brightness");
    if (!file.exists())
        push_button->setText("No Beep device");

    getBeepStatus();

    connect(push_button, SIGNAL(clicked()), this, SLOT(pushButtonClicked()));

    // ui->setupUi(this); this will render the buttons unresponsive to touch
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setBeepStatus()
{
    bool status = getBeepStatus();

    if (!file.exists())
        return;

    if (!file.open(QIODevice::ReadWrite))
        qDebug() << file.errorString();

    QByteArray buf[2] = {"0", "1"};

    if (status) file.write(buf[0]);
    else file.write(buf[1]);

    file.close();

    getBeepStatus();
}

bool MainWindow::getBeepStatus()
{
    if (!file.exists())
        return false;

    if (!file.open(QIODevice::ReadWrite))
        qDebug() << file.errorString();

    QTextStream in(&file);

    QString buf = in.readLine();

    qDebug() << "buf: " << buf <<endl;

    file.close();

    if (buf == "1")
    {
        push_button->setText("Beep is on");
        return true;
    }
    else
    {
        push_button->setText("Beep is off");
        return false;
    }
}

void MainWindow::pushButtonClicked()
{
    qDebug() << "pushButtonClick" << endl;
    setBeepStatus();
}
