#ifndef BLL_SAVEDATATOXLSX_H
#define BLL_SAVEDATATOXLSX_H
#include <QtXlsx>
#include "config.h"
using namespace QXlsx;
using namespace std;

//  C: Column 列
//  R: Row    行

#define INFO_C 2
#define INFO_R 4

#define FACTOR_C 2
#define FACTOR_R 13

#define AVERAGE_C 2
#define AVERAGE_R 17
#define AVERAGE_C_OFFSET 1

#define DATA_C 1
#define DATA_R 23
#define DATA_C_OFFSET 2

class Bll_SaveDataToXlsx : public QObject
{
    Q_OBJECT
public:
    Bll_SaveDataToXlsx(QObject *parent = nullptr);
    ~Bll_SaveDataToXlsx();

    void nextPoint();
    void saveInfo(const QStringList &info);
    void saveReport(const QString &fileName);

public slots:
    void saveData_Std(const vector<double> &data);
    void saveData_Dtm(const vector<double> &data);
    // void saveFactor(const vector<DECIMAL_TYPE> &factor);

private:
    Document *report = nullptr;
    size_t index = 0;

    QString getAverageFormula(int r1, int r2, int c);
};

#endif // BLL_SAVEDATATOXLSX_H
