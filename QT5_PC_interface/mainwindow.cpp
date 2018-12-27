#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtCharts/QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#ifdef Q_OS_LINUX
#include <termios.h>
#endif

void delay(int millisecondsToWait)
{
    QTime dieTime = QTime::currentTime().addMSecs(millisecondsToWait);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serial = new QSerialPort(this);
    series = new QSplineSeries();
    //--------------
    chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    //chart->createDefaultAxes();
    //chart->axisX()->setRange(0, maxRange);
    //chart->axisY()->setRange(0, 5);
    
/*
    QValueAxis *axisX = new QValueAxis;
    axisX->setLabelFormat("%f");
    axisX->setRange(0, 30);
    axisX->setTitleText("Minutes");
    chart->setAxisX(axisX, series);
*/
    QDateTimeAxis *axisX = new QDateTimeAxis;
        axisX->setTickCount(10);
        axisX->setFormat("h:mm");
        axisX->setTitleText("Time");
        axisX->setMin(startingdatetime);
        axisX->setMax(MaxChart);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%4d");
    axisY->setRange(0, 5000);
    axisY->setTitleText("mV");
    chart->setAxisY(axisY, series);

    chart->setTitle("DISCHARGE CURVE");
    //----------------
    chartView = new QChartView(chart);
    chartView->chart()->setTheme(QChart::ChartThemeDark);
    chartView->setRenderHint(QPainter::Antialiasing);
    QPen green(Qt::red);
    green.setWidth(2);
    series->setPen(green);
    ui->verticalLayout_3->addWidget(chartView);

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
}

MainWindow::~MainWindow()
{
    serial->write("E"); // 'E' End readings
    delay(200);
    serial->close();
    delete ui;
}

int MainWindow::openSerialPort()
{
    // Example use QSerialPortInfo
    foreach (const QSerialPortInfo& info, QSerialPortInfo::availablePorts()) {
        qDebug() << "Name : " << info.portName();
        qDebug() << "Description : " << info.description();
        // qDebug() << "Manufacturer: " << info.manufacturer();
        // qDebug() << "Location:" << info.systemLocation();
        // qDebug() << "Product Identifier:" << info.productIdentifier();
        // qDebug() << "Vendor Identifier:" << info.systemLocation();
        //  qDebug() << "Product Identifier:" << info.productIdentifier();

        // Example use QSerialPort
        serial->setPortName(info.portName());
        serial->setBaudRate(QSerialPort::Baud57600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::EvenParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        if (serial->open(QIODevice::ReadWrite)) {

            #ifdef Q_OS_LINUX
            int fd; /* File descriptor for the port */
            struct termios options;
            fd = serial->handle();
            options.c_cflag |= CREAD|CLOCAL;
            options.c_lflag &= (~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|ISIG));
            options.c_iflag &= (~(INPCK|IGNPAR|PARMRK|ISTRIP|ICRNL|IXANY));
            options.c_oflag &= (~OPOST);
            options.c_cc[VMIN] = 0;
            tcsetattr(fd, TCSANOW, &options);
            #endif

            serial->write("who");
            serial->waitForBytesWritten(-1);
           // serial->flush();
            delay(100);
            qDebug() << "data" << RXdata;
            if (RXdata == "XYZ") {
                qDebug() << "FOUND ScarsFun on port: " << info.portName();
                ui->statusBar->showMessage("FOUND ScarsFun USB device", 5000);
                return 1;
            }
            else {
                serial->close();
                qDebug() << "no response from ScarsFun on port" << info.portName();
            }
        }
    }
    //ui->statusBar->showMessage("scarsFun not found on any serial", 5000);
    qDebug() << "scarsFun not found on any serial";
    return 0;
}
void MainWindow::readData()
{
    RXdata = serial->readAll();
    switch (DisplayFlag) {
    case 'S':
        sscanf(RXdata, "%d", &STM_Value);
        if (STM_Value == 99999)
        {
           serial->close(); 
           ui->Lconnect->setStyleSheet("background-color: QColor(53,53,53);");
           ui->statusBar->showMessage(tr("DATA RECEIVED - CONNECTION CLOSED"), 5000);
           ui->button_connection->setEnabled(false);

           break;  
        }
            mAs += (STM_Value * 10) / LOAD_RESISTANCE;  //  24: 2.4 Ohm Resistor 
            ui->label->setText(QString::number(mAs / 360)); // mAh
            incX += 10; //10: data stored every 10 secs;
            // series->append(incX / 60, STM_Value);
            newdatetime = startingdatetime.addSecs(incX);
            series->append(newdatetime.toMSecsSinceEpoch(),STM_Value);

            if (newdatetime  > MaxChart) {
                MaxChart = newdatetime.addSecs(320);
                chart->axisX()->setMax(MaxChart);
                //chart->axisX()->setRange(0, maxRange);
            }
        break;
    }
}
void MainWindow::on_button_About_clicked()
{
    //QMessageBox::about(this, tr("About ScarsFun Connector"),
      //  tr("The <b>ScarsFun Connector</b> example demonstrates how to "
        //   "connect SM32 USB virtual com device to  PC using Qt."));
QMessageBox messageBox;
  QPixmap exportSuccess(":/new/images/logo-qt.png");
  messageBox.setIconPixmap(exportSuccess);
  //messageBox.setIconPixmap(QPixmap(":/logo-qt.png"));
  messageBox.setText("The <b>ScarsFun Connector</b> example demonstrates how to "
                      "connect SM32 USB virtual com device to  PC using QT5.");
  messageBox.setWindowTitle("About ScarsFun Connector");
  messageBox.show();
  messageBox.exec();
 

}
/*
void MainWindow::on_pushButton_toggled(bool checked)
{
    if (checked) {
        serial->write("S"); // 'S' Start readings
        DisplayFlag = 'S';
        qDebug() << "START";
        ui->pushButton->setText("OFF");
        ui->Lreceive->setStyleSheet("QLabel { background-color : green;  }");
        ui->button_connection->setEnabled(false);
    }
    else {
        serial->write("E"); // 'E' End readings
        DisplayFlag = 'E';
        qDebug() << "STOP";
        ui->pushButton->setText("ON");
        ui->Lreceive->setStyleSheet("background-color: QColor(53,53,53);");
        ui->pushButton->setStyleSheet("background-color: QColor(53,53,53);");
        ui->button_connection->setEnabled(true);
    }
}
*/
void MainWindow::on_button_connection_clicked(bool checked)
{
    if (checked) {
        if (openSerialPort()) {
            qDebug() << "CONNECTED";
            ui->button_connection->setText("OFF");
            ui->Lconnect->setStyleSheet("background-color: green;");
            //ui->pushButton->setEnabled(true);
            delay(100);
            serial->write("S"); // 'S' Start readings
            DisplayFlag = 'S';
        }
        else {
            qDebug() << "UNCONNECTED";
            ui->Lconnect->setStyleSheet("background-color: red;");
            ui->statusBar->showMessage(tr("FAILED TO CONNECT"), 5000);
            ui->button_connection->setChecked(false);
        }
    }
    else {
        serial->close();
        ui->button_connection->setText("ON");
        //ui->pushButton->setEnabled(false);
        ui->Lconnect->setStyleSheet("background-color: QColor(53,53,53);");
        ui->button_connection->setStyleSheet("background-color: QColor(53,53,53);");
    }
}
