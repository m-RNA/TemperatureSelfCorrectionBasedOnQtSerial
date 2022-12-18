#include "mainwindow.h"
#include "startcommunication.h"
#include "customchart.h"
#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
/* 自适应高分辨率（DPI） */
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    //    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough); //第二个参数用来控制缩放策略。详细解释可以按F1看帮助文档。
    qDebug() << "主线程ID：" << QThread::currentThread();

    QApplication a(argc, argv);
    
    // 程序异常退出：检查有无野指针，未初始化变量 参考B站：BV1U14y1K7Po
    MainWindow w;
    // StartCommunication w;
    // CustomChart w;
    w.show();

    //    MainWindow *w = new MainWindow; // 程序异常退出：此方法不推荐，有的任务未处理
    //    w->show();

    return a.exec();
}
