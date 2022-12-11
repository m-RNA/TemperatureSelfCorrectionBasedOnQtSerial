#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool eventFilter(QObject *obj, QEvent *event);

private slots:

    void on_btnSerialSwitch_Std_clicked();
    void on_btnSerialSwitch_Dtm_clicked();

    void uiLookUpdata_Std(bool state);
    void uiLookUpdata_Dtm(bool state);


signals:
    void serialStateChange_Std(bool state);
    void serialStateChange_Dtm(bool state);

private:
    Ui::MainWindow *ui;
    void serialInfoUpdata(void);
    void serialInit(QSerialPort sp);

    QSerialPort *serial_Std;
    QSerialPort *serial_Dtm;
    bool serialState_Std = false;
    bool serialState_Dtm = false;

};
#endif // MAINWINDOW_H
