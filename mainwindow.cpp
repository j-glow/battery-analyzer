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


    connect(ui->buttonFirst,SIGNAL(clicked()),
            this,SLOT(s_buttonFirst()));
    connect(ui->buttonLast,SIGNAL(clicked()),
            this,SLOT(s_buttonLast()));
    connect(ui->buttonNext,SIGNAL(clicked()),
            this,SLOT(s_buttonNext()));
    connect(ui->buttonPrev,SIGNAL(clicked()),
            this,SLOT(s_buttonPrev()));
    connect(ui->tableWidget,SIGNAL(cellClicked(int,int)),
            this,SLOT(s_rowClicked(int,int)));

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
        chooseDir();
    }


    while (!file.atEnd()) {
        auto list = file.readLine().split(',');
        Record record;

        record.batID=list[0].toInt();
        record.date=QDate::fromString(list[1].split(' ')[0], "dd.MM.yyyy");
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
    QStringList date{};
    date << "Mon" << "Tue" << "Wed" << "Thu" << "Fri" << "Sat" << "Sun";
    m_axis_x->setCategories(date);

    m_chart->axes(Qt::Vertical, m_series);
    m_chart->setTitle("Choose battery from table in order to see its usage history");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
}

void MainWindow::s_rowClicked(int row, [[maybe_unused]]int column)
{
    f_batteryDisplay=ui->tableWidget->item(row,0)->text().toInt();

    QDate mostRecent(1800,1,1);

    for(QList<Record>::Iterator i = m_records.begin(); i != m_records.end();i++){

        if(i->batID == f_batteryDisplay && i->date > mostRecent){
            mostRecent=i->date;
        }
    }


    if(mostRecent.dayOfWeek()==1){
        f_weekDisplay=mostRecent;
    }
    else{
        f_weekDisplay=mostRecent.addDays(1-mostRecent.dayOfWeek());
    }
    updateChart();
}

void MainWindow::updateChart(){
    qint32 max{0};

    while(m_set->count()!=0){
        m_set->remove(m_set->count()-1);
    }
    for(qint8 i = 0; i<7;i++){
        m_set->append(0);
    }
    for(QList<Record>::Iterator i = m_records.begin(); i != m_records.end();i++){
        if(i->batID == f_batteryDisplay && i->date>=f_weekDisplay && i->date<f_weekDisplay.addDays(7)){
            m_set->replace(i->date.dayOfWeek()-1,m_set->at(i->date.dayOfWeek()-1)+qreal(i->duration_sec)/60);

            if(max<(i->duration_sec)){
                max=(i->duration_sec);
            }
        }
    }
    qInfo()<<"\n\n\n\nweek content:";
    for(int i=0;i<m_set->count();i++){
        qInfo()<<m_set->at(i);
    }

    m_axis_y->setRange(0,max/60);
    m_series->append(m_set);
    m_chart->setTitle("BatteryID: "+QString::number(f_batteryDisplay));
    ui->week_display->setText(f_weekDisplay.toString("dd.MM.yyyy") + " - " + f_weekDisplay.addDays(6).toString("dd.MM.yyyy"));
    checkButtons();
}

void MainWindow::s_buttonFirst()
{
    if(f_batteryDisplay!=0){
        QDate temp=f_weekDisplay;
        for(QList<Record>::Iterator i = m_records.begin(); i != m_records.end();i++){
            if(i->batID == f_batteryDisplay && i->date < temp){
                temp=i->date;
            }
        }
        if(temp.dayOfWeek()==1){
            f_weekDisplay=temp;
        }
        else{
            f_weekDisplay=temp.addDays(1-temp.dayOfWeek());
        }
        updateChart();
    }
}

void MainWindow::s_buttonLast()
{
    if(f_batteryDisplay!=0){
        QDate temp=f_weekDisplay;
        for(QList<Record>::Iterator i = m_records.begin(); i != m_records.end();i++){
            if(i->batID == f_batteryDisplay && i->date > temp){
                temp=i->date;
            }
        }
        if(temp.dayOfWeek()==1){
            f_weekDisplay=temp;
        }
        else{
            f_weekDisplay=temp.addDays(1-temp.dayOfWeek());
        }
        updateChart();
    }
}

void MainWindow::s_buttonNext()
{
    if(f_batteryDisplay!=0){
        f_weekDisplay=f_weekDisplay.addDays(7);
        updateChart();
    }
}

void MainWindow::s_buttonPrev()
{
    if(f_batteryDisplay!=0){
        f_weekDisplay=f_weekDisplay.addDays(-7);
        updateChart();
    }
}


void MainWindow::checkButtons(){
    QDate min=f_weekDisplay;
    QDate max=f_weekDisplay.addDays(6);

    for(QList<Record>::Iterator i = m_records.begin(); i != m_records.end();i++){
        if(i->batID == f_batteryDisplay && i->date > max){
            max=i->date;
        }
        if(i->batID == f_batteryDisplay && i->date < min){
            min=i->date;
        }
    }

    if(min==f_weekDisplay){
        ui->buttonFirst->setEnabled(false);
        ui->buttonPrev->setEnabled(false);
    }
    if(min!=f_weekDisplay){
        ui->buttonFirst->setEnabled(true);
        ui->buttonPrev->setEnabled(true);
    }
    if(max==f_weekDisplay.addDays(6)){
        ui->buttonLast->setEnabled(false);
        ui->buttonNext->setEnabled(false);
    }
    if(max!=f_weekDisplay.addDays(6)){
        ui->buttonLast->setEnabled(true);
        ui->buttonNext->setEnabled(true);
    }
}

void MainWindow::chooseDir(){
//choose new data file directory
}
