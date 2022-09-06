#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMap>
#include <QString>
#include <QChart>

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include <QtCharts/QChart>

QT_CHARTS_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QGuiApplication::setWindowIcon(QIcon(":/icons/ikona.ico"));

    readData();
    configTable();
    fillTable();

    configChart();

}

MainWindow::~MainWindow()
{
    delete m_chart;

    delete ui;
}

void MainWindow::readData(){
    QFile file("C:/Users/jakub.glowacki/Documents/Projekty/battery/battery.dat.txt");


    if(!file.open(QIODevice::ReadOnly)){
        qCritical() << "Could not open file!";
        qCritical() << file.errorString();
    }


    while (!file.atEnd()) {
        auto list = file.readLine().split(',');
        Record record;

        record.batID=list[0].toInt();
        record.date=QDateTime::fromString(list[1], "dd.MM.yyyy hh:mm:ss");
        record.duration_sec=list[2].toInt();
        record.voltage_start=list[3].toDouble();
        record.voltage_end=list[4].toDouble();
        record.current_mAh=list[5].toInt();
        record.power_mode=list[6].toInt();

        m_records << record;
    }
    file.close();
}

void MainWindow::configTable(){
    ui->tableWidget->QTableView::setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setMaximumSectionSize(80);
}

void MainWindow::fillTable(){
    QMap<qint16, QVector<qint16>> battery;

    for(QList<Record>::Iterator i = m_records.begin(); i != m_records.end(); i++){
        if(!battery.contains(i->batID)){
            battery.insert(i->batID,QVector<qint16>(2));
        }
        battery[i->batID][0]++;
        battery[i->batID][1]+=i->duration_sec;
    }
    ui->tableWidget->setRowCount(battery.count());

    qint16 j = 0;
    for(QMap<qint16, QVector<qint16>>::iterator i = battery.begin(); i != battery.end(); i++){
        auto id = new QTableWidgetItem;
        auto count = new QTableWidgetItem;
        auto time = new QTableWidgetItem;

        id->setData(Qt::EditRole,i.key());
        count->setData(Qt::EditRole,i.value()[0]);
        time->setData(Qt::EditRole,i.value()[1]/60);

        ui->tableWidget->setItem(j,0,id);
        ui->tableWidget->setItem(j,1,count);
        ui->tableWidget->setItem(j,2,time);
        j++;
    }
}

void MainWindow::configChart(){
    ui->graphicsView->setChart(m_chart);

    m_chart->addSeries(m_series);

    m_chart->addAxis(m_axis_y,Qt::AlignLeft);
    m_series->attachAxis(m_axis_y);

    m_chart->addAxis(m_axis_x,Qt::AlignBottom);
    m_series->attachAxis(m_axis_x);

    m_chart->axes(Qt::Vertical, m_series);
    m_chart->setTitle("Battery usage chart");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
}

void MainWindow::on_tableWidget_cellClicked(int row, [[maybe_unused]]int column)
{
    QStringList date;
    qint32 max{0};

    while(m_set->count()!=0){
        m_set->remove(m_set->count()-1);
    }

    for(QList<Record>::Iterator i = m_records.begin(); i != m_records.end();i++){
        if(i->batID == ui->tableWidget->item(row, 0)->text().toInt()){
            *m_set << qreal(i->duration_sec)/60;
            date << (i->date.toString("dd.MM.yyyy hh:mm:ss"));

            if(max<(i->duration_sec)){
                max=(i->duration_sec);
            }
        }
    }

    m_axis_y->setRange(0,max/60);
    m_axis_x->setCategories(date);

    m_series->append(m_set);
}
