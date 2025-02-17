#ifndef BLL_SAVEDATATOXLSX_H
#define BLL_SAVEDATATOXLSX_H
#if USE_LOCAL_COMPILED_LIB == 1
#include <QtXlsx>
#else
#include "xlsxdocument.h"
// #include "xlsxchart.h"
#endif

#include "config.h"
#include "wizard.h"
using namespace QXlsx;
using namespace std;

//  C: Column 列
//  R: Row    行

#define INFO_C 2
#define INFO_R 4

#define FACTOR_C 2
#define FACTOR_R 14
#define FACTOR_C_OFFSET 2

#define AVERAGE_C 2
#define AVERAGE_R 18
#define AVERAGE_C_OFFSET 1

#define DATA_C 2
#define DATA_R 26
#define DATA_C_OFFSET 2

#define RANGE_C DATA_C
#define RANGE_R 4
#define RANGE_C_OFFSET 1

// #define CHART_C DATA_C
// #define CHART_R 3
// #define CHART_W 400
// #define CHART_H 200
// #define CHART_MARGIN 10

class Bll_SaveDataToXlsx : public QObject
{
    Q_OBJECT
public:
    Bll_SaveDataToXlsx(QObject *parent = nullptr);
    ~Bll_SaveDataToXlsx();

public slots:
    void saveReport();
    static void setAutoSave(const bool);
    void startPoint();
    void saveInfo(const BaseInfo &info);
    void saveData_Std(const vector<double> &data);
    void saveData_Dtm(const vector<double> &data);
    void saveFactor(const vector<DECIMAL_TYPE> &factor);

private:
    Document *report = nullptr;
    QString fileName = "";
    static bool autoSaveState;

    void initReport();
    bool switchWorkSheet(const int index);
    QString getRangeFormula(int r1, int r2, int c);
    QString getAverageFormula(int r1, int r2, int c);
    QString getAverageReferFormula(int r1, int r2, int c);
    void saveData(const vector<double> &data, const int index);
};

#endif // BLL_SAVEDATATOXLSX_H
