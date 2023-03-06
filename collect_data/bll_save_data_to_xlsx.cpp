#include "bll_save_data_to_xlsx.h"

Bll_SaveDataToXlsx::Bll_SaveDataToXlsx()
{
    report = new Document("../TemperatureSensorCalibration/ReportTemplate.xlsx");
    report->write(INFO_R, INFO_C, "测试");

    for (int i = 0; i <= 100; ++i)
    {
        report->write(DETAIL_R + i, DETAIL_C, i);
    }
    report->saveAs("Test.xlsx");
}

Bll_SaveDataToXlsx::~Bll_SaveDataToXlsx()
{
    report->deleteLater();
}
