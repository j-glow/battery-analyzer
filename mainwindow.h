#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QChart>

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    struct Record{
        qint16 batID;
        QDateTime date;
        qint32 duration_sec;
        qreal voltage_start;
        qreal voltage_end;
        qint32 current_mAh;
        qint8 power_mode;
    };

    QList<Record> m_records;

    QChart* m_chart = new QChart();
    QBarSet* m_set = new QBarSet("Minutes used",m_chart);
    QBarSeries* m_series = new QBarSeries(m_chart);
    QBarCategoryAxis* m_axis_x = new QBarCategoryAxis(m_chart);
    QValueAxis* m_axis_y = new QValueAxis(m_chart);

    void readData();
    void fillTable();
    void configTable();

    void configChart();

private slots:
    void on_tableWidget_cellClicked(int row, int column);
};

#endif // MAINWINDOW_H
