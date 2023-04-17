#include "bll_save_data_to_xlsx.h"
bool Bll_SaveDataToXlsx::autoSaveState = false;

Bll_SaveDataToXlsx::Bll_SaveDataToXlsx(QObject *parent) : QObject(parent)
{
    initReport();
}

Bll_SaveDataToXlsx::~Bll_SaveDataToXlsx()
{
    report->deleteLater();
}

void Bll_SaveDataToXlsx::startPoint()
{
    int col = DATA_C + DATA_C_OFFSET * index + 1;

    report->write(DATA_R - 2, col, QDateTime::currentDateTime().toString("hh:mm:ss(yyyy/MM/dd)"));

    if (autoSaveState)
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

void Bll_SaveDataToXlsx::initReport()
{
    if (report != nullptr)
        delete report;
    report = new Document(":/ReportTemplate.xlsx");

    fileName = "温度传感器自校准数据表格 " + QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh时mm分ss秒");
    index = 0;
}

void Bll_SaveDataToXlsx::setAutoSave(const bool state)
{
    autoSaveState = state;
}

void Bll_SaveDataToXlsx::saveInfo(const BaseInfo &info)
{
    int row = INFO_R;
    report->write(row, INFO_C, info.place); // 地点

    report->write(++row, INFO_C, QString::number(info.temp) + "℃");   // 温度
    report->write(row, INFO_C + 2, QString::number(info.rh) + "%RH"); // 湿度

    report->write(++row, INFO_C, info.date);           // 日期
    report->write(row, INFO_C + 2, info.operatorName); // 人员

    report->write(++row, INFO_C + 1, info.id_Std); // 标准传感器编号
    report->write(++row, INFO_C + 1, info.id_Dtm); // 待定传感器编号

    if (autoSaveState)
        saveReport();
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

    // 清空多余的单元格
    while (report->read(DATA_R + counter, col).toString() != "")
        report->write(DATA_R + counter++, col, QVariant());
    if (autoSaveState)
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

    // 清空多余的单元格
    while (report->read(DATA_R + counter, col).toString() != "")
        report->write(DATA_R + counter++, col, QVariant());

    if (autoSaveState)
        saveReport();
}

void Bll_SaveDataToXlsx::saveFactor(const vector<DECIMAL_TYPE> &factor)
{
    size_t counter = 0;
    for (DECIMAL_TYPE f : factor)
    {
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "%.8LE", f);
        report->write(FACTOR_R, FACTOR_C + counter, globalStringBuffer);
        report->write(FACTOR_R - 1, FACTOR_C + counter, factor.size() - counter - 1);
        ++counter;
    }

    // 清空多余的单元格
    while (report->read(FACTOR_R, FACTOR_C + counter).toString() != "")
    {
        report->write(FACTOR_R, FACTOR_C + counter, QVariant());
        report->write(FACTOR_R - 1, FACTOR_C + counter, QVariant());
        ++counter;
    }

    if (autoSaveState)
        saveReport();
}

// 输入r1，r2，c，返回xlsx中的求平均值公式字符串
QString Bll_SaveDataToXlsx::getAverageFormula(int r1, int r2, int c)
{
    QString formula = QString("=AVERAGE(%1%2:%3%4)").arg(QChar(c + 'A' - 1)).arg(r1).arg(QChar(c + 'A' - 1)).arg(r2);
    return formula;
}
