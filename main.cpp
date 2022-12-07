#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    /* 自适应高分辨率（DPI） */
    #if (QT_VERSION >= QT_VERSION_CHECK(5,9,0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    #endif
    //    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough); //第二个参数用来控制缩放策略。详细解释可以按F1看帮助文档。

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
