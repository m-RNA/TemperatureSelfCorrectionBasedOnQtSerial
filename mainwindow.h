#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_btnCollect_clicked();

signals:

private:
    Ui::MainWindow *ui;
    QTimer *timerCollect;
    int collectTimestamp;    // 采集时间戳（秒 * TIMESTAMP_FACTOR)
    int sampledPointNum = 0; // 已采集点数
    int sampledPointSum = 8; // 需要采集点数

    void timerCollectTimeOut();
    QString collectTimestampToHhMmSs(int timestamp);
};
#endif // MAINWINDOW_H
