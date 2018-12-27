#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QTime>

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    int openSerialPort();
    void on_button_About_clicked();
    void readData();


//    void on_pushButton_toggled(bool checked);

    void on_button_connection_clicked(bool checked);

private:

    int incX=0, maxRange=10;
    uint mAs=0;

    const uint LOAD_RESISTANCE = 24; //2.4 Ohm
    QByteArray RXdata;
    char DisplayFlag;
    uint STM_Value;
    uint records_nr=0;
    QDateTime startingdatetime = QDateTime(QDate(2000, 7, 7), QTime(0, 0, 0)),
    MaxChart = QDateTime(QDate(2000, 7, 7), QTime(0, 1, 0)),
    newdatetime;

    QSplineSeries *series;
    QChartView *chartView;
    QChart *chart;
    QSerialPort *serial;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
