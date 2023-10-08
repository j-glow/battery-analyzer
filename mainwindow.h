#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QMap>
#include <QString>
#include <QChart>
#include <QFileDialog>

#include <QtCharts/QChartView>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;

    struct Record{
        qint16 batID;
        QDate date;
        QTime time;
        qint32 duration_sec;
        qreal voltage_start;
        qreal voltage_end;
        qint32 current_mAh;
        qint8 power_mode;
    };

    QList<Record> m_records;
    QDate f_weekDisplay{};
    qint16 f_batteryDisplay{0};

    QChart* m_chart = new QChart();
    QVector<QBarSet*> m_sets;
    QStackedBarSeries* m_series = new QStackedBarSeries(m_chart);
    QBarCategoryAxis* m_axis_x = new QBarCategoryAxis(m_chart);
    QValueAxis* m_axis_y = new QValueAxis(m_chart);

    void fillTable();
    void configTable();
    void configChart();
    void updateChart();
    void checkButtons();

private slots:
    void s_rowClicked(int row, int column);
    void s_buttonFirst();
    void s_buttonLast();
    void s_buttonNext();
    void s_buttonPrev();
    void s_loadFile();
    void s_showDay(int day);
    void s_readData(QString path = "./battery.dat.txt");
};

#endif // MAINWINDOW_H
