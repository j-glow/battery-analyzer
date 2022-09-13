#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dirchoosedialog.h"

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
    connect(ui->batteryTable,SIGNAL(cellClicked(int,int)),
            this,SLOT(s_rowClicked(int,int)));
    connect(ui->buttonLoad,SIGNAL(clicked()),
            this,SLOT(s_loadFile()));

    s_readData();
    configTable();
    configChart();

    //default splitter position ...
    ui->splitter->setStretchFactor(0,3);
    ui->splitter->setStretchFactor(1,1);
}

MainWindow::~MainWindow()
{
    delete m_chart;
    delete ui;
}

void MainWindow::s_readData(QString path)
{
    QFile file(path);


    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "Error", "Could not open file!");
        s_loadFile();
    }

    m_records.clear();

    while (!file.atEnd()) {
        auto list = file.readLine().split(',');
        Record record;

        record.batID=list[0].toInt();
        record.date=QDate::fromString(list[1].split(' ')[0], "dd.MM.yyyy");
        record.time=QTime::fromString(list[1].split(' ')[1], "hh:mm:ss");
        record.duration_sec=list[2].toInt();
        record.voltage_start=list[3].toDouble();
        record.voltage_end=list[4].toDouble();
        record.current_mAh=list[5].toInt();
        record.power_mode=list[6].toInt();

        m_records << record;
    }
    file.close();
    fillTable();
}

void MainWindow::configTable()
{
    ui->batteryTable->QTableView::setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->batteryTable->horizontalHeader()->setMaximumSectionSize(80);
}

void MainWindow::fillTable()
{
    QMap<qint16, QVector<qint16>> battery;

    for(QList<Record>::Iterator i = m_records.begin(); i != m_records.end(); i++){
        if(!battery.contains(i->batID)){
            battery.insert(i->batID,QVector<qint16>(2));
        }
        battery[i->batID][0]++;
        battery[i->batID][1]+=i->duration_sec;
    }//filtering records as per battery not as per record
    ui->batteryTable->setRowCount(battery.count());

    qint16 j = 0;
    for(QMap<qint16, QVector<qint16>>::iterator i = battery.begin(); i != battery.end(); i++){
        auto id = new QTableWidgetItem;
        auto count = new QTableWidgetItem;
        auto time = new QTableWidgetItem;

        id->setData(Qt::EditRole,i.key());
        count->setData(Qt::EditRole,i.value()[0]);
        time->setData(Qt::EditRole,i.value()[1]/60);

        ui->batteryTable->setItem(j,0,id);
        ui->batteryTable->setItem(j,1,count);
        ui->batteryTable->setItem(j,2,time);
        j++;
    }//inputing info into record table
}

void MainWindow::configChart()
{
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
    m_chart->legend()->setVisible(false);
}

void MainWindow::s_rowClicked(int row, [[maybe_unused]]int column)
{
    f_batteryDisplay=ui->batteryTable->item(row,0)->text().toInt();

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

void MainWindow::updateChart()
{
    for(auto i{0}; i<m_sets.size();i++){
        disconnect(m_sets[i],SIGNAL(clicked(int)),
                   this,SLOT(s_showDay(int)));
        m_series->remove(m_sets[i]);
        m_sets[i]=nullptr;
    }//clean up previous bars
    m_sets.clear();

    qint8 days[7]={0,0,0,0,0,0,0};
    for(QList<Record>::Iterator i = m_records.begin(); i != m_records.end();i++){
        if(i->batID == f_batteryDisplay && i->date>=f_weekDisplay && i->date<f_weekDisplay.addDays(7)){
            days[f_weekDisplay.dayOfWeek()-1]++;
        }
    }//find how many values for each day

    qint16 numOfSets{0};
    for(auto i : days){
        if (i>numOfSets){
            numOfSets=i;
        }
    }//take in maximum value to use as limiter for bar array creator

    for(qint16 i{0};i<numOfSets;i++)
    {
        if(m_sets.size()<=i) m_sets.push_back(new QBarSet(QString::number(i),m_chart));
        else m_sets[i]=new QBarSet(QString::number(i),m_chart);

        for(qint8 j = 0; j<7;j++){
            m_sets[i]->append(0);
        }
    }//create empty 0-filled bars

    qint32 dailyDuration[7]={0,0,0,0,0,0,0};
    qint16 j{0};
    for(QList<Record>::Iterator i = m_records.begin(); i != m_records.end();i++){
        if(i->batID == f_batteryDisplay && i->date>=f_weekDisplay && i->date<f_weekDisplay.addDays(7)){
            j=0;
            while(m_sets[j]->at(i->date.dayOfWeek()-1)){
                j++;
            }//find first empty set

            m_sets[j]->replace(i->date.dayOfWeek()-1,qreal(i->duration_sec)/60);

            dailyDuration[i->date.dayOfWeek()-1]+=(i->duration_sec);
        }
    }//fill up bars based on records

    qint32 max{0};
    for(qint8 i=0;i<7;i++){
        if(max<dailyDuration[i]){
            max=dailyDuration[i];
        }
    }//find max day duration
    m_axis_y->setRange(0,qreal(max)/60);
    for(qint16 i{0};i<m_sets.size();i++){
        m_series->append(m_sets[i]);
        connect(m_sets[i],SIGNAL(clicked(int)),
                this,SLOT(s_showDay(int)));
    }

    m_chart->setTitle("BatteryID: "+QString::number(f_batteryDisplay));
    ui->week_display->setText(f_weekDisplay.toString("dd.MM.yyyy") + " - " + f_weekDisplay.addDays(6).toString("dd.MM.yyyy"));
    checkButtons();

    qint8 i{0};
    while(m_sets[0]->at(i)==0) i++;
    s_showDay(i);
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

void MainWindow::s_showDay(int day)
{
    ui->day_indicator->setText("Battery: " + QString::number(f_batteryDisplay) +
                               "\tDay: " +f_weekDisplay.addDays(day).toString("dd.MM.yyyy"));
    ui->recordTable->setRowCount(0);
    qint16 j=0;
    for(QList<Record>::Iterator i = m_records.begin(); i != m_records.end(); i++)
    {

        if(i->batID==f_batteryDisplay && i->date==f_weekDisplay.addDays(day))
        {
            ui->recordTable->setRowCount(ui->recordTable->rowCount()+1);
            auto start = new QTableWidgetItem;
            auto finish = new QTableWidgetItem;
            auto duration = new QTableWidgetItem;

            QString dur = QString::number(i->duration_sec/3600)+"h "+
                    QString::number(i->duration_sec/60)+"min "+QString::number(i->duration_sec%60)+"s";

            start->setData(Qt::EditRole,i->time.toString("hh:mm:ss"));
            finish->setData(Qt::EditRole,i->time.addSecs(i->duration_sec).toString("hh:mm:ss"));
            duration->setData(Qt::EditRole,dur);

            ui->recordTable->setItem(j,0,start);
            ui->recordTable->setItem(j,1,finish);
            ui->recordTable->setItem(j,2,duration);
            j++;
        }
    }
}

void MainWindow::s_loadFile(){
    d = new dirChooseDialog(this);
    d->setModal(true);
    d->show();
    connect(d,SIGNAL(sendDir(QString)),
            this,SLOT(s_readData(QString)));
}
