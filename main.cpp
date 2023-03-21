#include "mainwindow.h"
#include "startcommunication.h"
#include <QApplication>
#include <QThread>
#include "bll_save_data_to_xlsx.h"
#include "bll_serialrecvanalyse.h"
#include "wizard.h"

int main(int argc, char *argv[])
{
/* 自适应高分辨率（DPI） */
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    //    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough); //第二个参数用来控制缩放策略。详细解释可以按F1看帮助文档。

    qRegisterMetaType<QVector<double>>("QVector<double>");           // 添加不支持的类型
    qRegisterMetaType<vector<double>>("vector<double>");             // 添加不支持的类型
    qRegisterMetaType<QVector<long double>>("QVector<long double>"); // 添加不支持的类型
    qRegisterMetaType<vector<long double>>("vector<long double>");   // 添加不支持的类型
    qRegisterMetaType<serialAnalyseCell>("serialAnalyseCell");       // 添加不支持的类型
    qRegisterMetaType<WizardInfo>("WizardInfo");                     // 添加不支持的类型
    qRegisterMetaType<SoundIndex>("SoundIndex");                     // 添加不支持的类型

    qDebug() << "主线程ID：" << QThread::currentThread();
    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD_COUNT);

    QApplication a(argc, argv);
    // StartCommunication w;

    // 程序异常退出：检查有无野指针，未初始化变量 参考B站：BV1U14y1K7Po
    MainWindow w;
    w.show();
    return a.exec();

    //    MainWindow *w = new MainWindow; // 程序异常退出：此方法不推荐，有的任务未处理
    //    w->show();

    // Bll_SaveDataToXlsx bll;
    // return 0;
}
