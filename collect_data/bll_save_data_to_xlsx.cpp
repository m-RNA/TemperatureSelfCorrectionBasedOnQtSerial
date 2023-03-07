#include "bll_save_data_to_xlsx.h"

Bll_SaveDataToXlsx::Bll_SaveDataToXlsx(QObject *parent) : QObject(parent)
{
    resetIndex();
}

Bll_SaveDataToXlsx::~Bll_SaveDataToXlsx()
{
    report->deleteLater();
}

void Bll_SaveDataToXlsx::startPoint()
{
    int col = DATA_C + DATA_C_OFFSET * index + 1;

    report->write(DATA_R - 2, col, QDateTime::currentDateTime().toString("hh:mm:ss(yyyy/MM/dd)"));
    saveReport();
}

void Bll_SaveDataToXlsx::nextPoint()
{
    ++index;
}

void Bll_SaveDataToXlsx::saveReport()
{
    report->saveAs(fileName + ".xlsx");
}

void Bll_SaveDataToXlsx::resetIndex()
{
    delete report;
    report = new Document("../TemperatureSensorCalibration/ReportTemplate.xlsx");

    fileName = "Test " + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
    index = 0;
}

void Bll_SaveDataToXlsx::saveData_Std(const vector<double> &data)
{
    size_t counter = 0;
    int col = DATA_C + DATA_C_OFFSET * index;
    for (double d : data)
    {
        report->write(DATA_R + counter, col, d);
        ++counter;
    }
    // 在求平均值的单元格写入公式
    report->write(AVERAGE_R, AVERAGE_C + AVERAGE_C_OFFSET * index, getAverageFormula(DATA_R, DATA_R + counter - 1, col));
    saveReport();
}

void Bll_SaveDataToXlsx::saveData_Dtm(const vector<double> &data)
{
    size_t counter = 0;
    int col = DATA_C + DATA_C_OFFSET * index + 1;
    for (double d : data)
    {
        report->write(DATA_R + counter, col, d);
        ++counter;
    }
    // 在求平均值的单元格写入公式
    report->write(AVERAGE_R + 1, AVERAGE_C + AVERAGE_C_OFFSET * index, getAverageFormula(DATA_R, DATA_R + counter - 1, col));
    saveReport();
}

void saveFactor(const vector<DECIMAL_TYPE> &factor)
{
    size_t counter = 0;
    for (DECIMAL_TYPE d : factor)
    {
        // report->write(FACTOR_R, FACTOR_C + counter, d);
        ++counter;
    }
}

// 输入r1，r2，c，返回xlsx中的求平均值公式字符串
QString Bll_SaveDataToXlsx::getAverageFormula(int r1, int r2, int c)
{
    QString formula = QString("=AVERAGE(%1%2:%3%4)").arg(QChar(c + 'A' - 1)).arg(r1).arg(QChar(c + 'A' - 1)).arg(r2);
    return formula;
}
