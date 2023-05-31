#include "about.h"
#include "ui_about.h"

/** https://blog.csdn.net/toopoo/article/details/104269471
  */
void Get_Compile_Date_Base(uint8_t *Year, uint8_t *Month, uint8_t *Day)
{
    const char *pMonth[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const char Date[12] = __DATE__; // 取编译时间
    uint8_t i;
    for (i = 0; i < 12; i++)
        if (memcmp(Date, pMonth[i], 3) == 0)
            *Month = i + 1, i = 12;
    *Year = (uint8_t)atoi(Date + 9); // Date[9]为２位年份，Date[7]为完整年份
    *Day = (uint8_t)atoi(Date + 4);
}

About::About(QWidget *parent) : QDialog(parent),
                                ui(new Ui::About)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // 去掉问号，只保留关闭

    uint8_t Year, Month, Day;
    char date_buf[16];
    Get_Compile_Date_Base(&Year, &Month, &Day); // 取编译时间
    sprintf(date_buf, "%02d%02d%02d", Year, Month, Day);
    ui->lbBuildDate->setText("Build" + QString(date_buf));
}

About::~About()
{
    delete ui;
}
